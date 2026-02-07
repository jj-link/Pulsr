/**
 * IR Native Samsung Hardware Test
 * 
 * Uses the IRremoteESP8266 library's built-in sendSamsung() directly
 * to test whether the IR LED hardware can control your Samsung TV.
 * 
 * This bypasses our custom ProtocolEncoders entirely — if the TV
 * responds to this but not the production firmware, the issue is
 * in our custom encoder timing.
 * 
 * Upload to ESP32-S3, point IR LED at TV, press BOOT button.
 * 
 * Command: Samsung TV Power (addr: 0x40BF, cmd: 0xE0)
 */

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include <IRsend.h>
#include "config.h"

// Hardware instances
IRsend irsend(IR_SEND_PIN);
Adafruit_NeoPixel statusLED(NEOPIXEL_COUNT, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);

// LED colors
#define COLOR_IDLE         statusLED.Color(0, 10, 0)    // Dim green
#define COLOR_TRANSMIT     statusLED.Color(100, 0, 100) // Purple
#define COLOR_SUCCESS      statusLED.Color(0, 100, 0)   // Green

// Boot button (built-in on ESP32-S3)
#define BOOT_BUTTON 0
bool lastButtonState = HIGH;
uint8_t pressCount = 0;

void transmitSamsungPower() {
    Serial.println("\n========== NATIVE SAMSUNG TRANSMIT ==========");
    Serial.print("Press #");
    Serial.println(pressCount + 1);
    
    statusLED.setPixelColor(0, COLOR_TRANSMIT);
    statusLED.show();
    
    // Samsung32 data format: [customer][~customer][data][~data]
    // customer=0x40, ~customer=0xBF, data=0xE0, ~data=0x1F
    // Full 32-bit word: 0x40BFE01F
    uint64_t samsungData = 0x40BFE01F;
    
    Serial.print("Data: 0x");
    Serial.println((unsigned long)samsungData, HEX);
    Serial.println("Method: IRsend::sendSAMSUNG() (library native)");
    
    irsend.sendSAMSUNG(samsungData, 32);
    
    Serial.println("[DONE] Signal sent!");
    statusLED.setPixelColor(0, COLOR_SUCCESS);
    statusLED.show();
    delay(500);
    
    statusLED.setPixelColor(0, COLOR_IDLE);
    statusLED.show();
    
    Serial.println("==============================================\n");
    pressCount++;
}

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("\n========================================");
    Serial.println("  Native Samsung IR Test");
    Serial.println("========================================");
    Serial.print("IR Send GPIO: ");
    Serial.println(IR_SEND_PIN);
    Serial.println("Command: Samsung TV Power");
    Serial.println("Data: 0x40BFE01F (32-bit)");
    Serial.println("\nPress BOOT button to transmit");
    Serial.println("========================================\n");
    
    // Initialize NeoPixel
    statusLED.begin();
    statusLED.setBrightness(NEOPIXEL_BRIGHTNESS);
    statusLED.setPixelColor(0, COLOR_IDLE);
    statusLED.show();
    
    // Initialize IR sender
    irsend.begin();
    
    // Initialize boot button
    pinMode(BOOT_BUTTON, INPUT_PULLUP);
}

void loop() {
    bool buttonState = digitalRead(BOOT_BUTTON);
    
    if (buttonState == LOW && lastButtonState == HIGH) {
        delay(50);  // Debounce
        if (digitalRead(BOOT_BUTTON) == LOW) {
            transmitSamsungPower();
        }
    }
    
    lastButtonState = buttonState;
    delay(10);
}
