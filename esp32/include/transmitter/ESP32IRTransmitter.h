#ifndef ESP32_IR_TRANSMITTER_H
#define ESP32_IR_TRANSMITTER_H

#include "IIRTransmitter.h"
#include <IRsend.h>

class ESP32IRTransmitter : public IIRTransmitter {
public:
    explicit ESP32IRTransmitter(uint16_t pin, bool inverted = false);
    ~ESP32IRTransmitter() override;

    void begin() override;
    TransmitResult transmit(uint16_t* rawData, uint16_t length, uint16_t frequency = 38) override;
    TransmitResult transmitNEC(uint32_t data, uint16_t nbits = 32) override;

private:
    IRsend* irsend;
    uint16_t pin;
    bool inverted;
};

#endif
