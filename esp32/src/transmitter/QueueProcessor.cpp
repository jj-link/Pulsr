#include "transmitter/QueueProcessor.h"

QueueProcessor::QueueProcessor(
    FirebaseData* fbdo,
    const char* projectId,
    const char* deviceId,
    IProtocolEncoder* encoder,
    IIRTransmitter* transmitter,
    uint32_t pollIntervalMs
) : fbdo(fbdo),
    projectId(projectId),
    deviceId(deviceId),
    encoder(encoder),
    transmitter(transmitter),
    pollIntervalMs(pollIntervalMs),
    lastPollTime(0),
    processing(false),
    totalSent(0),
    totalFailed(0),
    transmissionCallback(nullptr)
{
}

void QueueProcessor::update() {
    // Check if it's time to poll
    if (millis() - lastPollTime < pollIntervalMs) {
        return;
    }
    
    lastPollTime = millis();
    
    // Poll for pending queue items
    if (!processing) {
        pollQueue();
    }
}

bool QueueProcessor::pollQueue() {
    String queuePath = getQueuePath();
    
    // List all documents in the queue subcollection
    if (!Firebase.Firestore.listDocuments(fbdo, projectId, "", queuePath.c_str(), 10, "", "", "", false)) {
        return false;
    }
    
    // Parse the list response
    FirebaseJson json;
    json.setJsonData(fbdo->payload());
    
    // Check if there are any documents
    FirebaseJsonData documentsArray;
    if (!json.get(documentsArray, "documents")) {
        return false;  // No documents in queue
    }
    
    // Iterate through documents to find a pending one
    FirebaseJsonArray arr;
    documentsArray.getArray(arr);
    
    for (size_t i = 0; i < arr.size(); i++) {
        FirebaseJsonData docData;
        arr.get(docData, i);
        
        FirebaseJson docJson;
        docJson.setJsonData(docData.stringValue);
        
        // Check status
        FirebaseJsonData statusResult;
        if (!docJson.get(statusResult, "fields/status/stringValue")) {
            continue;
        }
        
        if (statusResult.stringValue != "pending") {
            continue;  // Skip non-pending items
        }
        
        // Found a pending item - extract its info
        FirebaseJsonData nameResult;
        if (!docJson.get(nameResult, "name")) {
            continue;
        }
        
        // Extract queueId from document path (last segment)
        String fullPath = nameResult.stringValue;
        int lastSlash = fullPath.lastIndexOf('/');
        String queueId = fullPath.substring(lastSlash + 1);
        
        // Extract commandId
        FirebaseJsonData cmdResult;
        if (!docJson.get(cmdResult, "fields/commandId/stringValue")) {
            continue;
        }
        String commandId = cmdResult.stringValue;
        
        // Process this item
        processing = true;
        
        // Notify: processing started
        if (transmissionCallback) {
            transmissionCallback(TransmissionStatus::PROCESSING, "", commandId);
        }
        
        updateQueueStatus(queueId, TransmissionStatus::PROCESSING);
        
        // Load command details
        String protocol;
        uint32_t address, command;
        uint16_t bits;
        
        // The commandId from the web app is in format "deviceId/firestoreDocId"
        // We only need the firestoreDocId part for loading
        String cmdDocId = commandId;
        int slashIdx = commandId.indexOf('/');
        if (slashIdx >= 0) {
            cmdDocId = commandId.substring(slashIdx + 1);
        }
        
        if (!loadCommand(cmdDocId, protocol, address, command, bits)) {
            Serial.println("[Queue] Failed to load command");
            updateQueueStatus(queueId, TransmissionStatus::FAILED);
            if (transmissionCallback) {
                transmissionCallback(TransmissionStatus::FAILED, "", commandId);
            }
            processing = false;
            totalFailed++;
            return false;
        }
        
        // Transmit the command
        if (transmitCommand(protocol, address, command, bits)) {
            Serial.println("[Queue] Command sent successfully");
            updateQueueStatus(queueId, TransmissionStatus::COMPLETED);
            if (transmissionCallback) {
                transmissionCallback(TransmissionStatus::COMPLETED, protocol, commandId);
            }
            totalSent++;
        } else {
            Serial.println("[Queue] Command transmission failed");
            updateQueueStatus(queueId, TransmissionStatus::FAILED);
            if (transmissionCallback) {
                transmissionCallback(TransmissionStatus::FAILED, protocol, commandId);
            }
            totalFailed++;
        }
        
        processing = false;
        return true;  // Process one item per poll cycle
    }
    
    return false;  // No pending items found
}

