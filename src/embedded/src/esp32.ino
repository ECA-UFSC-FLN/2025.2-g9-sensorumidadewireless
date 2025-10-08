#include "../lib/controllers/MainController.h"
#include "../lib/esp32/esp32_hardware.h"
#include "../lib/esp32/esp32_mqtt.h"
#include "../lib/esp32/esp32_wifi.h"
#include "../lib/esp32/esp32_logger.h"

// Configurações
const char* WIFI_SSID = "wifi";
const char* WIFI_PASSWORD = "senha";
const char* MQTT_SERVER = "127.0.0.1";
const int MQTT_PORT = 1883;

// Instâncias
ESP32Logger logger;
ESP32WiFi wifi(logger);
ESP32Hardware hardware(logger);
ESP32MQTT mqtt(logger);
MainController* controller = nullptr;

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
    mqtt.configure(MQTT_SERVER, MQTT_PORT);
    if (!mqtt.connect("ESP32_Client")) {
        logger.error("Falha fatal - Não foi possível conectar ao MQTT");
        return;
    }
    
    // Inicializa controlador principal
    controller = new MainController(hardware, mqtt, logger);
    logger.info("Sistema inicializado!");
}

void loop() {
    // Verifica conexões
    if (!wifi.isConnected()) {
        logger.error("Conexão WiFi perdida. Reconectando...");
        wifi.connect();
    }
    
    if (!mqtt.isConnected()) {
        logger.error("Conexão MQTT perdida. Reconectando...");
        mqtt.connect("ESP32_Client");
    }
    
    // Executa loop do controlador
    if (controller) {
        controller->loop();
    }
    
    // Delay para evitar sobrecarga do CPU
    delay(10);
}