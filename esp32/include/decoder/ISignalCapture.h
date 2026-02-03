#ifndef I_SIGNAL_CAPTURE_H
#define I_SIGNAL_CAPTURE_H

#include <IRrecv.h>

class ISignalCapture {
public:
    virtual ~ISignalCapture() = default;
    
    virtual void enable() = 0;
    virtual void resume() = 0;
    virtual bool hasSignal() = 0;
    virtual bool decode(decode_results* results) = 0;
};

#endif
