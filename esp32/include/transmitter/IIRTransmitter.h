#ifndef I_IR_TRANSMITTER_H
#define I_IR_TRANSMITTER_H

#include <Arduino.h>
#include <IRsend.h>

struct TransmitResult {
    bool success;
    String errorMessage;
};

class IIRTransmitter {
public:
    virtual ~IIRTransmitter() = default;
    
    virtual void begin() = 0;
    virtual TransmitResult transmit(uint16_t* rawData, uint16_t length, uint16_t frequency = 38) = 0;
    virtual TransmitResult transmitNEC(uint32_t data, uint16_t nbits = 32) = 0;
};

#endif
