/**
 * Pulsr - ESP32 IR Controller Production Embedded Software
 * 
 * This embedded software implements a cloud-connected IR controller with:
 * - IR signal learning and decoding (receiver)
 * - IR signal transmission via RTDB pendingCommand (transmitter)
 * - Firestore integration for command storage
 * - Real-time control from web UI via RTDB streaming
 * - Runtime provisioning via AP setup portal (onboarding)
 * 
 * Architecture:
 * - Uses interface abstractions for testability
 * - Dependency injection for loose coupling
 * - Callback-based event system
 * - Boot modes: UNPROVISIONED, PROVISIONED_UNCLAIMED, RUNNING, RECOVERY
 */

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include "config.h"

#include "utils/ProvisioningManager.h"
#include "utils/APSetupServer.h"
#include "utils/ClaimManager.h"

#include "receiver/ESP32SignalCapture.h"
#include "receiver/IRLibProtocolDecoder.h"
#include "receiver/LearningStateMachine.h"
#include "transmitter/ESP32IRTransmitter.h"
#include "utils/FirebaseManager.h"
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"

ProvisioningManager provisioningManager;
APSetupServer* apSetupServer = nullptr;
ClaimManager* claimManager = nullptr;

ESP32SignalCapture signalCapture(IR_RECEIVE_PIN);
IRLibProtocolDecoder protocolDecoder;
LearningStateMachine learningStateMachine(&signalCapture, &protocolDecoder, LEARNING_TIMEOUT_MS);
ESP32IRTransmitter irTransmitter(IR_SEND_PIN, false);
FirebaseManager* firebaseManager = nullptr;

Adafruit_NeoPixel statusLED(NEOPIXEL_COUNT, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);

#define COLOR_OFF           statusLED.Color(0, 0, 0)
#define COLOR_CONNECTING    statusLED.Color(20, 20, 0)
#define COLOR_READY         statusLED.Color(0, 20, 0)
#define COLOR_LEARNING      statusLED.Color(0, 0, 100)
#define COLOR_SUCCESS       statusLED.Color(0, 100, 0)
#define COLOR_ERROR         statusLED.Color(100, 0, 0)
#define COLOR_TIMEOUT       statusLED.Color(100, 50, 0)
#define COLOR_TX_PROCESSING statusLED.Color(80, 0, 80)
#define COLOR_TX_SUCCESS    statusLED.Color(0, 100, 50)
#define COLOR_TX_FAILED     statusLED.Color(100, 20, 0)
#define COLOR_AP_MODE       statusLED.Color(50, 50, 100)
#define COLOR_CLAIMING      statusLED.Color(100, 100, 0)

bool runningModeActive = false;

void onLearningStateChanged(LearningState state) {
    Serial.print("[Learning] State changed: ");
    
    switch (state) {
        case LearningState::IDLE:
            Serial.println("IDLE");
            statusLED.setPixelColor(0, COLOR_READY);
            statusLED.show();
            if (firebaseManager) firebaseManager->setLearningMode(false);
            break;
            
        case LearningState::LEARNING:
            Serial.println("LEARNING - Waiting for IR signal...");
            statusLED.setPixelColor(0, COLOR_LEARNING);
            statusLED.show();
            break;
            
        case LearningState::CAPTURED:
            Serial.println("CAPTURED - Signal received!");
            statusLED.setPixelColor(0, COLOR_SUCCESS);
            statusLED.show();
            break;
            
        case LearningState::TIMEOUT:
            Serial.println("TIMEOUT - No signal received");
            statusLED.setPixelColor(0, COLOR_TIMEOUT);
            statusLED.show();
            break;
    }
}

void onSignalCaptured(const DecodedSignal& signal) {
    Serial.println("========== CAPTURED IR SIGNAL ==========");
    Serial.print("Protocol: ");
    Serial.println(signal.protocol);
    Serial.print("Address: 0x");
    Serial.println(signal.address, HEX);
    Serial.print("Command: 0x");
    Serial.println(signal.command, HEX);
    Serial.print("Value: 0x");
    Serial.println((unsigned long)signal.value, HEX);
    Serial.print("Bits: ");
    Serial.println(signal.bits);
    Serial.print("Known Protocol: ");
    Serial.println(signal.isKnownProtocol ? "Yes" : "No");
    Serial.println("=========================================");
    
    String commandName = String("cmd_") + String(millis());
    if (firebaseManager && firebaseManager->uploadSignal(signal, commandName)) {
        Serial.println("[Main] Signal uploaded to Firestore successfully!");
    } else {
        Serial.println("[Main] Failed to upload signal to Firestore");
    }
}

