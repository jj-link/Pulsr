/**
 * IR Loopback Hardware Test
 * 
 * Full round-trip validation: encode → transmit → receive → decode → compare.
 * Tests the entire IR pipeline through real hardware on a single board.
 * 
 * Setup:
 * - IR LED (GPIO 4) must be pointed at IR receiver (GPIO 5)
 * - Place them a few cm apart, or use a reflective surface
 * 
 * What this validates:
 * - IR LED circuit and driver are working
 * - IR receiver module (TSOP38238) is picking up signals
 * - IRLibProtocolEncoders produces valid IR timings
 * - ESP32IRTransmitter sends signals correctly
 * - ESP32SignalCapture receives and buffers signals
 * - IRLibProtocolDecoder correctly extracts address/command
 * - Full encode/decode symmetry through real hardware
 * 
 * Press BOOT button to run the next test. Tests cycle through:
 *   1. NEC (addr: 0x04, cmd: 0x08)
 *   2. NEC via sendNEC (raw 0xE817FF00 = addr: 0x00, cmd: 0x18)
 *   3. Samsung (addr: 0x0707, cmd: 0x02)
 * 
 * NeoPixel colors:
 *   Dim green  = idle/ready
 *   Purple     = transmitting
 *   Blue       = waiting for reception
 *   Green      = PASS (decoded matches sent)
 *   Red        = FAIL (mismatch or no signal)
 */

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include "config.h"
#include "receiver/ESP32SignalCapture.h"
#include "receiver/IRLibProtocolDecoder.h"
#include "transmitter/ESP32IRTransmitter.h"
#include "transmitter/IRLibProtocolEncoders.h"

// Hardware instances
ESP32SignalCapture signalCapture(IR_RECEIVE_PIN);
IRLibProtocolDecoder protocolDecoder;
ESP32IRTransmitter irTransmitter(IR_SEND_PIN, false);
IRLibProtocolEncoders protocolEncoder;
Adafruit_NeoPixel statusLED(NEOPIXEL_COUNT, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);

// LED colors
#define COLOR_IDLE         statusLED.Color(0, 10, 0)    // Dim green
#define COLOR_TRANSMIT     statusLED.Color(100, 0, 100) // Purple
#define COLOR_WAITING      statusLED.Color(0, 0, 100)   // Blue
#define COLOR_PASS         statusLED.Color(0, 100, 0)   // Green
#define COLOR_FAIL         statusLED.Color(100, 0, 0)   // Red

// Boot button
#define BOOT_BUTTON 0
bool lastButtonState = HIGH;

// Test tracking
uint8_t testIndex = 0;
uint8_t totalPass = 0;
uint8_t totalFail = 0;

// Wait for a signal with timeout (ms). Returns true if signal received.
bool waitForSignal(unsigned long timeoutMs) {
    unsigned long start = millis();
    while (millis() - start < timeoutMs) {
        decode_results results;
        if (signalCapture.decode(&results)) {
            return true;
        }
        delay(5);
    }
    return false;
}

