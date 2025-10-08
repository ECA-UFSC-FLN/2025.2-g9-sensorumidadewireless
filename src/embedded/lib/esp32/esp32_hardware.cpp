#include "./esp32_hardware.h"
#include <Arduino.h>
#include <esp_system.h>

void ESP32Hardware::delay(unsigned long ms) {
    ::delay(ms);  // Arduino's delay function
}

void ESP32Hardware::deepSleep(unsigned long microseconds) {
    esp_sleep_enable_timer_wakeup(microseconds);
    esp_deep_sleep_start();
}

unsigned long ESP32Hardware::generateRandomNumber() {
    return esp_random();
}

unsigned long ESP32Hardware::getCurrentTime() {
    return millis();
}

float ESP32Hardware::readAnalog(uint8_t pin) {
    return analogRead(pin) * (3.3 / 4095.0);  // Convert to voltage (3.3V reference)
}