/**
 * IR Transmitter Hardware Test
 * 
 * Tests the IR transmitter hardware and protocol encoding.
 * Use this to validate:
 * - IR LED is wired correctly (with transistor/MOSFET driver)
 * - Signal transmission is working
 * - Protocol encoding is functional
 * - Commands can control IR devices
 * 
 * Upload this to ESP32-S3 and point the IR LED at a TV/device.
 * Press the boot button to transmit test commands.
 * The NeoPixel will indicate transmission status.
 * 
 * Test Commands:
 * - NEC: Samsung TV Power (addr: 0x07, cmd: 0x02)
 * - Samsung: Generic TV Volume Up (addr: 0x0707, cmd: 0x07)
 * - Sony: Sony TV Power (addr: 0x01, cmd: 0x15, 12-bit)
 */

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include "config.h"
#include "transmitter/ESP32IRTransmitter.h"
#include "transmitter/IRLibProtocolEncoders.h"

// Hardware instances
ESP32IRTransmitter irTransmitter(IR_SEND_PIN, false);
IRLibProtocolEncoders protocolEncoder;
Adafruit_NeoPixel statusLED(NEOPIXEL_COUNT, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);

// LED colors
#define COLOR_IDLE         statusLED.Color(0, 10, 0)    // Dim green
#define COLOR_TRANSMIT     statusLED.Color(100, 0, 100) // Purple
#define COLOR_SUCCESS      statusLED.Color(0, 100, 0)   // Green
#define COLOR_ERROR        statusLED.Color(100, 0, 0)   // Red

// Boot button (built-in on ESP32-S3)
#define BOOT_BUTTON 0
bool lastButtonState = HIGH;
uint8_t testMode = 0;  // Cycle through different test commands

void transmitTestSignal() {
    Serial.println("\n========== TRANSMITTING TEST SIGNAL ==========");
    
    statusLED.setPixelColor(0, COLOR_TRANSMIT);
    statusLED.show();
    
    EncodedSignal encoded;
    const char* protocol;
    uint32_t address;
    uint32_t command;
    uint16_t bits;
    
    // Cycle through different protocols
    switch (testMode % 3) {
        case 0:
            protocol = "NEC";
            address = 0x07;
            command = 0x02;
            bits = 32;
            Serial.println("Protocol: NEC (Samsung TV Power)");
            break;
            
        case 1:
            protocol = "SAMSUNG";
            address = 0x0707;
            command = 0x07;
            bits = 32;
            Serial.println("Protocol: SAMSUNG (Generic TV Volume Up)");
            break;
            
        case 2:
            protocol = "SONY";
            address = 0x01;
            command = 0x15;
            bits = 12;
            Serial.println("Protocol: SONY (Sony TV Power, 12-bit)");
            break;
    }
    
    Serial.print("Address:  0x");
    Serial.println(address, HEX);
    Serial.print("Command:  0x");
    Serial.println(command, HEX);
    Serial.print("Bits:     ");
    Serial.println(bits);
    
    // Encode the signal
    encoded = protocolEncoder.encode(protocol, address, command, bits);
    
    if (!encoded.isKnownProtocol) {
        Serial.println("[ERROR] Failed to encode signal");
        statusLED.setPixelColor(0, COLOR_ERROR);
        statusLED.show();
        delay(500);
        statusLED.setPixelColor(0, COLOR_IDLE);
        statusLED.show();
        return;
    }
    
    Serial.print("Encoded length: ");
    Serial.print(encoded.rawLength);
    Serial.println(" timings");
    Serial.print("Frequency: ");
    Serial.print(encoded.frequency);
    Serial.println(" kHz");
    
    // Transmit the signal
    TransmitResult result = irTransmitter.transmit(encoded.rawData, encoded.rawLength, encoded.frequency);
    
    // Clean up
    if (encoded.rawData) {
        delete[] encoded.rawData;
    }
    
    if (result.success) {
        Serial.println("[SUCCESS] Signal transmitted!");
        statusLED.setPixelColor(0, COLOR_SUCCESS);
        statusLED.show();
        delay(300);
    } else {
        Serial.print("[ERROR] Transmission failed: ");
        Serial.println(result.errorMessage);
        statusLED.setPixelColor(0, COLOR_ERROR);
        statusLED.show();
        delay(500);
    }
    
    Serial.println("==============================================\n");
    
    // Return to idle
    statusLED.setPixelColor(0, COLOR_IDLE);
    statusLED.show();
    
    testMode++;
}

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("\n========================================");
    Serial.println("  Pulsr IR Transmitter Hardware Test");
    Serial.println("========================================");
    Serial.print("IR Transmitter GPIO: ");
    Serial.println(IR_SEND_PIN);
    Serial.print("NeoPixel GPIO: ");
    Serial.println(NEOPIXEL_PIN);
    Serial.println("\nPress BOOT button to transmit test signals");
    Serial.println("Point IR LED at your TV/device\n");
    
    // Initialize NeoPixel
    statusLED.begin();
    statusLED.setBrightness(NEOPIXEL_BRIGHTNESS);
    statusLED.setPixelColor(0, COLOR_IDLE);
    statusLED.show();
    
    // Initialize IR transmitter
    irTransmitter.begin();
    
    // Initialize boot button
    pinMode(BOOT_BUTTON, INPUT_PULLUP);
    
    Serial.println("[READY] Press BOOT to send test signals...");
    Serial.println("Will cycle through: NEC -> SAMSUNG -> SONY\n");
}

void loop() {
    // Read boot button
    bool buttonState = digitalRead(BOOT_BUTTON);
    
    // Detect button press (falling edge with debounce)
    if (buttonState == LOW && lastButtonState == HIGH) {
        delay(50);  // Debounce
        if (digitalRead(BOOT_BUTTON) == LOW) {
            transmitTestSignal();
        }
    }
    
    lastButtonState = buttonState;
    delay(10);
}
