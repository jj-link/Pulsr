#include "receiver/IRLibProtocolDecoder.h"

#ifndef NATIVE_BUILD
    #include <Arduino.h>
#endif

DecodedSignal IRLibProtocolDecoder::decode(decode_results* raw) {
    if (!raw) {
        return decodeRaw(nullptr);
    }
    
    switch (raw->decode_type) {
        case NEC:
            return decodeNEC(raw);
        case SAMSUNG:
            return decodeSamsung(raw);
        case SONY:
            return decodeSony(raw);
        default:
            return decodeRaw(raw);
    }
}

DecodedSignal IRLibProtocolDecoder::decodeNEC(decode_results* raw) {
    DecodedSignal signal = {};  // Zero-initialize
    signal.protocol = "NEC";
    signal.isKnownProtocol = true;
    signal.value = raw->value;
    signal.bits = raw->bits;
    signal.rawTimings = nullptr;
    signal.rawLength = 0;
    
    // Use the library's already-extracted address and command
    signal.address = raw->address;
    signal.command = raw->command;
    
    return signal;
}

DecodedSignal IRLibProtocolDecoder::decodeSamsung(decode_results* raw) {
    DecodedSignal signal = {};  // Zero-initialize
    signal.protocol = "SAMSUNG";
    signal.isKnownProtocol = true;
    signal.value = raw->value;
    signal.bits = raw->bits;
    signal.rawTimings = nullptr;
    signal.rawLength = 0;
    
    // Use the library's already-extracted address and command
    signal.address = raw->address;
    signal.command = raw->command;
    
    return signal;
}

DecodedSignal IRLibProtocolDecoder::decodeSony(decode_results* raw) {
    DecodedSignal signal = {};  // Zero-initialize
    signal.protocol = "SONY";
    signal.isKnownProtocol = true;
    signal.value = raw->value;
    signal.bits = raw->bits;
    signal.rawTimings = nullptr;
    signal.rawLength = 0;
    
    // Use the library's already-extracted address and command
    signal.address = raw->address;
    signal.command = raw->command;
    
    return signal;
}

DecodedSignal IRLibProtocolDecoder::decodeRaw(decode_results* raw) {
    DecodedSignal signal = {};  // Zero-initialize
    signal.protocol = "RAW";
    signal.isKnownProtocol = false;
    signal.value = 0;
    signal.bits = 0;
    signal.address = 0;
    signal.command = 0;
    
    if (raw && raw->rawbuf && raw->rawlen > 0) {
        // Allocate memory for raw timing data
        signal.rawLength = raw->rawlen;
        signal.rawTimings = new uint16_t[raw->rawlen];
        
        // Copy raw timing data
        for (size_t i = 0; i < raw->rawlen; i++) {
            signal.rawTimings[i] = raw->rawbuf[i];
        }
    } else {
        signal.rawTimings = nullptr;
        signal.rawLength = 0;
    }
    
    return signal;
}