bool QueueProcessor::loadCommand(const String& commandId, String& protocol, uint32_t& address, uint32_t& command, uint16_t& bits) {
    String commandPath = getCommandPath(commandId);
    
    if (!Firebase.Firestore.getDocument(fbdo, projectId, "", commandPath.c_str())) {
        Serial.print("[Queue] Failed to load command: ");
        Serial.println(fbdo->errorReason());
        return false;
    }
    
    FirebaseJson json;
    json.setJsonData(fbdo->payload());
    
    FirebaseJsonData result;
    
    // Extract protocol
    if (json.get(result, "fields/protocol/stringValue")) {
        protocol = result.stringValue;
    } else {
        return false;
    }
    
    // Extract address (stored as stringValue in Firestore)
    if (json.get(result, "fields/address/stringValue")) {
        address = strtoul(result.stringValue.c_str(), nullptr, 10);
    } else if (json.get(result, "fields/address/integerValue")) {
        address = result.intValue;
    } else {
        address = 0;
    }
    
    // Extract command (stored as stringValue in Firestore)
    if (json.get(result, "fields/command/stringValue")) {
        command = strtoul(result.stringValue.c_str(), nullptr, 10);
    } else if (json.get(result, "fields/command/integerValue")) {
        command = result.intValue;
    } else {
        return false;
    }
    
    // Extract bits
    if (json.get(result, "fields/bits/integerValue")) {
        bits = result.intValue;
    } else {
        bits = 32;  // Default to 32 bits
    }
    
    return true;
}

bool QueueProcessor::transmitCommand(const String& protocol, uint32_t address, uint32_t command, uint16_t bits) {
    Serial.print("[Queue] Transmitting: ");
    Serial.print(protocol);
    Serial.print(" addr=0x");
    Serial.print(address, HEX);
    Serial.print(" cmd=0x");
    Serial.println(command, HEX);
    
    // Encode the command
    EncodedSignal encoded = encoder->encode(protocol.c_str(), address, command, bits);
    
    if (!encoded.isKnownProtocol) {
        Serial.println("[Queue] Unknown protocol, cannot encode");
        return false;
    }
    
    // Transmit the encoded signal
    TransmitResult result = transmitter->transmit(encoded.rawData, encoded.rawLength, encoded.frequency);
    
    // Clean up allocated memory
    if (encoded.rawData) {
        delete[] encoded.rawData;
    }
    
    return result.success;
}

bool QueueProcessor::updateQueueStatus(const String& queueId, TransmissionStatus status) {
    String queuePath = getQueuePath() + "/" + queueId;
    
    FirebaseJson content;
    
    // Use lowercase status values to match web app
    String statusStr;
    switch (status) {
        case TransmissionStatus::PENDING:
            statusStr = "pending";
            break;
        case TransmissionStatus::PROCESSING:
            statusStr = "processing";
            break;
        case TransmissionStatus::COMPLETED:
            statusStr = "completed";
            break;
        case TransmissionStatus::FAILED:
            statusStr = "failed";
            break;
    }
    
    content.set("fields/status/stringValue", statusStr);
    
    // Set processedAt timestamp using ISO 8601 format
    if (status == TransmissionStatus::COMPLETED || status == TransmissionStatus::FAILED) {
        time_t now = time(nullptr);
        struct tm* timeinfo = gmtime(&now);
        char timestamp[30];
        strftime(timestamp, sizeof(timestamp), "%Y-%m-%dT%H:%M:%SZ", timeinfo);
        content.set("fields/processedAt/timestampValue", timestamp);
    }
    
    // Patch both status and processedAt fields
    String updateMask = "status";
    if (status == TransmissionStatus::COMPLETED || status == TransmissionStatus::FAILED) {
        updateMask += ",processedAt";
    }
    
    if (Firebase.Firestore.patchDocument(fbdo, projectId, "", queuePath.c_str(), content.raw(), updateMask.c_str())) {
        return true;
    } else {
        Serial.print("[Queue] Failed to update status: ");
        Serial.println(fbdo->errorReason());
        return false;
    }
}

String QueueProcessor::getQueuePath() const {
    return String("devices/") + deviceId + "/queue";
}

String QueueProcessor::getCommandPath(const String& commandId) const {
    return String("devices/") + deviceId + "/commands/" + commandId;
}
