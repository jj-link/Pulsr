/**
 * Pulsr - ESP32 IR Controller Production Embedded Software
 * 
 * This embedded software implements a cloud-connected IR controller with:
 * - IR signal learning and decoding (receiver)
 * - IR signal transmission from Firestore queue (transmitter)
 * - Firestore integration for command storage and queue processing
 * - Real-time learning mode control from web UI
 * 
 * Architecture:
 * - Uses interface abstractions for testability
 * - Dependency injection for loose coupling
 * - Callback-based event system
 */

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include "config.h"

// Receiver components
#include "receiver/ESP32SignalCapture.h"
#include "receiver/IRLibProtocolDecoder.h"
#include "receiver/LearningStateMachine.h"

// Transmitter components
#include "transmitter/ESP32IRTransmitter.h"
#include "transmitter/IRLibProtocolEncoders.h"
#include "transmitter/QueueProcessor.h"

// Firebase integration
#include "utils/FirebaseManager.h"

// Firebase helper includes (must be after FirebaseManager)
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"

// ============== Hardware Instances ==============

// Receiver subsystem
ESP32SignalCapture signalCapture(IR_RECEIVE_PIN);
IRLibProtocolDecoder protocolDecoder;
LearningStateMachine learningStateMachine(&signalCapture, &protocolDecoder, LEARNING_TIMEOUT_MS);

// Transmitter subsystem
ESP32IRTransmitter irTransmitter(IR_SEND_PIN, false);  // GPIO 4, not inverted
IRLibProtocolEncoders protocolEncoder;
FirebaseData queueFbdo;  // Separate FirebaseData instance for queue polling

// Firebase integration
FirebaseManager firebaseManager(
    WIFI_SSID,
    WIFI_PASSWORD,
    FIREBASE_API_KEY,
    FIREBASE_PROJECT_ID,
    FIREBASE_USER_EMAIL,
    FIREBASE_USER_PASSWORD,
    DEVICE_ID
);

// Queue processor (initialized in setup after Firebase is ready)
QueueProcessor* queueProcessor = nullptr;

// Status LED
Adafruit_NeoPixel statusLED(NEOPIXEL_COUNT, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);

// ============== Color Definitions ==============
#define COLOR_OFF           statusLED.Color(0, 0, 0)
#define COLOR_CONNECTING    statusLED.Color(20, 20, 0)   // Yellow = connecting
#define COLOR_READY         statusLED.Color(0, 20, 0)    // Dim green = ready
#define COLOR_LEARNING      statusLED.Color(0, 0, 100)   // Blue = listening
#define COLOR_SUCCESS       statusLED.Color(0, 100, 0)   // Green = success
#define COLOR_ERROR         statusLED.Color(100, 0, 0)   // Red = error
#define COLOR_TIMEOUT       statusLED.Color(100, 50, 0)  // Orange = timeout
#define COLOR_TX_PROCESSING statusLED.Color(80, 0, 80)   // Purple = processing queue item
#define COLOR_TX_SUCCESS    statusLED.Color(0, 100, 50)   // Cyan-green = transmit success
#define COLOR_TX_FAILED     statusLED.Color(100, 20, 0)   // Red-orange = transmit failed

// ============== Callback Handlers ==============

