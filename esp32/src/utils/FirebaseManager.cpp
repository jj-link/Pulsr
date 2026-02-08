#include "utils/FirebaseManager.h"

// Static singleton reference for stream callbacks
FirebaseManager* FirebaseManager::instance = nullptr;

FirebaseManager::FirebaseManager(
    const char* wifiSSID,
    const char* wifiPassword,
    const char* apiKey,
    const char* projectId,
    const char* databaseUrl,
    const char* userEmail,
    const char* userPassword,
    const char* deviceId
) : wifiSSID(wifiSSID),
    wifiPassword(wifiPassword),
    apiKey(apiKey),
    projectId(projectId),
    databaseUrl(databaseUrl),
    userEmail(userEmail),
    userPassword(userPassword),
    deviceId(deviceId),
    state(FirebaseState::DISCONNECTED),
    lastConnectionAttempt(0),
    streamStarted(false),
    pendingLearningChange(false),
    pendingLearningState(false),
    pendingCommandReceived(false),
    lastLearningState(false),
    learningStateCallback(nullptr),
    commandCallback(nullptr)
{
    instance = this;
}

bool FirebaseManager::begin() {
    Serial.println("[Firebase] Initializing...");
    
    // Connect to WiFi
    if (!connectWiFi()) {
        state = FirebaseState::ERROR_WIFI_FAILED;
        return false;
    }
    
    // Configure Firebase
    config.api_key = apiKey;
    config.database_url = databaseUrl;
    auth.user.email = userEmail;
    auth.user.password = userPassword;
    
    // Initialize Firebase
    Firebase.begin(&config, &auth);
    Firebase.reconnectWiFi(false);  // We handle WiFi reconnection manually
    
    Serial.println("[Firebase] Configuration complete");
    state = FirebaseState::FIREBASE_AUTHENTICATING;
    
    return true;
}

void FirebaseManager::update() {
    // Check WiFi connection
    if (WiFi.status() != WL_CONNECTED) {
        if (state != FirebaseState::ERROR_WIFI_FAILED) {
            Serial.println("[Firebase] WiFi connection lost");
            state = FirebaseState::ERROR_WIFI_FAILED;
            // Cleanly stop the RTDB stream so SSL state doesn't corrupt
            if (streamStarted) {
                Firebase.RTDB.endStream(&streamFbdo);
                streamFbdo.clear();
                streamStarted = false;
                Serial.println("[RTDB] Stream stopped (WiFi lost)");
            }
        }
        
        // Attempt reconnection every 10 seconds
        if (millis() - lastConnectionAttempt > 10000) {
            Serial.println("[Firebase] Attempting WiFi reconnection...");
            reconnectWiFi();
            lastConnectionAttempt = millis();
        }
        return;
    }
    
    // Check Firebase ready state
    if (Firebase.ready() && state != FirebaseState::FIREBASE_READY) {
        Serial.println("[Firebase] Authentication successful - Ready!");
        state = FirebaseState::FIREBASE_READY;
    }
    
    // Start RTDB stream once ready
    if (isReady() && !streamStarted) {
        beginDeviceStream();
    }
    
    // Process pending stream events (thread-safe: flags set by stream callback)
    if (pendingLearningChange) {
        pendingLearningChange = false;
        bool newState = pendingLearningState;
        
        if (newState != lastLearningState) {
            Serial.print("[RTDB] Learning mode changed: ");
            Serial.println(newState ? "ON" : "OFF");
            lastLearningState = newState;
            
            if (learningStateCallback) {
                learningStateCallback(newState);
            }
        }
    }
    
    if (pendingCommandReceived) {
        pendingCommandReceived = false;
        
        Serial.print("[RTDB] Command received: ");
        Serial.print(pendingCmd.protocol);
        Serial.print(" value=0x");
        Serial.print((unsigned long)pendingCmd.value, HEX);
        Serial.print(" bits=");
        Serial.println(pendingCmd.bits);
        
        if (commandCallback) {
            commandCallback(pendingCmd);
        }
        
        // Clear pendingCommand from RTDB so it doesn't re-trigger on reconnect
        String cmdPath = getRtdbDevicePath() + "/pendingCommand";
        Firebase.RTDB.deleteNode(&fbdo, cmdPath.c_str());
    }
}

