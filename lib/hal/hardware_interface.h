#ifndef HARDWARE_INTERFACE_H
#define HARDWARE_INTERFACE_H

#include <stdint.h>

class IHardware {
public:
    virtual ~IHardware() = default;
    
    /**
     * @brief Delays execution for the specified number of milliseconds
     * @param ms Time to delay in milliseconds
     */
    virtual void delay(unsigned long ms) = 0;
    
    /**
     * @brief Puts the device into deep sleep mode
     * @param microseconds Time to sleep in microseconds
     */
    virtual void deepSleep(unsigned long microseconds) = 0;
    
    /**
     * @brief Generates a random number
     * @return Random unsigned long value
     */
    virtual unsigned long generateRandomNumber() = 0;
    
    /**
     * @brief Gets current time in milliseconds
     * @return Current time in milliseconds
     */
    virtual unsigned long getCurrentTime() = 0;
    
    /**
     * @brief Reads analog value from specified pin
     * @param pin Pin number to read from
     * @return Analog reading as float (normalized to voltage)
     */
    virtual float readAnalog(uint8_t pin) = 0;
};

#endif // HARDWARE_INTERFACE_H