unsigned long txLedRevertTime = 0;

void onCommandReceived(const PendingCommand& cmd) {
    statusLED.setPixelColor(0, COLOR_TX_PROCESSING);
    statusLED.show();
    
    Serial.print("[TX] Dispatching: ");
    Serial.print(cmd.protocol);
    Serial.print(" value=0x");
    Serial.print((unsigned long)cmd.value, HEX);
    Serial.print(" bits=");
    Serial.println(cmd.bits);
    
    TransmitResult result;
    if (cmd.protocol == "SAMSUNG") {
        result = irTransmitter.transmitSamsung(cmd.value, cmd.bits);
    } else if (cmd.protocol == "NEC") {
        result = irTransmitter.transmitNEC((uint32_t)cmd.value, cmd.bits);
    } else if (cmd.protocol == "SONY") {
        result = irTransmitter.transmitSony((uint32_t)cmd.value, cmd.bits);
    } else {
        Serial.print("[TX] Unknown protocol: ");
        Serial.println(cmd.protocol);
        statusLED.setPixelColor(0, COLOR_TX_FAILED);
        statusLED.show();
        txLedRevertTime = millis() + 1000;
        return;
    }
    
    if (result.success) {
        Serial.print("[TX] Transmitted OK: ");
        Serial.print(cmd.protocol);
        Serial.print(" value=0x");
        Serial.println((unsigned long)cmd.value, HEX);
        statusLED.setPixelColor(0, COLOR_TX_SUCCESS);
        statusLED.show();
        txLedRevertTime = millis() + 500;
    } else {
        Serial.println("[TX] Transmit failed!");
        statusLED.setPixelColor(0, COLOR_TX_FAILED);
        statusLED.show();
        txLedRevertTime = millis() + 1000;
    }
}

void onFirebaseLearningModeChanged(bool isLearning) {
    Serial.print("[Firebase] Learning mode changed: ");
    Serial.println(isLearning ? "ON" : "OFF");
    
    if (isLearning) {
        signalCapture.enable();
        learningStateMachine.startLearning();
    } else {
        learningStateMachine.stopLearning();
        signalCapture.disable();
    }
}

void handleUnprovisionedMode() {
    Serial.println("[Main] Starting UNPROVISIONED mode - AP setup portal");
    
    statusLED.setPixelColor(0, COLOR_AP_MODE);
    statusLED.show();
    
    apSetupServer = new APSetupServer(&provisioningManager);
    apSetupServer->begin();
    
    while (true) {
        apSetupServer->update();
        delay(10);
    }
}

void handleProvisionedUnclaimedMode() {
    Serial.println("[Main] Starting PROVISIONED_UNCLAIMED mode - attempting claim");
    
    statusLED.setPixelColor(0, COLOR_CLAIMING);
    statusLED.show();
    
    claimManager = new ClaimManager(
        &provisioningManager,
        FIREBASE_API_KEY,
        FIREBASE_PROJECT_ID,
        FIREBASE_DATABASE_URL
    );
    
    ClaimResult result = claimManager->redeemClaim();
    
    if (result.success) {
        Serial.println("[Main] Claim successful! Rebooting to RUNNING mode...");
        statusLED.setPixelColor(0, COLOR_SUCCESS);
        statusLED.show();
        delay(2000);
        ESP.restart();
    } else {
        Serial.print("[Main] Claim failed: ");
        Serial.println(result.errorMessage);
        
        Serial.println("[Main] Rebooting to retry...");
        statusLED.setPixelColor(0, COLOR_ERROR);
        statusLED.show();
        delay(3000);
        ESP.restart();
    }
}