bool FirebaseManager::beginDeviceStream() {
    String streamPath = getRtdbDevicePath();
    
    Serial.print("[RTDB] Starting stream on: ");
    Serial.println(streamPath);
    
    // Clear any stale SSL/connection state before starting
    streamFbdo.clear();
    
    if (!Firebase.RTDB.beginStream(&streamFbdo, streamPath.c_str())) {
        Serial.print("[RTDB] Stream begin failed: ");
        Serial.println(streamFbdo.errorReason());
        return false;
    }
    
    Firebase.RTDB.setStreamCallback(&streamFbdo, onStreamData, onStreamTimeout);
    
    streamStarted = true;
    Serial.println("[RTDB] Stream started successfully");
    return true;
}

void FirebaseManager::onStreamData(FirebaseStream data) {
    if (!instance) return;
    
    String path = data.dataPath();
    
    if (path == "/isLearning" || path == "isLearning") {
        instance->pendingLearningState = data.boolData();
        instance->pendingLearningChange = true;
    } else if (path == "/pendingCommand") {
        // Command dispatch — parse the command object
        if (data.dataType() == "json") {
            FirebaseJson json = data.jsonObject();
            FirebaseJsonData result;
            
            if (json.get(result, "protocol")) instance->pendingCmd.protocol = result.stringValue;
            if (json.get(result, "value")) instance->pendingCmd.value = strtoull(result.stringValue.c_str(), nullptr, 10);
            if (json.get(result, "bits")) instance->pendingCmd.bits = result.intValue;
            
            instance->pendingCommandReceived = true;
        }
    } else if (path == "/") {
        // Initial stream event sends the entire node — parse children
        FirebaseJson json = data.jsonObject();
        FirebaseJsonData result;
        
        if (json.get(result, "isLearning")) {
            instance->pendingLearningState = result.boolValue;
            instance->pendingLearningChange = true;
        }
        // Parse pendingCommand from initial load if present
        // (will be cleared after processing, so usually absent)
        FirebaseJsonData cmdData;
        if (json.get(cmdData, "pendingCommand") && cmdData.type == "object") {
            FirebaseJson cmdJson;
            cmdJson.setJsonData(cmdData.stringValue);
            FirebaseJsonData r;
            if (cmdJson.get(r, "protocol")) instance->pendingCmd.protocol = r.stringValue;
            if (cmdJson.get(r, "value")) instance->pendingCmd.value = strtoull(r.stringValue.c_str(), nullptr, 10);
            if (cmdJson.get(r, "bits")) instance->pendingCmd.bits = r.intValue;
            instance->pendingCommandReceived = true;
        }
    }
}

void FirebaseManager::onStreamTimeout(bool timeout) {
    if (timeout) {
        Serial.println("[RTDB] Stream timeout - will auto-reconnect");
        // End and clear the stream so beginDeviceStream() gets a clean slate
        Firebase.RTDB.endStream(&instance->streamFbdo);
        instance->streamFbdo.clear();
    }
    if (instance) {
        instance->streamStarted = false;
    }
}

bool FirebaseManager::connectWiFi() {
    // Set station mode explicitly (required for ESP32-S3)
    WiFi.mode(WIFI_STA);
    delay(100);
    
    Serial.println("[WiFi] Scanning for networks...");
    int n = WiFi.scanNetworks();
    Serial.print("[WiFi] Found ");
    Serial.print(n);
    Serial.println(" networks:");
    for (int i = 0; i < n && i < 10; i++) {
        Serial.print("  ");
        Serial.print(i + 1);
        Serial.print(": '");
        Serial.print(WiFi.SSID(i));
        Serial.print("' (");
        Serial.print(WiFi.RSSI(i));
        Serial.println(" dBm)");
    }
    WiFi.scanDelete();
    
    return startWiFiConnection();
}

bool FirebaseManager::reconnectWiFi() {
    // Full radio reset for clean reconnection
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
    delay(500);
    WiFi.mode(WIFI_STA);
    delay(100);
    
    Serial.print("[WiFi] Reconnecting to: '");
    Serial.print(wifiSSID);
    Serial.println("'");
    
    return startWiFiConnection();
}

