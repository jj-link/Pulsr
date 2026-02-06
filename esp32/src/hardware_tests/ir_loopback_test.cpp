/**
 * IR Loopback Hardware Test
 * 
 * Full round-trip validation: encode -> transmit -> receive -> decode -> compare.
 * Tests the entire IR pipeline through real hardware on a single board.
 * 
 * Setup:
 *   IR LED (GPIO 4) must be pointed at IR receiver (GPIO 5)
 * 
 * Usage:
 *   1. Upload and open serial monitor
 *   2. Press any key in the serial monitor to start
 *   3. All tests run automatically, results printed to serial
 *   4. Press any key again to re-run
 */

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include <cstring>
#include "config.h"
#include "receiver/ESP32SignalCapture.h"
#include "receiver/IRLibProtocolDecoder.h"
#include "transmitter/ESP32IRTransmitter.h"

// Hardware instances
ESP32SignalCapture signalCapture(IR_RECEIVE_PIN);
IRLibProtocolDecoder protocolDecoder;
ESP32IRTransmitter irTransmitter(IR_SEND_PIN, false);
Adafruit_NeoPixel statusLED(NEOPIXEL_COUNT, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);

// LED colors
#define COLOR_IDLE    statusLED.Color(0, 10, 0)    // Dim green = ready
#define COLOR_RUNNING statusLED.Color(0, 0, 100)   // Blue = test running
#define COLOR_PASS    statusLED.Color(0, 100, 0)   // Green = all passed
#define COLOR_FAIL    statusLED.Color(100, 0, 0)   // Red = something failed

// Test tracking
uint8_t totalPass = 0;
uint8_t totalFail = 0;

// NEC/Samsung protocols send bits LSB first on the wire, but sendNEC/sendSAMSUNG
// transmit data MSB first. The library's decoder bit-reverses each byte when
// extracting address/command. We must pre-reverse to compensate.
uint8_t reverseByte(uint8_t b) {
    b = (b & 0xF0) >> 4 | (b & 0x0F) << 4;
    b = (b & 0xCC) >> 2 | (b & 0x33) << 2;
    b = (b & 0xAA) >> 1 | (b & 0x55) << 1;
    return b;
}

// Run a single loopback test using protocol-specific library senders
void runLoopbackTest(const char* protocol, uint32_t sendAddr, uint32_t sendCmd, uint16_t sendBits) {
    Serial.print("\n  [");
    Serial.print(protocol);
    Serial.print("] addr=0x");
    Serial.print(sendAddr, HEX);
    Serial.print(" cmd=0x");
    Serial.print(sendCmd, HEX);
    Serial.print(" ... ");
    
    // Drain any stale/noise signals from the receiver buffer
    {
        decode_results dummy;
        while (signalCapture.decode(&dummy)) {
            signalCapture.resume();
            delay(10);
        }
    }
    
    // Prepare receiver fresh, then transmit immediately (minimize noise window)
    signalCapture.resume();
    Serial.flush();  // Ensure serial writes complete before IR timing-critical code
    delay(10);
    
    // Transmit using the library's protocol-specific sender
    TransmitResult txResult;
    if (strcmp(protocol, "NEC") == 0) {
        // NEC: reverse each byte for LSB-first wire format
        uint8_t rAddr = reverseByte(sendAddr & 0xFF);
        uint8_t rCmd = reverseByte(sendCmd & 0xFF);
        uint32_t data = ((uint32_t)rAddr << 24) | ((uint32_t)(uint8_t)~rAddr << 16) |
                        ((uint32_t)rCmd << 8) | (uint8_t)~rCmd;
        txResult = irTransmitter.transmitNEC(data, sendBits);
    } else if (strcmp(protocol, "SAMSUNG") == 0) {
        // Samsung: reverse each byte for LSB-first wire format
        uint8_t rCust = reverseByte(sendAddr & 0xFF);
        uint8_t rCmd = reverseByte(sendCmd & 0xFF);
        uint64_t data = ((uint64_t)rCust << 24) | ((uint64_t)rCust << 16) |
                        ((uint64_t)rCmd << 8) | (uint8_t)~rCmd;
        txResult = irTransmitter.transmitSamsung(data, sendBits);
    } else if (strcmp(protocol, "SONY") == 0) {
        // Sony data format: command(7) | address(5/8/13)
        uint32_t data = (sendCmd & 0x7F) |
                        ((sendAddr & ((1 << (sendBits - 7)) - 1)) << 7);
        txResult = irTransmitter.transmitSony(data, sendBits);
    } else {
        Serial.println("FAIL (unknown protocol)");
        totalFail++;
        return;
    }
    
    if (!txResult.success) {
        Serial.println("FAIL (transmit error)");
        totalFail++;
        return;
    }
    
    // Wait for reception
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
        Serial.println("FAIL (no signal received - check IR LED aim)");
        totalFail++;
        return;
    }
    
    // Decode
    DecodedSignal decoded = protocolDecoder.decode(&results);
    
    // Compare
    bool protocolMatch = (strcmp(decoded.protocol, protocol) == 0);
    bool addressMatch = (decoded.address == sendAddr);
    bool commandMatch = (decoded.command == sendCmd);
    
    if (protocolMatch && addressMatch && commandMatch) {
        unsigned long rxTime = millis() - waitStart;
        Serial.print("PASS (");
        Serial.print(rxTime);
        Serial.println("ms)");
        totalPass++;
    } else {
        Serial.println("FAIL");
        Serial.print("    Sent:     protocol=");
        Serial.print(protocol);
        Serial.print(" addr=0x");
        Serial.print(sendAddr, HEX);
        Serial.print(" cmd=0x");
        Serial.println(sendCmd, HEX);
        Serial.print("    Received: protocol=");
        Serial.print(decoded.protocol);
        Serial.print(" addr=0x");
        Serial.print(decoded.address, HEX);
        Serial.print(" cmd=0x");
        Serial.println(decoded.command, HEX);
        Serial.print("    Raw: decode_type=");
        Serial.print(results.decode_type);
        Serial.print(" value=0x");
        Serial.print((unsigned long)results.value, HEX);
        Serial.print(" rawlen=");
        Serial.println(results.rawlen);
        totalFail++;
    }
}

