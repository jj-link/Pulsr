/**
 * IR Receiver Hardware Test
 * 
 * Tests the IR receiver hardware and protocol decoding.
 * Use this to validate:
 * - IR receiver module (TSOP38238) is wired correctly
 * - Signal capture is working
 * - Protocol decoding is functional
 * - NeoPixel status LED is working
 * 
 * Upload this to ESP32-S3 and point your TV/AC remote at the receiver.
 * The NeoPixel will flash and serial output will show decoded signals.
 */

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include "config.h"
#include "receiver/ESP32SignalCapture.h"
#include "receiver/IRLibProtocolDecoder.h"

// Hardware instances
ESP32SignalCapture signalCapture(IR_RECEIVE_PIN);
IRLibProtocolDecoder protocolDecoder;
Adafruit_NeoPixel statusLED(NEOPIXEL_COUNT, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);

// LED colors
#define COLOR_IDLE      statusLED.Color(0, 10, 0)    // Dim green
#define COLOR_SIGNAL    statusLED.Color(0, 0, 100)   // Blue flash
#define COLOR_SUCCESS   statusLED.Color(0, 100, 0)   // Bright green
#define COLOR_UNKNOWN   statusLED.Color(100, 50, 0)  // Orange

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("\n========================================");
    Serial.println("   Pulsr IR Receiver Hardware Test");
    Serial.println("========================================");
    Serial.print("IR Receiver GPIO: ");
    Serial.println(IR_RECEIVE_PIN);
    Serial.print("NeoPixel GPIO: ");
    Serial.println(NEOPIXEL_PIN);
    Serial.println("\nPoint your remote at the IR receiver...\n");
    
    // Initialize NeoPixel
    statusLED.begin();
    statusLED.setBrightness(NEOPIXEL_BRIGHTNESS);
    statusLED.setPixelColor(0, COLOR_IDLE);
    statusLED.show();
    
    // Initialize IR receiver
    signalCapture.enable();
    
    Serial.println("[READY] Waiting for IR signals...");
}

void loop() {
    // Check if a signal has been captured
    decode_results results;
    if (signalCapture.decode(&results)) {
        // Flash LED blue to indicate signal received
        statusLED.setPixelColor(0, COLOR_SIGNAL);
        statusLED.show();
        
        // Decode the signal
        DecodedSignal decoded = protocolDecoder.decode(&results);
        
        // Print results
        Serial.println("\n========== IR SIGNAL CAPTURED ==========");
        Serial.print("Protocol: ");
        Serial.println(decoded.protocol);
        
        if (decoded.isKnownProtocol) {
            Serial.print("Address:  0x");
            Serial.print(decoded.address, HEX);
            Serial.print(" (");
            Serial.print(decoded.address);
            Serial.println(")");
            
            Serial.print("Command:  0x");
            Serial.print(decoded.command, HEX);
            Serial.print(" (");
            Serial.print(decoded.command);
            Serial.println(")");
            
            Serial.print("Value:    0x");
            Serial.println((unsigned long)decoded.value, HEX);
            
            Serial.print("Bits:     ");
            Serial.println(decoded.bits);
            
            // Flash green for known protocol
            statusLED.setPixelColor(0, COLOR_SUCCESS);
            statusLED.show();
            delay(200);
        } else {
            Serial.print("Raw data length: ");
            Serial.println(decoded.rawLength);
            Serial.println("(Unknown protocol - raw timings captured)");
            
            // Flash orange for unknown protocol
            statusLED.setPixelColor(0, COLOR_UNKNOWN);
            statusLED.show();
            delay(200);
        }
        
        Serial.println("========================================\n");
        
        // Return to idle color
        statusLED.setPixelColor(0, COLOR_IDLE);
        statusLED.show();
        
        // Resume receiving
        signalCapture.resume();
    }
    
    delay(10);
}
