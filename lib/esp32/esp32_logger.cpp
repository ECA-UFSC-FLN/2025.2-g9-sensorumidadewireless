#include "./esp32_logger.h"
#include <Arduino.h>

ESP32Logger::ESP32Logger(unsigned long baudRate) {
    Serial.begin(baudRate);
}

void ESP32Logger::debug(const char* message) {
    Serial.print("[DEBUG] ");
    Serial.println(message);
}

void ESP32Logger::info(const char* message) {
    Serial.print("[INFO] ");
    Serial.println(message);
}

void ESP32Logger::error(const char* message) {
    Serial.print("[ERROR] ");
    Serial.println(message);
}

void ESP32Logger::printf(const char* format, ...) {
    char buffer[256];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    Serial.print(buffer);
}