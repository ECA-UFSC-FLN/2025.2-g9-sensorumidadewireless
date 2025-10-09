#ifndef LOGGER_INTERFACE_H
#define LOGGER_INTERFACE_H

#include <stdarg.h>

class ILogger {
public:
    virtual ~ILogger() = default;
    
    /**
     * @brief Log debug message
     * @param message Message to log
     */
    virtual void debug(const char* message) = 0;
    
    /**
     * @brief Log info message
     * @param message Message to log
     */
    virtual void info(const char* message) = 0;
    
    /**
     * @brief Log error message
     * @param message Message to log
     */
    virtual void error(const char* message) = 0;
    
    /**
     * @brief Printf-style logging
     * @param format Format string
     * @param ... Variable arguments
     */
    virtual void printf(const char* format, ...) = 0;
};

#endif // LOGGER_INTERFACE_H