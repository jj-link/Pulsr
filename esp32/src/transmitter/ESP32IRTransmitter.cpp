#include "transmitter/ESP32IRTransmitter.h"

ESP32IRTransmitter::ESP32IRTransmitter(uint16_t pin, bool inverted) 
    : pin(pin), inverted(inverted) {
    irsend = new IRsend(pin, inverted);
}

ESP32IRTransmitter::~ESP32IRTransmitter() {
    delete irsend;
}

void ESP32IRTransmitter::begin() {
    irsend->begin();
}

TransmitResult ESP32IRTransmitter::transmit(uint16_t* rawData, uint16_t length, uint16_t frequency) {
    TransmitResult result;
    
    if (!rawData || length == 0) {
        result.success = false;
        result.errorMessage = "Invalid raw data";
        return result;
    }
    
    // Send raw IR signal
    // IRsend::sendRaw expects: rawData, length, frequency (in kHz)
    irsend->sendRaw(rawData, length, frequency);
    
    result.success = true;
    result.errorMessage = "";
    return result;
}

TransmitResult ESP32IRTransmitter::transmitNEC(uint32_t data, uint16_t nbits) {
    TransmitResult result;
    
    // Send NEC protocol signal
    irsend->sendNEC(data, nbits);
    
    result.success = true;
    result.errorMessage = "";
    return result;
}

TransmitResult ESP32IRTransmitter::transmitSamsung(uint64_t data, uint16_t nbits) {
    TransmitResult result;
    
    irsend->sendSAMSUNG(data, nbits);
    
    result.success = true;
    result.errorMessage = "";
    return result;
}

TransmitResult ESP32IRTransmitter::transmitSony(uint32_t data, uint16_t nbits) {
    TransmitResult result;
    
    irsend->sendSony(data, nbits);
    
    result.success = true;
    result.errorMessage = "";
    return result;
}
