#ifndef I_PROTOCOL_DECODER_H
#define I_PROTOCOL_DECODER_H

#include <Arduino.h>
#include <IRrecv.h>

struct DecodedSignal {
    String protocol;
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
