#ifndef ESP32_SIGNAL_CAPTURE_H
#define ESP32_SIGNAL_CAPTURE_H

#include "ISignalCapture.h"
#include <IRrecv.h>

class ESP32SignalCapture : public ISignalCapture {
public:
    explicit ESP32SignalCapture(uint16_t pin, uint16_t bufferSize = 1024);
    ~ESP32SignalCapture() override;
    
    void enable() override;
    void disable() override;
    void resume() override;
    bool hasSignal() override;
    bool decode(decode_results* results) override;

private:
    IRrecv* irrecv;
};

#endif
