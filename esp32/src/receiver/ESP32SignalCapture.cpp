#include "receiver/ESP32SignalCapture.h"

ESP32SignalCapture::ESP32SignalCapture(uint16_t pin, uint16_t bufferSize) {
    irrecv = new IRrecv(pin, bufferSize, 50, true);
}

ESP32SignalCapture::~ESP32SignalCapture() {
    delete irrecv;
}

void ESP32SignalCapture::enable() {
    irrecv->enableIRIn();
}

void ESP32SignalCapture::resume() {
    irrecv->resume();
}

bool ESP32SignalCapture::hasSignal() {
    return irrecv->decode(nullptr);
}

bool ESP32SignalCapture::decode(decode_results* results) {
    return irrecv->decode(results);
}
