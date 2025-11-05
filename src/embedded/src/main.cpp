#include "../lib/controllers/MainController.h"
#include "../lib/esp32/esp32_hardware.h"
#include "../lib/esp32/esp32_mqtt.h"
#include "../lib/esp32/esp32_wifi.h"
#include "../lib/esp32/esp32_logger.h"
#include "../lib/esp32/esp32_json.h"

// Configurações
const char* WIFI_SSID = "Gelain";
const char* WIFI_PASSWORD = "gabriel12345";
const char* MQTT_SERVER = "172.20.10.4";
const int MQTT_PORT = 1883;

// Instâncias
ESP32Logger logger;
ESP32WiFi wifi(logger);
ESP32Hardware hardware;
ESP32MQTTClient mqtt(MQTT_SERVER, MQTT_PORT);
MainController* controller = nullptr;
ESP32JsonSerializer json;

// NOTA SOBRE DEEP SLEEP:
// Após acordar do deep sleep, o ESP32 reinicia completamente.
// Para manter estado entre ciclos de sleep, você precisará:
// 1. Usar RTC memory (RTC_DATA_ATTR)
// 2. Verificar esp_reset_reason() no setup()
// 3. Restaurar o estado da máquina conforme necessário


void setup() {
    // Inicializa serial para logging (não precisa aqui, logger já faz isso)
    // Serial.begin(115200);  // REMOVIDO - Logger já inicializa
    // while (!Serial) delay(100);
    
    delay(1000); // Aguarda estabilização do sistema
    
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
    // Verifica conexão WiFi primeiro
    if (!wifi.isConnected()) {
        logger.error("Conexão WiFi perdida. Reconectando...");
        wifi.connect();
        
        // Aguarda WiFi reconectar antes de tentar MQTT
        if (wifi.isConnected()) {
            delay(2000); // Aguarda estabilização
            logger.info("WiFi reconectado. Tentando MQTT...");
        } else {
            delay(5000); // Aguarda antes de tentar novamente
            return; // Não tenta MQTT sem WiFi
        }
    }
    
    // Só tenta MQTT se WiFi estiver conectado
    if (wifi.isConnected() && !mqtt.isConnected()) {
        logger.error("Conexão MQTT perdida. Reconectando...");
        
        // Aguarda um pouco antes de reconectar MQTT
        static unsigned long lastMqttAttempt = 0;
        unsigned long now = millis();
        
        if (now - lastMqttAttempt > 5000) { // Tenta a cada 5 segundos
            if (mqtt.connect("ESP32_Client")) {
                logger.info("MQTT reconectado com sucesso!");
            }
            lastMqttAttempt = now;
        }
    }
    
    // Executa loop do controlador apenas se ambos estiverem conectados
    if (controller && wifi.isConnected() && mqtt.isConnected()) {
        controller->loop();
    }
    
    // Delay para evitar sobrecarga do CPU
    delay(10);
}