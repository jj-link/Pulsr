#ifndef QUEUE_PROCESSOR_H
#define QUEUE_PROCESSOR_H

#include <Arduino.h>
#include <functional>
#include <Firebase_ESP_Client.h>
#include "transmitter/IProtocolEncoder.h"
#include "transmitter/IIRTransmitter.h"

enum class TransmissionStatus {
    PENDING,
    PROCESSING,
    COMPLETED,
    FAILED
};

// Callback for transmission events (status, protocol, commandId)
using TransmissionEventCallback = std::function<void(TransmissionStatus status, const String& protocol, const String& commandId)>;

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
        uint32_t pollIntervalMs = 2000
    );

    void update();  // Call in main loop
    bool isProcessing() const { return processing; }
    
    // Callbacks
    void onTransmissionEvent(TransmissionEventCallback callback) {
        transmissionCallback = callback;
    }
    
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
    
    // Error recovery
    uint32_t consecutiveErrors;
    unsigned long backoffUntil;
    static const uint32_t MAX_BACKOFF_MS = 60000;  // 1 minute max
    static const uint32_t RESET_AFTER_ERRORS = 5;  // Reset SSL after 5 consecutive failures
    void resetConnection();
    uint32_t getBackoffDelay() const;
    
    // Stats
    uint32_t totalSent;
    uint32_t totalFailed;
    
    // Callback
    TransmissionEventCallback transmissionCallback;
    
    // Helper methods
    bool pollQueue();
    bool loadCommand(const String& commandId, String& protocol, uint32_t& address, uint32_t& command, uint16_t& bits);
    bool transmitCommand(const String& protocol, uint32_t address, uint32_t command, uint16_t bits);
    bool updateQueueStatus(const String& queueId, TransmissionStatus status);
    String getQueuePath() const;
    String getCommandPath(const String& commandId) const;
};

#endif
