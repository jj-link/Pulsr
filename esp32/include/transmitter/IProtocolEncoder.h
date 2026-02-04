#ifndef I_PROTOCOL_ENCODER_H
#define I_PROTOCOL_ENCODER_H

#ifdef NATIVE_BUILD
    // Native testing mode - use mocks
    #include "../test/mock_arduino.h"
#else
    // Embedded mode - use real Arduino/IRremoteESP8266
    #include <Arduino.h>
    #include <IRsend.h>
#endif

#include <cstdint>
#include <cstddef>

// Encoded signal structure for transmission
struct EncodedSignal {
    const char* protocol;      // Protocol name (NEC, SAMSUNG, SONY, RAW)
    uint16_t* rawData;         // Raw timing data (microseconds)
    uint16_t rawLength;        // Length of raw data array
    uint16_t frequency;        // Carrier frequency in kHz (default 38)
    bool isKnownProtocol;      // True if protocol-specific encoding was used
};

class IProtocolEncoder {
public:
    virtual ~IProtocolEncoder() = default;
    
    // Encode a signal from protocol parameters
    virtual EncodedSignal encode(const char* protocol, uint32_t address, uint32_t command, uint16_t bits) = 0;
    
    // Encode from raw timing data (for unknown protocols)
    virtual EncodedSignal encodeRaw(uint16_t* rawData, uint16_t rawLength, uint16_t frequency = 38) = 0;
};

#endif