void setup() {
    Serial.begin(115200);
    Serial.println("\n========================================");
    Serial.println("[Pulsr] Starting up...");
    Serial.println("========================================");
    
    statusLED.begin();
    statusLED.setBrightness(NEOPIXEL_BRIGHTNESS);
    statusLED.setPixelColor(0, COLOR_CONNECTING);
    statusLED.show();
    
    if (!provisioningManager.begin()) {
        Serial.println("[Main] Failed to initialize NVS - forcing AP mode");
        handleUnprovisionedMode();
        return;
    }
    
    String hardwareId = provisioningManager.getHardwareId();
    Serial.print("[Pulsr] Hardware ID: ");
    Serial.println(hardwareId);
    
    BootMode bootMode = provisioningManager.determineBootMode();
    
    Serial.print("[Pulsr] Boot mode: ");
    switch (bootMode) {
        case BootMode::UNPROVISIONED:
            Serial.println("UNPROVISIONED");
            break;
        case BootMode::PROVISIONED_UNCLAIMED:
            Serial.println("PROVISIONED_UNCLAIMED");
            break;
        case BootMode::RUNNING:
            Serial.println("RUNNING");
            break;
        case BootMode::RECOVERY:
            Serial.println("RECOVERY");
            break;
    }
    
    switch (bootMode) {
        case BootMode::UNPROVISIONED:
        case BootMode::RECOVERY:
            handleUnprovisionedMode();
            break;
            
        case BootMode::PROVISIONED_UNCLAIMED:
            handleProvisionedUnclaimedMode();
            break;
            
        case BootMode::RUNNING: {
            String deviceId = provisioningManager.getDeviceId();
            String wifiSSID = provisioningManager.getWiFiSSID();
            String wifiPassword = provisioningManager.getWiFiPassword();
            
            Serial.print("[Pulsr] Device ID: ");
            Serial.println(deviceId);
            
            firebaseManager = new FirebaseManager(
                FIREBASE_API_KEY,
                FIREBASE_PROJECT_ID,
                FIREBASE_DATABASE_URL,
                FIREBASE_USER_EMAIL,
                FIREBASE_USER_PASSWORD
            );
            firebaseManager->setWiFiCredentials(wifiSSID.c_str(), wifiPassword.c_str());
            firebaseManager->setDeviceId(deviceId.c_str());
            
            Serial.println("[Main] Starting RUNNING mode - normal operation");
            
            Serial.print("[Pulsr] IR Receiver on GPIO ");
            Serial.println(IR_RECEIVE_PIN);
            
            irTransmitter.begin();
            Serial.print("[Pulsr] IR Transmitter initialized on GPIO ");
            Serial.println(IR_SEND_PIN);
            
            learningStateMachine.onStateChange(onLearningStateChanged);
            learningStateMachine.onSignalCapture(onSignalCaptured);
            firebaseManager->onLearningStateChange(onFirebaseLearningModeChanged);
            firebaseManager->onCommandReceived(onCommandReceived);
            
            Serial.println("[Pulsr] Connecting to Firebase...");
            if (firebaseManager->begin()) {
                Serial.println("[Pulsr] Firebase connection initiated");
            } else {
                Serial.println("[Pulsr] Firebase connection failed - will retry");
                statusLED.setPixelColor(0, COLOR_ERROR);
                statusLED.show();
            }
            
            runningModeActive = true;
            Serial.println("[Pulsr] Initialization complete!");
            break;
        }
    }
}

void loop() {
    if (runningModeActive && firebaseManager) {
        firebaseManager->update();
        learningStateMachine.update();
        
        if (txLedRevertTime > 0 && millis() >= txLedRevertTime) {
            statusLED.setPixelColor(0, COLOR_READY);
            statusLED.show();
            txLedRevertTime = 0;
        }
        
        static FirebaseState lastFirebaseState = FirebaseState::DISCONNECTED;
        FirebaseState currentFirebaseState = firebaseManager->getState();
        
        if (currentFirebaseState != lastFirebaseState) {
            if (currentFirebaseState == FirebaseState::FIREBASE_READY) {
                statusLED.setPixelColor(0, COLOR_READY);
                statusLED.show();
            } else if (currentFirebaseState == FirebaseState::ERROR_WIFI_FAILED ||
                       currentFirebaseState == FirebaseState::ERROR_AUTH_FAILED) {
                statusLED.setPixelColor(0, COLOR_ERROR);
                statusLED.show();
            } else {
                statusLED.setPixelColor(0, COLOR_CONNECTING);
                statusLED.show();
            }
            lastFirebaseState = currentFirebaseState;
        }
    }
    
    delay(10);
}