bool FirebaseManager::startWiFiConnection() {
    Serial.print("[WiFi] Connecting to: '");
    Serial.print(wifiSSID);
    Serial.println("'");
    
    state = FirebaseState::WIFI_CONNECTING;
    WiFi.begin(wifiSSID, wifiPassword);
    WiFi.setSleep(false);  // Disable WiFi power saving to prevent disconnects
    WiFi.setTxPower(WIFI_POWER_8_5dBm);  // Reduce TX power to fix auth issues with some routers
    
    unsigned long startAttempt = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - startAttempt < 10000) {
        delay(100);
        Serial.print(".");
    }
    Serial.println();
    
    if (WiFi.status() != WL_CONNECTED) {
        Serial.print("[WiFi] Connection failed! Status code: ");
        Serial.println(WiFi.status());
        return false;
    }
    
    Serial.print("[WiFi] Connected! IP: ");
    Serial.println(WiFi.localIP());
    state = FirebaseState::WIFI_CONNECTED;
    return true;
}

bool FirebaseManager::uploadSignal(const DecodedSignal& signal, const String& commandName) {
    if (!isReady()) {
        Serial.println("[Firebase] Not ready - cannot upload signal");
        return false;
    }
    
    // Write pendingSignal field on the device document (limited to 1)
    String documentPath = getDevicePath();
    
    // Build pendingSignal as a map field on the device document
    FirebaseJson content;
    content.set("fields/pendingSignal/mapValue/fields/protocol/stringValue", signal.protocol);
    content.set("fields/pendingSignal/mapValue/fields/address/stringValue", String(signal.address));
    content.set("fields/pendingSignal/mapValue/fields/command/stringValue", String(signal.command));
    content.set("fields/pendingSignal/mapValue/fields/value/stringValue", String(signal.value));
    content.set("fields/pendingSignal/mapValue/fields/bits/integerValue", String(signal.bits));
    content.set("fields/pendingSignal/mapValue/fields/isKnownProtocol/booleanValue", signal.isKnownProtocol);
    
    // Use ISO 8601 timestamp format (required by Firestore REST API)
    time_t now = time(nullptr);
    struct tm* timeinfo = gmtime(&now);
    char timestamp[30];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%dT%H:%M:%SZ", timeinfo);
    content.set("fields/pendingSignal/mapValue/fields/capturedAt/timestampValue", timestamp);
    
    // Upload to Firestore (patch only the pendingSignal field)
    Serial.print("[Firebase] Uploading pending signal to: ");
    Serial.println(documentPath);
    
    if (Firebase.Firestore.patchDocument(&fbdo, projectId, "", documentPath.c_str(), content.raw(), "pendingSignal")) {
        Serial.println("[Firebase] Pending signal uploaded successfully!");
        return true;
    } else {
        Serial.print("[Firebase] Upload failed: ");
        Serial.println(fbdo.errorReason());
        return false;
    }
}

bool FirebaseManager::setLearningMode(bool isLearning) {
    if (!isReady()) {
        Serial.println("[Firebase] Not ready - cannot set learning mode");
        return false;
    }
    
    String documentPath = getDevicePath();
    
    FirebaseJson content;
    content.set("fields/isLearning/booleanValue", isLearning);
    
    Serial.print("[Firebase] Setting learning mode: ");
    Serial.println(isLearning ? "ON" : "OFF");
    
    if (Firebase.Firestore.patchDocument(&fbdo, projectId, "", documentPath.c_str(), content.raw(), "isLearning")) {
        Serial.println("[Firebase] Learning mode updated");
        lastLearningState = isLearning;
        return true;
    } else {
        Serial.print("[Firebase] Update failed: ");
        Serial.println(fbdo.errorReason());
        return false;
    }
}

String FirebaseManager::getDevicePath() const {
    return String("devices/") + deviceId;
}

String FirebaseManager::getCommandsPath() const {
    return getDevicePath() + "/commands";
}

String FirebaseManager::getRtdbDevicePath() const {
    return String("/devices/") + deviceId;
}
