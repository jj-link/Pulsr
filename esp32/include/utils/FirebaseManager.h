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

// Callback for isLearning state changes from Firestore
using LearningStateCallback = std::function<void(bool isLearning)>;

class FirebaseManager {
public:
    FirebaseManager(
        const char* wifiSSID,
        const char* wifiPassword,
        const char* apiKey,
        const char* projectId,
        const char* userEmail,
        const char* userPassword,
        const char* deviceId
    );

    // Connection management
    bool begin();
    void update();  // Call in main loop
    bool isReady() const { return state == FirebaseState::FIREBASE_READY; }
    FirebaseState getState() const { return state; }
    
    // Firestore operations
    bool uploadSignal(const DecodedSignal& signal, const String& commandName);
    bool setLearningMode(bool isLearning);
    bool checkLearningMode();
    
    // Callbacks
    void onLearningStateChange(LearningStateCallback callback) { 
        learningStateCallback = callback; 
    }

private:
    // Configuration
    const char* wifiSSID;
    const char* wifiPassword;
    const char* apiKey;
    const char* projectId;
    const char* userEmail;
    const char* userPassword;
    const char* deviceId;
    
    // Firebase objects
    FirebaseData fbdo;
    FirebaseAuth auth;
    FirebaseConfig config;
    
    // State
    FirebaseState state;
    unsigned long lastConnectionAttempt;
    unsigned long lastLearningCheck;
    bool lastLearningState;
    LearningStateCallback learningStateCallback;
    
    // Helper methods
    bool connectWiFi();
    bool authenticateFirebase();
    String getDevicePath() const;
    String getCommandsPath() const;
};

#endif
