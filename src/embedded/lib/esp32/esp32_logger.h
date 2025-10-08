#ifndef ESP32_LOGGER_H
#define ESP32_LOGGER_H

#include "../utils/logger_interface.h"

class ESP32Logger : public ILogger {
public:
    ESP32Logger(unsigned long baudRate = 115200);
    void debug(const char* message) override;
    void info(const char* message) override;
    void error(const char* message) override;
    void printf(const char* format, ...) override;
};

#endif // ESP32_LOGGER_H