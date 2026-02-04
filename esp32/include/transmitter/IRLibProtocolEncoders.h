#ifndef IRLIB_PROTOCOL_ENCODERS_H
#define IRLIB_PROTOCOL_ENCODERS_H

#include "IProtocolEncoder.h"

class IRLibProtocolEncoders : public IProtocolEncoder {
public:
    IRLibProtocolEncoders() = default;
    ~IRLibProtocolEncoders() override = default;

    EncodedSignal encode(const char* protocol, uint32_t address, uint32_t command, uint16_t bits) override;
    EncodedSignal encodeRaw(uint16_t* rawData, uint16_t rawLength, uint16_t frequency = 38) override;

private:
    EncodedSignal encodeNEC(uint32_t address, uint32_t command);
    EncodedSignal encodeSamsung(uint32_t address, uint32_t command);
    EncodedSignal encodeSony(uint32_t address, uint32_t command, uint16_t bits);
    
    // Helper to allocate and copy raw data
    uint16_t* allocateRawData(uint16_t length);
};

#endif
