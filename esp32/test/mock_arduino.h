#ifndef MOCK_ARDUINO_H
#define MOCK_ARDUINO_H

// Mock Arduino types for native testing
// This allows tests to run on your desktop instead of requiring ESP32 hardware

#include <cstdint>
#include <cstddef>

// Mock IRremoteESP8266 protocol type constants
#define NEC 1
#define SAMSUNG 2
#define SONY 3
#define UNKNOWN 0

// Mock decode_results structure from IRremoteESP8266
struct decode_results {
    int decode_type;      // Protocol type (NEC, SAMSUNG, SONY, UNKNOWN, etc.)
    uint64_t value;       // Decoded value
    uint16_t bits;        // Number of bits in the signal
    uint16_t* rawbuf;     // Raw timing data buffer
    size_t rawlen;        // Length of raw buffer
};

#endif
