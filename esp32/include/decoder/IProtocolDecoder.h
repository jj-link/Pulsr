#ifndef I_PROTOCOL_DECODER_H
#define I_PROTOCOL_DECODER_H

#ifdef NATIVE_BUILD
    // Native testing mode - use mocks
    #include "../test/mock_arduino.h"
#else
    // Embedded mode - use real Arduino/IRremoteESP8266
    #include <Arduino.h>
    #include <IRrecv.h>
#endif

struct DecodedSignal {
    const char* protocol;  // String literal: "NEC", "SAMSUNG", "SONY", "RAW"
    uint32_t address;
    uint32_t command;
    uint64_t value;
    uint16_t bits;
    uint16_t* rawTimings;
    size_t rawLength;
    bool isKnownProtocol;
};

class IProtocolDecoder {
public:
    virtual ~IProtocolDecoder() = default;
    
    virtual DecodedSignal decode(decode_results* raw) = 0;
};

#endif
