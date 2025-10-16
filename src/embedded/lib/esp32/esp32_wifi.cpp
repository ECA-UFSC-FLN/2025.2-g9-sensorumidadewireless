#include "esp32_wifi.h"
#include <WiFi.h>

ESP32WiFi::ESP32WiFi(ILogger& logger)
    : _logger(logger), _ssid(nullptr), _password(nullptr) {}

bool ESP32WiFi::connect() {
    if (_ssid == nullptr || _password == nullptr) {
        _logger.error("Credenciais de WiFi não configuradas!");
        return false;
    }

    _logger.info("Conectando ao WiFi...");
    
    // Desconecta se já estiver conectado
    if (WiFi.status() == WL_CONNECTED) {
        WiFi.disconnect();
        delay(100);
    }
    
    // Inicia conexão WiFi
    WiFi.mode(WIFI_STA);
    WiFi.begin(_ssid, _password);
    
    // Aguarda conexão (timeout de 20 segundos)
    int attempts = 0;
    const int maxAttempts = 40; // 40 * 500ms = 20 segundos
    
    while (WiFi.status() != WL_CONNECTED && attempts < maxAttempts) {
        delay(500);
        _logger.debug(".");
        attempts++;
    }
    
    if (WiFi.status() == WL_CONNECTED) {
        _logger.info("WiFi conectado!");
        _logger.printf("Endereço IP: %s\n", WiFi.localIP().toString().c_str());
        return true;
    } else {
        _logger.error("Falha ao conectar ao WiFi!");
        return false;
    }
}

void ESP32WiFi::disconnect() {
    _logger.info("Desconectando do WiFi...");
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
}

bool ESP32WiFi::isConnected() {
    return WiFi.status() == WL_CONNECTED;
}

const char* ESP32WiFi::getIPAddress() {
    static String ipString;
    ipString = WiFi.localIP().toString();
    return ipString.c_str();
}

void ESP32WiFi::setCredentials(const char* ssid, const char* password) {
    _ssid = ssid;
    _password = password;
    _logger.info("Credenciais de WiFi configuradas.");
}