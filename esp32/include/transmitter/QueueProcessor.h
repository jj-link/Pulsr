#ifndef QUEUE_PROCESSOR_H
#define QUEUE_PROCESSOR_H

#include <Arduino.h>
#include <Firebase_ESP_Client.h>
#include "transmitter/IProtocolEncoder.h"
#include "transmitter/IIRTransmitter.h"

enum class TransmissionStatus {
    PENDING,
    PROCESSING,
    SENT,
    FAILED
};

struct TransmissionQueueItem {
    String queueId;
    String commandId;
    TransmissionStatus status;
    unsigned long createdAt;
    unsigned long processedAt;
    int retryCount;
};

class QueueProcessor {
public:
    QueueProcessor(
        FirebaseData* fbdo,
        const char* projectId,
        const char* deviceId,
        IProtocolEncoder* encoder,
        IIRTransmitter* transmitter,
        uint32_t pollIntervalMs = 100
    );

    void update();  // Call in main loop
    bool isProcessing() const { return processing; }
    
    // Stats
    uint32_t getTotalSent() const { return totalSent; }
    uint32_t getTotalFailed() const { return totalFailed; }

private:
    FirebaseData* fbdo;
    const char* projectId;
    const char* deviceId;
    IProtocolEncoder* encoder;
    IIRTransmitter* transmitter;
    
    uint32_t pollIntervalMs;
    unsigned long lastPollTime;
    bool processing;
    
    // Stats
    uint32_t totalSent;
    uint32_t totalFailed;
    
    // Helper methods
    bool pollQueue();
    bool loadCommand(const String& commandId, String& protocol, uint32_t& address, uint32_t& command, uint16_t& bits);
    bool transmitCommand(const String& protocol, uint32_t address, uint32_t command, uint16_t bits);
    bool updateQueueStatus(const String& queueId, TransmissionStatus status);
    String getQueuePath() const;
    String getCommandPath(const String& commandId) const;
};

#endif
