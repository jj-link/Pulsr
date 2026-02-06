#include "transmitter/IRLibProtocolEncoders.h"

#ifndef NATIVE_BUILD
    #include <Arduino.h>
#endif

#include <cstring>

EncodedSignal IRLibProtocolEncoders::encode(const char* protocol, uint32_t address, uint32_t command, uint16_t bits) {
    // Route to appropriate encoder based on protocol
    if (strcmp(protocol, "NEC") == 0) {
        return encodeNEC(address, command);
    } else if (strcmp(protocol, "SAMSUNG") == 0) {
        return encodeSamsung(address, command);
    } else if (strcmp(protocol, "SONY") == 0) {
        return encodeSony(address, command, bits);
    }
    
    // Unknown protocol - return empty signal
    EncodedSignal signal = {};
    signal.protocol = "UNKNOWN";
    signal.rawData = nullptr;
    signal.rawLength = 0;
    signal.frequency = 38;
    signal.isKnownProtocol = false;
    return signal;
}

EncodedSignal IRLibProtocolEncoders::encodeRaw(uint16_t* rawData, uint16_t rawLength, uint16_t frequency) {
    EncodedSignal signal = {};
    signal.protocol = "RAW";
    signal.rawData = rawData;  // Direct reference (caller owns memory)
    signal.rawLength = rawLength;
    signal.frequency = frequency;
    signal.isKnownProtocol = false;
    return signal;
}

EncodedSignal IRLibProtocolEncoders::encodeNEC(uint32_t address, uint32_t command) {
    EncodedSignal signal = {};
    signal.protocol = "NEC";
    signal.frequency = 38;
    signal.isKnownProtocol = true;
    
    // NEC protocol timing (in microseconds)
    // Header: 9000us mark, 4500us space
    // Logical '0': 560us mark, 560us space
    // Logical '1': 560us mark, 1690us space
    // Footer: 560us mark
    
    // NEC sends: 8-bit address, 8-bit ~address, 8-bit command, 8-bit ~command
    // Total: 32 bits = 64 pulses (mark + space for each bit) + header (2) + footer (1) = 67 elements
    
    signal.rawLength = 67;
    signal.rawData = allocateRawData(signal.rawLength);
    
    if (!signal.rawData) {
        signal.isKnownProtocol = false;
        signal.rawLength = 0;
        return signal;
    }
    
    uint16_t idx = 0;
    
    // Header
    signal.rawData[idx++] = 9000;
    signal.rawData[idx++] = 4500;
    
    // Combine address and command into 32-bit value
    // NEC format: address (8 bits) | ~address (8 bits) | command (8 bits) | ~command (8 bits)
    uint32_t data = (address & 0xFF) | ((~address & 0xFF) << 8) | ((command & 0xFF) << 16) | ((~command & 0xFF) << 24);
    
    // Encode 32 bits (MSB first, matching IRremoteESP8266 convention)
    for (int i = 31; i >= 0; i--) {
        signal.rawData[idx++] = 560;  // Mark
        signal.rawData[idx++] = ((data >> i) & 1) ? 1690 : 560;  // Space (1690 for '1', 560 for '0')
    }
    
    // Footer mark
    signal.rawData[idx++] = 560;
    
    return signal;
}

EncodedSignal IRLibProtocolEncoders::encodeSamsung(uint32_t address, uint32_t command) {
    EncodedSignal signal = {};
    signal.protocol = "SAMSUNG";
    signal.frequency = 38;
    signal.isKnownProtocol = true;
    
    // Samsung protocol is similar to NEC but with different header timing
    // Header: 4500us mark, 4500us space
    // Logical '0': 560us mark, 560us space
    // Logical '1': 560us mark, 1690us space
    // Footer: 560us mark
    
    signal.rawLength = 67;
    signal.rawData = allocateRawData(signal.rawLength);
    
    if (!signal.rawData) {
        signal.isKnownProtocol = false;
        signal.rawLength = 0;
        return signal;
    }
    
    uint16_t idx = 0;
    
    // Samsung header
    signal.rawData[idx++] = 4500;
    signal.rawData[idx++] = 4500;
    
    // Combine address and command (IRremoteESP8266 format: address in upper 16 bits)
    uint32_t data = ((address & 0xFFFF) << 16) | ((command & 0xFF) << 8) | (~command & 0xFF);
    
    // Encode 32 bits (MSB first, matching IRremoteESP8266 convention)
    for (int i = 31; i >= 0; i--) {
        signal.rawData[idx++] = 560;
        signal.rawData[idx++] = ((data >> i) & 1) ? 1690 : 560;
    }
    
    // Footer
    signal.rawData[idx++] = 560;
    
    return signal;
}

EncodedSignal IRLibProtocolEncoders::encodeSony(uint32_t address, uint32_t command, uint16_t bits) {
    EncodedSignal signal = {};
    signal.protocol = "SONY";
    signal.frequency = 38;
    signal.isKnownProtocol = true;
    
    // Sony SIRC protocol timing
    // Header: 2400us mark
    // Logical '0': 600us space
    // Logical '1': 1200us space
    // Each bit ends with 600us mark
    
    // Sony format: 7-bit command | 5/8/13-bit address (depending on 12/15/20-bit variant)
    // For 12-bit: 7-bit command + 5-bit address = 12 bits
    
    signal.rawLength = (bits * 2) + 1;  // Each bit is mark+space, plus header mark
    signal.rawData = allocateRawData(signal.rawLength);
    
    if (!signal.rawData) {
        signal.isKnownProtocol = false;
        signal.rawLength = 0;
        return signal;
    }
    
    uint16_t idx = 0;
    
    // Sony header (just a mark, no space before first bit)
    signal.rawData[idx++] = 2400;
    
    // Combine command (7 bits) and address (5/8/13 bits depending on total bits)
    uint32_t data = (command & 0x7F) | ((address & ((1 << (bits - 7)) - 1)) << 7);
    
    // Encode bits (MSB first, matching IRremoteESP8266 convention)
    for (int i = bits - 1; i >= 0; i--) {
        signal.rawData[idx++] = (((data >> i) & 1)) ? 1200 : 600;  // Space
        signal.rawData[idx++] = 600;  // Mark
    }
    
    return signal;
}

uint16_t* IRLibProtocolEncoders::allocateRawData(uint16_t length) {
    // Allocate raw timing array
    // Note: In production, caller is responsible for freeing this memory
    uint16_t* data = new uint16_t[length];
    return data;
}
