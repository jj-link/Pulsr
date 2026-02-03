/**
 * Pulsr - ESP32 IR Controller Firmware (Production)
 * 
 * This firmware implements a loosely-coupled architecture with dependency injection.
 * For hardware testing, see src/hardware_tests/ir_decoder_test.cpp
 * 
 * Architecture:
 * - Uses interface abstractions for testability
 * - Follows SOLID principles
 * - Supports TDD with mock implementations
 */

#include <Arduino.h>
#include "config.h"
#include "abstractions/ISignalCapture.h"
#include "abstractions/IProtocolDecoder.h"
#include "abstractions/IStatusIndicator.h"
#include "abstractions/IIRTransmitter.h"

// TODO: Implement concrete classes
// - ESP32SignalCapture : ISignalCapture
// - IRLibProtocolDecoder : IProtocolDecoder
// - NeoPixelIndicator : IStatusIndicator
// - ESP32IRTransmitter : IIRTransmitter

// Dependency injection - interfaces will be assigned concrete implementations
ISignalCapture* signalCapture = nullptr;
IProtocolDecoder* protocolDecoder = nullptr;
IStatusIndicator* statusIndicator = nullptr;
IIRTransmitter* irTransmitter = nullptr;

// State management
enum DeviceState {
    STATE_IDLE,
    STATE_LEARNING,
    STATE_SENDING
};

DeviceState currentState = STATE_IDLE;

void setup() {
    Serial.begin(115200);
    Serial.println("\n[Pulsr Production] Starting up...");
    Serial.println("[Pulsr] NOTE: Concrete implementations not yet created.");
    Serial.println("[Pulsr] Use 'pio run -e ir_receiver_test' to run hardware test script.");
    
    // TODO: Instantiate concrete implementations and inject dependencies
    // signalCapture = new ESP32SignalCapture(IR_RECEIVE_PIN);
    // protocolDecoder = new IRLibProtocolDecoder();
    // statusIndicator = new NeoPixelIndicator(NEOPIXEL_PIN, NEOPIXEL_COUNT);
    // irTransmitter = new ESP32IRTransmitter(IR_SEND_PIN);
}

void loop() {
    // Production firmware loop will be implemented here
    // For now, just indicate this is a placeholder
    delay(1000);
}
