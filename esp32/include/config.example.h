// Pulsr ESP32 Configuration
// Copy this file to config.h and fill in your credentials

#ifndef CONFIG_H
#define CONFIG_H

// WiFi Settings
#define WIFI_SSID "YOUR_WIFI_SSID"
#define WIFI_PASSWORD "YOUR_WIFI_PASSWORD"
#define WIFI_TIMEOUT_MS 10000  // 10 seconds

// Firebase Settings
#define FIREBASE_API_KEY "YOUR_FIREBASE_API_KEY"
#define FIREBASE_PROJECT_ID "YOUR_FIREBASE_PROJECT_ID"
#define FIREBASE_DATABASE_URL "https://YOUR_PROJECT_ID-default-rtdb.firebaseio.com"
#define FIREBASE_USER_EMAIL "YOUR_FIREBASE_USER_EMAIL"
#define FIREBASE_USER_PASSWORD "YOUR_FIREBASE_USER_PASSWORD"

// Device ID (unique identifier for this ESP32 unit)
#define DEVICE_ID "esp32-001"

// Hardware Pin Configuration
#define IR_RECEIVE_PIN 5    // GPIO for IR receiver (TSOP38238)
#define IR_SEND_PIN 4       // GPIO for IR LED transmitter
#define NEOPIXEL_PIN 48     // Built-in NeoPixel on ESP32-S3 DevKit
#define NEOPIXEL_COUNT 1    // Number of NeoPixels
#define NEOPIXEL_BRIGHTNESS 25  // 0-255, lower = dimmer (25 ≈ 10%)

// Timing Configuration
#define LEARNING_TIMEOUT_MS 30000  // 30 seconds timeout for learning mode

#endif // CONFIG_H
