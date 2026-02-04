#include "utils/FirebaseManager.h"

FirebaseManager::FirebaseManager(
    const char* wifiSSID,
    const char* wifiPassword,
    const char* apiKey,
    const char* projectId,
    const char* userEmail,
    const char* userPassword,
    const char* deviceId
) : wifiSSID(wifiSSID),
    wifiPassword(wifiPassword),
    apiKey(apiKey),
    projectId(projectId),
    userEmail(userEmail),
    userPassword(userPassword),
    deviceId(deviceId),
    state(FirebaseState::DISCONNECTED),
    lastConnectionAttempt(0),
    lastLearningCheck(0),
    lastLearningState(false),
    learningStateCallback(nullptr)
{
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
    auth.user.email = userEmail;
    auth.user.password = userPassword;
    
    // Set token status callback
    config.token_status_callback = tokenStatusCallback;
    
    // Initialize Firebase
    Firebase.begin(&config, &auth);
    Firebase.reconnectWiFi(true);
    
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
        }
        
        // Attempt reconnection every 30 seconds
        if (millis() - lastConnectionAttempt > 30000) {
            Serial.println("[Firebase] Attempting WiFi reconnection...");
            connectWiFi();
            lastConnectionAttempt = millis();
        }
        return;
    }
    
    // Check Firebase ready state
    if (Firebase.ready() && state != FirebaseState::FIREBASE_READY) {
        Serial.println("[Firebase] Authentication successful - Ready!");
        state = FirebaseState::FIREBASE_READY;
    }
    
    // Poll for learning mode changes every 2 seconds when ready
    if (isReady() && millis() - lastLearningCheck > 2000) {
        checkLearningMode();
        lastLearningCheck = millis();
    }
}

bool FirebaseManager::connectWiFi() {
    Serial.print("[WiFi] Connecting to ");
    Serial.println(wifiSSID);
    
    state = FirebaseState::WIFI_CONNECTING;
    WiFi.begin(wifiSSID, wifiPassword);
    
    unsigned long startAttempt = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - startAttempt < 10000) {
        delay(100);
        Serial.print(".");
    }
    Serial.println();
    
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("[WiFi] Connection failed!");
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
    
    // Build Firestore document path
    String documentPath = getCommandsPath() + "/" + commandName;
    
    // Create Firestore document content
    FirebaseJson content;
    content.set("fields/name/stringValue", commandName);
    content.set("fields/protocol/stringValue", signal.protocol);
    content.set("fields/address/integerValue", String(signal.address));
    content.set("fields/command/integerValue", String(signal.command));
    content.set("fields/value/integerValue", String(signal.value));
    content.set("fields/bits/integerValue", String(signal.bits));
    content.set("fields/isKnownProtocol/booleanValue", signal.isKnownProtocol);
    content.set("fields/capturedAt/timestampValue", "now");
    
    // Upload to Firestore
    Serial.print("[Firebase] Uploading signal: ");
    Serial.println(documentPath);
    
    if (Firebase.Firestore.createDocument(&fbdo, projectId, "", documentPath.c_str(), content.raw())) {
        Serial.println("[Firebase] Signal uploaded successfully!");
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

bool FirebaseManager::checkLearningMode() {
    if (!isReady()) {
        return false;
    }
    
    String documentPath = getDevicePath();
    
    if (Firebase.Firestore.getDocument(&fbdo, projectId, "", documentPath.c_str(), "isLearning")) {
        FirebaseJson json;
        json.setJsonData(fbdo.payload());
        
        FirebaseJsonData result;
        if (json.get(result, "fields/isLearning/booleanValue")) {
            bool currentLearningState = result.boolValue;
            
            // Check if state changed
            if (currentLearningState != lastLearningState) {
                Serial.print("[Firebase] Learning mode changed: ");
                Serial.println(currentLearningState ? "ON" : "OFF");
                
                lastLearningState = currentLearningState;
                
                // Notify callback
                if (learningStateCallback) {
                    learningStateCallback(currentLearningState);
                }
            }
            
            return true;
        }
    }
    
    return false;
}

String FirebaseManager::getDevicePath() const {
    return String("devices/") + deviceId;
}

String FirebaseManager::getCommandsPath() const {
    return getDevicePath() + "/commands";
}