// Run a single loopback test using the encoder pipeline (encode → raw → transmit → receive → decode)
void runEncoderLoopbackTest(const char* testName, const char* protocol,
                            uint32_t sendAddr, uint32_t sendCmd, uint16_t sendBits) {
    Serial.println("\n--------------------------------------------");
    Serial.print("[TEST] ");
    Serial.println(testName);
    Serial.print("  Send: protocol=");
    Serial.print(protocol);
    Serial.print(" addr=0x");
    Serial.print(sendAddr, HEX);
    Serial.print(" cmd=0x");
    Serial.print(sendCmd, HEX);
    Serial.print(" bits=");
    Serial.println(sendBits);
    
    // Step 1: Encode
    EncodedSignal encoded = protocolEncoder.encode(protocol, sendAddr, sendCmd, sendBits);
    if (!encoded.isKnownProtocol) {
        Serial.println("  [FAIL] Encoding failed");
        statusLED.setPixelColor(0, COLOR_FAIL);
        statusLED.show();
        totalFail++;
        return;
    }
    Serial.print("  Encoded: ");
    Serial.print(encoded.rawLength);
    Serial.print(" timings @ ");
    Serial.print(encoded.frequency);
    Serial.println(" kHz");
    
    // Step 2: Make sure receiver is ready
    signalCapture.resume();
    delay(50);
    
    // Step 3: Transmit
    statusLED.setPixelColor(0, COLOR_TRANSMIT);
    statusLED.show();
    
    TransmitResult txResult = irTransmitter.transmit(encoded.rawData, encoded.rawLength, encoded.frequency);
    
    // Clean up encoded data
    if (encoded.rawData) {
        delete[] encoded.rawData;
    }
    
    if (!txResult.success) {
        Serial.print("  [FAIL] Transmit error: ");
        Serial.println(txResult.errorMessage);
        statusLED.setPixelColor(0, COLOR_FAIL);
        statusLED.show();
        totalFail++;
        return;
    }
    Serial.println("  Transmitted OK");
    
    // Step 4: Wait for reception
    statusLED.setPixelColor(0, COLOR_WAITING);
    statusLED.show();
    
    unsigned long waitStart = millis();
    decode_results results;
    bool received = false;
    
    while (millis() - waitStart < 2000) {
        if (signalCapture.decode(&results)) {
            received = true;
            break;
        }
        delay(5);
    }
    
    if (!received) {
        Serial.println("  [FAIL] No signal received (timeout 2s)");
        Serial.println("  Check: Is IR LED pointed at receiver?");
        statusLED.setPixelColor(0, COLOR_FAIL);
        statusLED.show();
        totalFail++;
        return;
    }
    
    unsigned long rxTime = millis() - waitStart;
    Serial.print("  Received in ");
    Serial.print(rxTime);
    Serial.println(" ms");
    
    // Step 5: Decode
    DecodedSignal decoded = protocolDecoder.decode(&results);
    
    Serial.print("  Decoded: protocol=");
    Serial.print(decoded.protocol);
    Serial.print(" addr=0x");
    Serial.print(decoded.address, HEX);
    Serial.print(" cmd=0x");
    Serial.print(decoded.command, HEX);
    Serial.print(" value=0x");
    Serial.print((unsigned long)decoded.value, HEX);
    Serial.print(" bits=");
    Serial.print(decoded.bits);
    Serial.print(" known=");
    Serial.println(decoded.isKnownProtocol ? "yes" : "no");
    
    // Step 6: Compare
    bool protocolMatch = (strcmp(decoded.protocol, protocol) == 0);
    bool addressMatch = (decoded.address == sendAddr);
    bool commandMatch = (decoded.command == sendCmd);
    
    if (protocolMatch && addressMatch && commandMatch) {
        Serial.println("  [PASS] Full round-trip match!");
        statusLED.setPixelColor(0, COLOR_PASS);
        statusLED.show();
        totalPass++;
    } else {
        Serial.println("  [FAIL] Mismatch detected:");
        if (!protocolMatch) {
            Serial.print("    Protocol: expected=");
            Serial.print(protocol);
            Serial.print(" got=");
            Serial.println(decoded.protocol);
        }
        if (!addressMatch) {
            Serial.print("    Address: expected=0x");
            Serial.print(sendAddr, HEX);
            Serial.print(" got=0x");
            Serial.println(decoded.address, HEX);
        }
        if (!commandMatch) {
            Serial.print("    Command: expected=0x");
            Serial.print(sendCmd, HEX);
            Serial.print(" got=0x");
            Serial.println(decoded.command, HEX);
        }
        statusLED.setPixelColor(0, COLOR_FAIL);
        statusLED.show();
        totalFail++;
    }
}

// Run a loopback test using transmitNEC directly (bypasses our encoder)
void runDirectNECTest(const char* testName, uint32_t necData,
                      uint32_t expectAddr, uint32_t expectCmd) {
    Serial.println("\n--------------------------------------------");
    Serial.print("[TEST] ");
    Serial.println(testName);
    Serial.print("  Send NEC raw: 0x");
    Serial.println(necData, HEX);
    
    // Make sure receiver is ready
    signalCapture.resume();
    delay(50);
    
    // Transmit
    statusLED.setPixelColor(0, COLOR_TRANSMIT);
    statusLED.show();
    
    TransmitResult txResult = irTransmitter.transmitNEC(necData);
    if (!txResult.success) {
        Serial.print("  [FAIL] Transmit error: ");
        Serial.println(txResult.errorMessage);
        statusLED.setPixelColor(0, COLOR_FAIL);
        statusLED.show();
        totalFail++;
        return;
    }
    Serial.println("  Transmitted OK");
    
    // Wait for reception
    statusLED.setPixelColor(0, COLOR_WAITING);
    statusLED.show();
    
    unsigned long waitStart = millis();
    decode_results results;
    bool received = false;
    
    while (millis() - waitStart < 2000) {
        if (signalCapture.decode(&results)) {
            received = true;
            break;
        }
        delay(5);
    }
    
    if (!received) {
        Serial.println("  [FAIL] No signal received (timeout 2s)");
        Serial.println("  Check: Is IR LED pointed at receiver?");
        statusLED.setPixelColor(0, COLOR_FAIL);
        statusLED.show();
        totalFail++;
        return;
    }
    
    unsigned long rxTime = millis() - waitStart;
    Serial.print("  Received in ");
    Serial.print(rxTime);
    Serial.println(" ms");
    
    // Decode
    DecodedSignal decoded = protocolDecoder.decode(&results);
    
    Serial.print("  Decoded: protocol=");
    Serial.print(decoded.protocol);
    Serial.print(" addr=0x");
    Serial.print(decoded.address, HEX);
    Serial.print(" cmd=0x");
    Serial.print(decoded.command, HEX);
    Serial.print(" value=0x");
    Serial.print((unsigned long)decoded.value, HEX);
    Serial.print(" bits=");
    Serial.println(decoded.bits);
    
    // Compare
    bool addressMatch = (decoded.address == expectAddr);
    bool commandMatch = (decoded.command == expectCmd);
    bool isNEC = (strcmp(decoded.protocol, "NEC") == 0);
    
    if (isNEC && addressMatch && commandMatch) {
        Serial.println("  [PASS] Direct NEC round-trip match!");
        statusLED.setPixelColor(0, COLOR_PASS);
        statusLED.show();
        totalPass++;
    } else {
        Serial.println("  [FAIL] Mismatch detected:");
        if (!isNEC) {
            Serial.print("    Protocol: expected=NEC got=");
            Serial.println(decoded.protocol);
        }
        if (!addressMatch) {
            Serial.print("    Address: expected=0x");
            Serial.print(expectAddr, HEX);
            Serial.print(" got=0x");
            Serial.println(decoded.address, HEX);
        }
        if (!commandMatch) {
            Serial.print("    Command: expected=0x");
            Serial.print(expectCmd, HEX);
            Serial.print(" got=0x");
            Serial.println(decoded.command, HEX);
        }
        statusLED.setPixelColor(0, COLOR_FAIL);
        statusLED.show();
        totalFail++;
    }
}

