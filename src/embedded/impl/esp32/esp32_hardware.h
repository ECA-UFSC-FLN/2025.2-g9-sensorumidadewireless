#ifndef ESP32_HARDWARE_H
#define ESP32_HARDWARE_H

#include "../../include/hal/hardware_interface.h"

class ESP32Hardware : public IHardware {
public:
    void delay(unsigned long ms) override;
    void deepSleep(unsigned long microseconds) override;
    unsigned long generateRandomNumber() override;
    unsigned long getCurrentTime() override;
    float readAnalog(uint8_t pin) override;
};

#endif // ESP32_HARDWARE_H