void onLearningStateChanged(LearningState state) {
    Serial.print("[Learning] State changed: ");
    
    switch (state) {
        case LearningState::IDLE:
            Serial.println("IDLE");
            statusLED.setPixelColor(0, COLOR_READY);
            statusLED.show();
            // Update Firestore to indicate learning complete
            firebaseManager.setLearningMode(false);
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
    
    // Upload to Firestore
    String commandName = String("cmd_") + String(millis());
    if (firebaseManager.uploadSignal(signal, commandName)) {
        Serial.println("[Main] Signal uploaded to Firestore successfully!");
    } else {
        Serial.println("[Main] Failed to upload signal to Firestore");
    }
}

// Track when to revert LED back to ready after transmit flash
unsigned long txLedRevertTime = 0;

void onTransmissionEvent(TransmissionStatus status, const String& protocol, const String& commandId) {
    switch (status) {
        case TransmissionStatus::PROCESSING:
            Serial.print("[TX] Processing command: ");
            Serial.println(commandId);
            statusLED.setPixelColor(0, COLOR_TX_PROCESSING);
            statusLED.show();
            break;
            
        case TransmissionStatus::COMPLETED:
            Serial.print("[TX] Transmitted OK: ");
            Serial.print(protocol);
            Serial.print(" cmd=");
            Serial.println(commandId);
            statusLED.setPixelColor(0, COLOR_TX_SUCCESS);
            statusLED.show();
            txLedRevertTime = millis() + 500;  // Flash for 500ms
            break;
            
        case TransmissionStatus::FAILED:
            Serial.print("[TX] Failed: ");
            Serial.println(commandId);
            statusLED.setPixelColor(0, COLOR_TX_FAILED);
            statusLED.show();
            txLedRevertTime = millis() + 1000;  // Flash for 1s
            break;
            
        default:
            break;
    }
}

void onFirebaseLearningModeChanged(bool isLearning) {
    Serial.print("[Firebase] Learning mode changed: ");
    Serial.println(isLearning ? "ON" : "OFF");
    
    if (isLearning) {
        learningStateMachine.startLearning();
    } else {
        learningStateMachine.stopLearning();
    }
}

// ============== Setup ==============

void setup() {
    Serial.begin(115200);
    Serial.println("\n[Pulsr] Starting up...");
    Serial.print("[Pulsr] Device ID: ");
    Serial.println(DEVICE_ID);
    
    // Initialize NeoPixel
    statusLED.begin();
    statusLED.setBrightness(NEOPIXEL_BRIGHTNESS);
    statusLED.setPixelColor(0, COLOR_CONNECTING);
    statusLED.show();
    
    // Initialize IR receiver
    signalCapture.enable();
    Serial.print("[Pulsr] IR Receiver initialized on GPIO ");
    Serial.println(IR_RECEIVE_PIN);
    
    // Initialize IR transmitter
    irTransmitter.begin();
    Serial.print("[Pulsr] IR Transmitter initialized on GPIO ");
    Serial.println(IR_SEND_PIN);
    
    // Set up callbacks
    learningStateMachine.onStateChange(onLearningStateChanged);
    learningStateMachine.onSignalCapture(onSignalCaptured);
    firebaseManager.onLearningStateChange(onFirebaseLearningModeChanged);
    
    // Connect to Firebase
    Serial.println("[Pulsr] Connecting to Firebase...");
    if (firebaseManager.begin()) {
        Serial.println("[Pulsr] Firebase connection initiated");
        
        // Initialize queue processor after Firebase is ready
        queueProcessor = new QueueProcessor(
            &queueFbdo,
            FIREBASE_PROJECT_ID,
            DEVICE_ID,
            &protocolEncoder,
            &irTransmitter,
            100  // 100ms poll interval
        );
        queueProcessor->onTransmissionEvent(onTransmissionEvent);
        Serial.println("[Pulsr] Queue processor initialized");
    } else {
        Serial.println("[Pulsr] Firebase connection failed - will retry");
        statusLED.setPixelColor(0, COLOR_ERROR);
        statusLED.show();
    }
    
    Serial.println("[Pulsr] Initialization complete!");
}

// ============== Main Loop ==============

void loop() {
    // Update Firebase connection and poll for learning mode changes
    firebaseManager.update();
    
    // Update learning state machine (handles timeouts and signal capture)
    learningStateMachine.update();
    
    // Update queue processor (polls Firestore queue and transmits commands)
    if (queueProcessor != nullptr && firebaseManager.getState() == FirebaseState::FIREBASE_READY) {
        queueProcessor->update();
    }
    
    // Revert transmit LED flash back to ready color after timeout
    if (txLedRevertTime > 0 && millis() >= txLedRevertTime) {
        statusLED.setPixelColor(0, COLOR_READY);
        statusLED.show();
        txLedRevertTime = 0;
    }
    
    // Update status LED based on Firebase state
    static FirebaseState lastFirebaseState = FirebaseState::DISCONNECTED;
    FirebaseState currentFirebaseState = firebaseManager.getState();
    
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
    
    delay(10);
}
