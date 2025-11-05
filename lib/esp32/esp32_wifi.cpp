#include "esp32_wifi.h"
#include <cstring>

ESP32WiFi::ESP32WiFi(ILogger& logger)
    : _logger(logger), _ssid(nullptr), _password(nullptr) {}

bool ESP32WiFi::connect() {
    // Aqui você deve implementar a lógica de conexão WiFi real para ESP32
    _logger.info("Conectando ao WiFi...");
    // Simulação de sucesso
    return true;
}

void ESP32WiFi::disconnect() {
    _logger.info("Desconectando do WiFi...");
    // Simulação de desconexão
}

bool ESP32WiFi::isConnected() {
    // Simulação de status de conexão
    _logger.debug("Verificando conexão WiFi...");
    return true;
}

const char* ESP32WiFi::getIPAddress() {
    // Simulação de IP
    _logger.debug("Obtendo endereço IP...");
    return "192.168.1.100";
}

void ESP32WiFi::setCredentials(const char* ssid, const char* password) {
    _ssid = ssid;
    _password = password;
    _logger.info("Credenciais de WiFi configuradas.");
}