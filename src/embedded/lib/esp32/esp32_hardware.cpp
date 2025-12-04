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



// É necessária a realização de uma calibração
const int DRY_VALUE = 3800;   // leitura quando está seco
const int WET_VALUE = 1200;   // leitura quando está molhado

float ESP32Hardware::readAnalog(uint8_t pin) {

    int raw = analogRead(pin);

    // Converte para porcentagem (0–100)
    float percent = (float)(raw - DRY_VALUE) * 100.0 / (WET_VALUE - DRY_VALUE);

    // Limita entre 0 e 100
    if (percent < 0) percent = 0;
    if (percent > 100) percent = 100;

    return percent;
}