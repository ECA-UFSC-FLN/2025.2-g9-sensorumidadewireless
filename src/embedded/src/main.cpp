#include "../lib/controllers/MainController.h"
#include "../lib/esp32/esp32_hardware.h"
#include "../lib/esp32/esp32_mqtt.h"
#include "../lib/esp32/esp32_wifi.h"
#include "../lib/esp32/esp32_logger.h"
#include "../lib/esp32/esp32_json.h"

// Configurações
const char* WIFI_SSID = "Gelain";
const char* WIFI_PASSWORD = "Gabriel12345";
const char* MQTT_SERVER = "192.168.137.1";
const int MQTT_PORT = 1883;

// Instâncias
ESP32Logger logger;
ESP32WiFi wifi(logger);
ESP32Hardware hardware;
ESP32MQTTClient mqtt(MQTT_SERVER, MQTT_PORT);
MainController* controller = nullptr;
ESP32JsonSerializer json;

// Variáveis de controle de reconexão
unsigned long lastWifiReconnectAttempt = 0;
unsigned long lastMqttReconnectAttempt = 0;
const unsigned long RECONNECT_INTERVAL = 5000; // 5 segundos


void setup() {
    // Inicializa serial para logging
    Serial.begin(115200);
    while (!Serial) delay(100);
    
    // Configura e conecta WiFi
    wifi.setCredentials(WIFI_SSID, WIFI_PASSWORD);
    if (!wifi.connect()) {
        logger.error("Falha fatal - Não foi possível conectar ao WiFi");
        return;
    }

    // Configura MQTT
    // mqtt.configure();
    // if (!mqtt.connect("ESP32_Client")) {
    //     logger.error("Falha fatal - Não foi possível conectar ao MQTT");
    //     return;cd 
    // }
    
    // Inicializa controlador principal
    controller = new MainController(hardware, mqtt, json, logger);
    logger.info("Sistema inicializado!");
}

void loop() {
    unsigned long now = millis();
    
    // Verifica e reconecta WiFi se necessário
    if (!wifi.isConnected()) {
        if (now - lastWifiReconnectAttempt > RECONNECT_INTERVAL) {
            logger.info("Tentando reconectar ao WiFi...");
            if (wifi.connect()) {
                logger.info("WiFi reconectado com sucesso!");
                // Reseta a tentativa de reconexão MQTT quando WiFi reconecta
                lastMqttReconnectAttempt = 0;
            } else {
                logger.error("Falha ao reconectar ao WiFi");
            }
            lastWifiReconnectAttempt = now;
        }
        delay(100);
        return; // Aguarda WiFi antes de prosseguir
    }
    
    // Verifica e reconecta MQTT se necessário
    if (!mqtt.isConnected()) {
        if (now - lastMqttReconnectAttempt > RECONNECT_INTERVAL) {
            logger.info("Tentando reconectar ao MQTT...");
            if (mqtt.connect("ESP32_Client")) {
                logger.info("MQTT reconectado com sucesso!");
                // Re-subscribe aos tópicos após reconexão
                if (controller) {
                    controller->ensureSubscriptions();
                }
            } else {
                logger.error("Falha ao reconectar ao MQTT");
            }
            lastMqttReconnectAttempt = now;
        }
    } else {
        // Só executa o loop MQTT se estiver conectado
        mqtt.loop();
    }
    
    // Executa loop do controlador se as conexões estiverem OK
    if (controller && wifi.isConnected() && mqtt.isConnected()) {
        controller->loop();
    }
    
    // Pequeno delay para evitar sobrecarga
    delay(10);
}