#ifndef IRLIB_PROTOCOL_DECODER_H
#define IRLIB_PROTOCOL_DECODER_H

#include "IProtocolDecoder.h"

class IRLibProtocolDecoder : public IProtocolDecoder {
public:
    IRLibProtocolDecoder() = default;
    ~IRLibProtocolDecoder() override = default;
    
    DecodedSignal decode(decode_results* raw) override;

private:
    DecodedSignal decodeNEC(decode_results* raw);
    DecodedSignal decodeSamsung(decode_results* raw);
    DecodedSignal decodeSony(decode_results* raw);
    DecodedSignal decodeRaw(decode_results* raw);
};

#endif
