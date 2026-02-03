/**
 * Pulsr - ESP32 IR Controller Firmware
 * 
 * This firmware handles:
 * - IR signal reception and decoding (Learning Mode)
 * - IR signal transmission (Playback Mode)
 * - Firebase Firestore sync for cloud control
 */

#include <Arduino.h>
#include <IRrecv.h>
#include <IRsend.h>
#include <IRutils.h>
#include <Adafruit_NeoPixel.h>

// Include config - copy config.example.h to config.h and fill in credentials
#include "config.h"

// ============== Hardware Instances ==============
IRrecv irReceiver(IR_RECEIVE_PIN);
IRsend irSender(IR_SEND_PIN);
decode_results irResults;

// NeoPixel for status indication
Adafruit_NeoPixel statusLED(NEOPIXEL_COUNT, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);

// ============== Color Definitions ==============
#define COLOR_OFF       statusLED.Color(0, 0, 0)
#define COLOR_READY     statusLED.Color(0, 20, 0)      // Dim green = ready
#define COLOR_LEARNING  statusLED.Color(0, 0, 100)     // Blue = listening
#define COLOR_SUCCESS   statusLED.Color(0, 100, 0)     // Green = success
#define COLOR_ERROR     statusLED.Color(100, 0, 0)     // Red = error/timeout
#define COLOR_SENDING   statusLED.Color(100, 50, 0)    // Orange = sending

// ============== State Management ==============
enum DeviceState {
    STATE_IDLE,
    STATE_LEARNING,
    STATE_SENDING
};

DeviceState currentState = STATE_IDLE;
unsigned long learningStartTime = 0;

// ============== Function Prototypes ==============
void enterLearningMode();
void exitLearningMode();
void handleLearningMode();
void processReceivedSignal(decode_results* results);
void setStatusColor(uint32_t color);
void blinkStatus(uint32_t color, int times, int delayMs = 200);

// ============== Setup ==============
void setup() {
    Serial.begin(115200);
    Serial.println("\n[Pulsr] Starting up...");

    // Initialize NeoPixel
    statusLED.begin();
    statusLED.setBrightness(NEOPIXEL_BRIGHTNESS); // Configurable in config.h
    setStatusColor(COLOR_OFF);

    // Initialize IR receiver
    irReceiver.enableIRIn();
    Serial.println("[Pulsr] IR Receiver initialized on GPIO " + String(IR_RECEIVE_PIN));

    // Initialize IR sender
    irSender.begin();
    Serial.println("[Pulsr] IR Sender initialized on GPIO " + String(IR_SEND_PIN));

    // Startup indication - rainbow flash
    blinkStatus(COLOR_SUCCESS, 2, 150);
    setStatusColor(COLOR_READY);
    Serial.println("[Pulsr] Ready! Type 'L' to enter learning mode.");
}

// ============== Main Loop ==============
void loop() {
    switch (currentState) {
        case STATE_IDLE:
            // TODO: Poll Firestore for mode changes or command queue
            // For now, check serial for manual testing
            if (Serial.available()) {
                char cmd = Serial.read();
                if (cmd == 'L' || cmd == 'l') {
                    enterLearningMode();
                }
            }
            break;

        case STATE_LEARNING:
            handleLearningMode();
            break;

        case STATE_SENDING:
            // TODO: Implement sending state
            currentState = STATE_IDLE;
            setStatusColor(COLOR_READY);
            break;
    }

    delay(10); // Small delay to prevent watchdog issues
}

// ============== Learning Mode Functions ==============

void enterLearningMode() {
    Serial.println("[Pulsr] Entering Learning Mode...");
    currentState = STATE_LEARNING;
    learningStartTime = millis();
    setStatusColor(COLOR_LEARNING); // Blue = listening
    irReceiver.resume(); // Make sure receiver is ready
}

void exitLearningMode() {
    Serial.println("[Pulsr] Exiting Learning Mode");
    currentState = STATE_IDLE;
    setStatusColor(COLOR_READY);
}

void handleLearningMode() {
    // Check for timeout
    if (millis() - learningStartTime > LEARNING_TIMEOUT_MS) {
        Serial.println("[Pulsr] Learning Mode timeout - no signal received");
        blinkStatus(COLOR_ERROR, 5, 100); // Fast red blink = timeout
        exitLearningMode();
        return;
    }

    // Check for received IR signal
    if (irReceiver.decode(&irResults)) {
        Serial.println("[Pulsr] Signal received!");
        processReceivedSignal(&irResults);
        blinkStatus(COLOR_SUCCESS, 3, 200); // Green blink = success
        exitLearningMode();
    }
}

void processReceivedSignal(decode_results* results) {
    Serial.println("========== CAPTURED IR SIGNAL ==========");
    
    // Protocol information
    String protocolName = typeToString(results->decode_type);
    Serial.println("Protocol: " + protocolName);
    
    if (results->decode_type != decode_type_t::UNKNOWN) {
        // Known protocol - extract structured data
        Serial.println("Address: 0x" + String(results->address, HEX));
        Serial.println("Command: 0x" + String(results->command, HEX));
        Serial.println("Value: 0x" + uint64ToString(results->value, HEX));
        Serial.println("Bits: " + String(results->bits));
    } else {
        // Unknown protocol - save raw timing
        Serial.println("Unknown protocol - capturing raw timing data");
    }

    // Always capture raw timing as fallback
    Serial.println("Raw Timing (" + String(results->rawlen) + " pulses):");
    Serial.print("rawData[");
    Serial.print(results->rawlen);
    Serial.print("] = {");
    for (uint16_t i = 1; i < results->rawlen; i++) {
        Serial.print(results->rawbuf[i] * kRawTick);
        if (i < results->rawlen - 1) Serial.print(", ");
    }
    Serial.println("};");
    
    Serial.println("=========================================");

    // TODO: Upload to Firestore
    // This will be implemented once we add WiFi/Firebase connectivity
}

// ============== NeoPixel Utility Functions ==============

void setStatusColor(uint32_t color) {
    statusLED.setPixelColor(0, color);
    statusLED.show();
}

void blinkStatus(uint32_t color, int times, int delayMs) {
    for (int i = 0; i < times; i++) {
        setStatusColor(color);
        delay(delayMs);
        setStatusColor(COLOR_OFF);
        delay(delayMs);
    }
}
