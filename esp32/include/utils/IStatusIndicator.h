#ifndef I_STATUS_INDICATOR_H
#define I_STATUS_INDICATOR_H

#include <stdint.h>

class IStatusIndicator {
public:
    virtual ~IStatusIndicator() = default;
    
    virtual void begin() = 0;
    virtual void setColor(uint32_t color) = 0;
    virtual void blink(uint32_t color, int times, int delayMs = 200) = 0;
    virtual void off() = 0;
};

#endif
