#include "transmitter/ESP32IRTransmitter.h"

// Critical section mutex — disables interrupts on current core during IR send.
// IRremoteESP8266 uses software bit-banging (delayMicroseconds + digitalWrite),
// which is corrupted by WiFi interrupts. The critical section ensures accurate
// microsecond timing for the ~64ms IR signal duration.
static portMUX_TYPE irMux = portMUX_INITIALIZER_UNLOCKED;

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
    portENTER_CRITICAL(&irMux);
    irsend->sendRaw(rawData, length, frequency);
    portEXIT_CRITICAL(&irMux);
    
    result.success = true;
    result.errorMessage = "";
    return result;
}

TransmitResult ESP32IRTransmitter::transmitNEC(uint32_t data, uint16_t nbits) {
    TransmitResult result;
    
    // Send NEC protocol signal
    portENTER_CRITICAL(&irMux);
    irsend->sendNEC(data, nbits);
    portEXIT_CRITICAL(&irMux);
    
    result.success = true;
    result.errorMessage = "";
    return result;
}

TransmitResult ESP32IRTransmitter::transmitSamsung(uint64_t data, uint16_t nbits) {
    TransmitResult result;
    
    portENTER_CRITICAL(&irMux);
    irsend->sendSAMSUNG(data, nbits);
    portEXIT_CRITICAL(&irMux);
    
    result.success = true;
    result.errorMessage = "";
    return result;
}

TransmitResult ESP32IRTransmitter::transmitSony(uint32_t data, uint16_t nbits) {
    TransmitResult result;
    
    portENTER_CRITICAL(&irMux);
    irsend->sendSony(data, nbits);
    portEXIT_CRITICAL(&irMux);
    
    result.success = true;
    result.errorMessage = "";
    return result;
}
