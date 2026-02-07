#ifndef FIREBASE_MANAGER_H
#define FIREBASE_MANAGER_H

#include <Arduino.h>
#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include "receiver/IProtocolDecoder.h"

enum class FirebaseState {
    DISCONNECTED,
    WIFI_CONNECTING,
    WIFI_CONNECTED,
    FIREBASE_AUTHENTICATING,
    FIREBASE_READY,
    ERROR_WIFI_FAILED,
    ERROR_AUTH_FAILED
};

// Callback for isLearning state changes
using LearningStateCallback = std::function<void(bool isLearning)>;

// Command received via RTDB pendingCommand
struct PendingCommand {
    String protocol;
    uint32_t address;
    uint32_t command;
    uint16_t bits;
};

// Callback for command dispatch via RTDB pendingCommand
using CommandCallback = std::function<void(const PendingCommand& cmd)>;

class FirebaseManager {
public:
    FirebaseManager(
        const char* wifiSSID,
        const char* wifiPassword,
        const char* apiKey,
        const char* projectId,
        const char* databaseUrl,
        const char* userEmail,
        const char* userPassword,
        const char* deviceId
    );

    // Connection management
    bool begin();
    void update();  // Call in main loop
    bool isReady() const { return state == FirebaseState::FIREBASE_READY; }
    FirebaseState getState() const { return state; }
    
    // RTDB streaming (replaces Firestore polling)
    bool beginDeviceStream();
    
    // Firestore operations
    bool uploadSignal(const DecodedSignal& signal, const String& commandName);
    bool setLearningMode(bool isLearning);
    
    // Callbacks
    void onLearningStateChange(LearningStateCallback callback) { 
        learningStateCallback = callback; 
    }
    void onCommandReceived(CommandCallback callback) {
        commandCallback = callback;
    }

private:
    // Configuration
    const char* wifiSSID;
    const char* wifiPassword;
    const char* apiKey;
    const char* projectId;
    const char* databaseUrl;
    const char* userEmail;
    const char* userPassword;
    const char* deviceId;
    
    // Firebase objects
    FirebaseData fbdo;           // For Firestore operations
    FirebaseData streamFbdo;     // Dedicated for RTDB streaming
    FirebaseAuth auth;
    FirebaseConfig config;
    
    // State
    FirebaseState state;
    unsigned long lastConnectionAttempt;
    bool streamStarted;
    
    // Thread-safe flags set by RTDB stream callback, consumed by update()
    volatile bool pendingLearningChange;
    volatile bool pendingLearningState;
    volatile bool pendingCommandReceived;
    bool lastLearningState;
    PendingCommand pendingCmd;
    
    // Callbacks
    LearningStateCallback learningStateCallback;
    CommandCallback commandCallback;
    
    // Stream callbacks (static so they can be passed to library)
    static FirebaseManager* instance;  // Singleton ref for static callbacks
    static void onStreamData(FirebaseStream data);
    static void onStreamTimeout(bool timeout);
    
    // Helper methods
    bool connectWiFi();        // Initial connection with network scan
    bool reconnectWiFi();      // Reconnection with full radio reset
    bool startWiFiConnection(); // Shared connection logic
    String getDevicePath() const;
    String getCommandsPath() const;
    String getRtdbDevicePath() const;
};

#endif