void printSummary() {
    Serial.println("\n============================================");
    Serial.println("          TEST SUMMARY");
    Serial.println("============================================");
    Serial.print("  PASS: ");
    Serial.println(totalPass);
    Serial.print("  FAIL: ");
    Serial.println(totalFail);
    Serial.print("  TOTAL: ");
    Serial.println(totalPass + totalFail);
    Serial.println("============================================\n");
}

void runNextTest() {
    switch (testIndex) {
        case 0:
            // Test 1: NEC via encoder pipeline (full round-trip)
            runEncoderLoopbackTest(
                "NEC encoder pipeline (addr=0x04, cmd=0x08)",
                "NEC", 0x04, 0x08, 32
            );
            break;
            
        case 1:
            // Test 2: Direct NEC via sendNEC (bypasses our encoder)
            // NEC value: addr(8) | ~addr(8) | cmd(8) | ~cmd(8)
            // addr=0x00, cmd=0x18 → 0x00 | 0xFF<<8 | 0x18<<16 | 0xE7<<24 = 0xE718FF00
            runDirectNECTest(
                "NEC direct sendNEC (addr=0x00, cmd=0x18)",
                0xE718FF00, 0x00, 0x18
            );
            break;
            
        case 2:
            // Test 3: Samsung via encoder pipeline
            runEncoderLoopbackTest(
                "Samsung encoder pipeline (addr=0x0707, cmd=0x02)",
                "SAMSUNG", 0x0707, 0x02, 32
            );
            break;
            
        default:
            // All tests done, print summary
            printSummary();
            Serial.println("Press BOOT to restart tests...\n");
            testIndex = 0;
            totalPass = 0;
            totalFail = 0;
            return;
    }
    
    testIndex++;
    delay(1500);  // Wait between tests for LED visibility
    statusLED.setPixelColor(0, COLOR_IDLE);
    statusLED.show();
}

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("\n============================================");
    Serial.println("    Pulsr IR Loopback Hardware Test");
    Serial.println("============================================");
    Serial.print("IR Transmitter GPIO: ");
    Serial.println(IR_SEND_PIN);
    Serial.print("IR Receiver GPIO:    ");
    Serial.println(IR_RECEIVE_PIN);
    Serial.println("\nThis test sends IR signals and verifies");
    Serial.println("they are received and decoded correctly.");
    Serial.println("\nMake sure IR LED is pointed at the receiver!");
    Serial.println("============================================\n");
    
    // Initialize NeoPixel
    statusLED.begin();
    statusLED.setBrightness(NEOPIXEL_BRIGHTNESS);
    statusLED.setPixelColor(0, COLOR_IDLE);
    statusLED.show();
    
    // Initialize IR transmitter
    irTransmitter.begin();
    Serial.println("[OK] IR Transmitter initialized");
    
    // Initialize IR receiver
    signalCapture.enable();
    Serial.println("[OK] IR Receiver initialized");
    
    // Initialize boot button
    pinMode(BOOT_BUTTON, INPUT_PULLUP);
    
    Serial.println("\nPress BOOT to run each test:");
    Serial.println("  1. NEC encoder pipeline (addr=0x04, cmd=0x08)");
    Serial.println("  2. NEC direct sendNEC (addr=0x00, cmd=0x18)");
    Serial.println("  3. Samsung encoder pipeline (addr=0x0707, cmd=0x02)");
    Serial.println("  4. Print summary + reset\n");
}

void loop() {
    bool buttonState = digitalRead(BOOT_BUTTON);
    
    if (buttonState == LOW && lastButtonState == HIGH) {
        delay(50);  // Debounce
        if (digitalRead(BOOT_BUTTON) == LOW) {
            runNextTest();
        }
    }
    
    lastButtonState = buttonState;
    delay(10);
}
