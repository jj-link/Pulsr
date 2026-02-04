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
    totalFailed(0)
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
    
    // Query for PENDING items ordered by createdAt (FIFO)
    // Note: This is a simplified implementation
    // In production, you'd use Firestore query with where() and orderBy()
    
    if (!Firebase.Firestore.getDocument(fbdo, projectId, "", queuePath.c_str())) {
        // No pending items or error
        return false;
    }
    
    FirebaseJson json;
    json.setJsonData(fbdo->payload());
    
    FirebaseJsonData result;
    
    // Get the first pending item
    // Extract commandId
    if (!json.get(result, "fields/commandId/stringValue")) {
        return false;
    }
    String commandId = result.stringValue;
    
    // Get queueId from document path
    String queueId = "queue_" + String(millis());
    
    // Mark as processing
    processing = true;
    updateQueueStatus(queueId, TransmissionStatus::PROCESSING);
    
    // Load command details
    String protocol;
    uint32_t address, command;
    uint16_t bits;
    
    if (!loadCommand(commandId, protocol, address, command, bits)) {
        Serial.println("[Queue] Failed to load command");
        updateQueueStatus(queueId, TransmissionStatus::FAILED);
        processing = false;
        totalFailed++;
        return false;
    }
    
    // Transmit the command
    if (transmitCommand(protocol, address, command, bits)) {
        Serial.println("[Queue] Command sent successfully");
        updateQueueStatus(queueId, TransmissionStatus::SENT);
        totalSent++;
    } else {
        Serial.println("[Queue] Command transmission failed");
        updateQueueStatus(queueId, TransmissionStatus::FAILED);
        totalFailed++;
    }
    
    processing = false;
    return true;
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
    
    // Extract address
    if (json.get(result, "fields/address/integerValue")) {
        address = result.intValue;
    } else {
        address = 0;
    }
    
    // Extract command
    if (json.get(result, "fields/command/integerValue")) {
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
    
    // Set status
    String statusStr;
    switch (status) {
        case TransmissionStatus::PENDING:
            statusStr = "PENDING";
            break;
        case TransmissionStatus::PROCESSING:
            statusStr = "PROCESSING";
            break;
        case TransmissionStatus::SENT:
            statusStr = "SENT";
            break;
        case TransmissionStatus::FAILED:
            statusStr = "FAILED";
            break;
    }
    
    content.set("fields/status/stringValue", statusStr);
    
    // Set processedAt timestamp if completed
    if (status == TransmissionStatus::SENT || status == TransmissionStatus::FAILED) {
        content.set("fields/processedAt/timestampValue", "now");
    }
    
    if (Firebase.Firestore.patchDocument(fbdo, projectId, "", queuePath.c_str(), content.raw(), "status")) {
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