void runAllTests() {
    totalPass = 0;
    totalFail = 0;
    
    statusLED.setPixelColor(0, COLOR_RUNNING);
    statusLED.show();
    
    Serial.println("\n============================================");
    Serial.println("    Pulsr IR Loopback Test");
    Serial.println("============================================");
    Serial.print("  TX GPIO: ");
    Serial.print(IR_SEND_PIN);
    Serial.print("  |  RX GPIO: ");
    Serial.println(IR_RECEIVE_PIN);
    Serial.println("--------------------------------------------");
    
    // Test 1: NEC (known-good value from earlier test)
    runLoopbackTest("NEC", 0x00, 0x18, 32);
    delay(1000);
    
    // Test 2: NEC
    runLoopbackTest("NEC", 0x04, 0x08, 32);
    delay(1000);
    
    // Test 3: Samsung (8-bit customer code, repeated in 32-bit signal)
    runLoopbackTest("SAMSUNG", 0x07, 0x02, 32);
    delay(1000);
    
    // Test 4: NEC
    runLoopbackTest("NEC", 0x07, 0x02, 32);
    delay(1000);
    
    // Summary
    Serial.println("\n============================================");
    Serial.print("  Results: ");
    Serial.print(totalPass);
    Serial.print(" passed, ");
    Serial.print(totalFail);
    Serial.print(" failed (");
    Serial.print(totalPass + totalFail);
    Serial.println(" total)");
    Serial.println("============================================");
    
    if (totalFail == 0) {
        Serial.println("  All tests PASSED!");
        statusLED.setPixelColor(0, COLOR_PASS);
    } else {
        Serial.println("  Some tests FAILED.");
        statusLED.setPixelColor(0, COLOR_FAIL);
    }
    statusLED.show();
    
    Serial.println("\n  Press any key to run again...\n");
}

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    // Initialize NeoPixel
    statusLED.begin();
    statusLED.setBrightness(NEOPIXEL_BRIGHTNESS);
    statusLED.setPixelColor(0, COLOR_IDLE);
    statusLED.show();
    
    // Initialize IR hardware
    irTransmitter.begin();
    signalCapture.enable();
    
    Serial.println("\n============================================");
    Serial.println("    Pulsr IR Loopback Hardware Test");
    Serial.println("============================================");
    Serial.println("  Make sure IR LED is pointed at receiver.");
    Serial.println("  Press any key to start...");
    Serial.println("============================================\n");
}

void loop() {
    if (Serial.available() > 0) {
        // Consume all buffered input
        while (Serial.available()) Serial.read();
        
        runAllTests();
    }
    delay(10);
}
