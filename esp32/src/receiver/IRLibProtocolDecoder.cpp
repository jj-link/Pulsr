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
    
    // NEC format: address(8) ~address(8) command(8) ~command(8)
    // Extract address from bits 24-31
    signal.address = (raw->value >> 24) & 0xFF;
    
    // Extract command from bits 8-15
    signal.command = (raw->value >> 8) & 0xFF;
    
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
    
    // Samsung format is similar to NEC: address(8) ~address(8) command(8) ~command(8)
    signal.address = (raw->value >> 24) & 0xFF;
    signal.command = (raw->value >> 8) & 0xFF;
    
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
    
    // Sony SIRC format varies (12, 15, or 20 bits)
    // Command is in the lower 7 bits
    signal.command = raw->value & 0x7F;
    
    // Address/device code depends on bit length
    if (raw->bits == 12) {
        // 12-bit: 7 bits command, 5 bits address
        signal.address = (raw->value >> 7) & 0x1F;
    } else if (raw->bits == 15) {
        // 15-bit: 7 bits command, 8 bits address
        signal.address = (raw->value >> 7) & 0xFF;
    } else if (raw->bits == 20) {
        // 20-bit: 7 bits command, 5 bits address, 8 bits extended
        signal.address = (raw->value >> 7) & 0x1F;
    } else {
        signal.address = 0;
    }
    
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
