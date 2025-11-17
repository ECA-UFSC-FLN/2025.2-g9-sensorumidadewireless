#include "esp32_wifi.h"

#if defined(ARDUINO_ARCH_ESP32) || defined(ESP32)
#include <WiFi.h>
#include <Arduino.h>
#endif

ESP32WiFi::ESP32WiFi(ILogger& logger)
    : _logger(logger), _ssid(), _password() {}

bool ESP32WiFi::connect() {
    if (_ssid.empty()) {
        _logger.error("WiFi SSID not set");
        return false;
    }

#if defined(ARDUINO_ARCH_ESP32) || defined(ESP32)
    _logger.info("Conectando ao WiFi...");
    WiFi.mode(WIFI_STA);
    WiFi.begin(_ssid.c_str(), _password.c_str());

    unsigned long start = millis();
    const unsigned long timeout = 15000; // 15 seconds
    while (WiFi.status() != WL_CONNECTED && (millis() - start) < timeout) {
        delay(500);
        _logger.debug("Aguardando conexão WiFi...");
    }

    if (WiFi.status() == WL_CONNECTED) {
        _logger.info("WiFi conectado");
        return true;
    }
    _logger.error("Falha ao conectar no WiFi");
    return false;
#else
    _logger.info("Conectando ao WiFi... (simulado)");
    return true;
#endif
}

void ESP32WiFi::disconnect() {
#if defined(ARDUINO_ARCH_ESP32) || defined(ESP32)
    _logger.info("Desconectando do WiFi...");
    WiFi.disconnect(true);
#else
    _logger.info("Desconectando do WiFi... (simulado)");
#endif
}

bool ESP32WiFi::isConnected() {
#if defined(ARDUINO_ARCH_ESP32) || defined(ESP32)
    bool connected = (WiFi.status() == WL_CONNECTED);
    // _logger.debug(connected ? "WiFi está conectado" : "WiFi não está conectado");
    return connected;
#else
    _logger.debug("Verificando conexão WiFi... (simulado)");
    return true;
#endif
}

const char* ESP32WiFi::getIPAddress() {
#if defined(ARDUINO_ARCH_ESP32) || defined(ESP32)
    _logger.debug("Obtendo endereço IP...");
    static std::string ipStr;
    ipStr = WiFi.localIP().toString().c_str();
    return ipStr.c_str();
#else
    _logger.debug("Obtendo endereço IP... (simulado)");
    return "192.168.1.100";
#endif
}

void ESP32WiFi::setCredentials(const char* ssid, const char* password) {
    _ssid = ssid ? ssid : std::string();
    _password = password ? password : std::string();
    _logger.info("Credenciais de WiFi configuradas.");
}