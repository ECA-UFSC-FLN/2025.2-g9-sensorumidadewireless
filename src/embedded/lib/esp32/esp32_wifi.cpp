#include "esp32_wifi.h"
#include <WiFi.h>

ESP32WiFi::ESP32WiFi(ILogger& logger) 
    : _logger(logger)
    , _ssid(nullptr)
    , _password(nullptr) {
}

bool ESP32WiFi::connect() {
    if (!_ssid || !_password) {
        _logger.error("Credenciais WiFi não configuradas");
        return false;
    }

    _logger.info("Conectando ao WiFi...");
    WiFi.begin(_ssid, _password);
    
    unsigned long startAttempt = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - startAttempt < 10000) {
        delay(500);
    }

    if (WiFi.status() == WL_CONNECTED) {
        _logger.info("WiFi conectado!");
        _logger.info("IP: " + String(WiFi.localIP()));
        return true;
    }

    _logger.error("Falha ao conectar ao WiFi");
    return false;
}

void ESP32WiFi::disconnect() {
    WiFi.disconnect();
    _logger.info("WiFi desconectado");
}

bool ESP32WiFi::isConnected() {
    return WiFi.status() == WL_CONNECTED;
}

const char* ESP32WiFi::getIPAddress() {
    static char ipStr[16];
    IPAddress ip = WiFi.localIP();
    snprintf(ipStr, sizeof(ipStr), "%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);
    return ipStr;
}

void ESP32WiFi::setCredentials(const char* ssid, const char* password) {
    _ssid = ssid;
    _password = password;
}