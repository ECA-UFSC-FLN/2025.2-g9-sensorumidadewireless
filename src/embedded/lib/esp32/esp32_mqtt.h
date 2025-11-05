#ifndef ESP32_MQTT_H
#define ESP32_MQTT_H

#include "../comm/mqtt_interface.h"
#include <stdint.h>

// Forward declarations
class WiFiClient;
class PubSubClient;

class ESP32MQTTClient : public IMQTTClient {
public:
    ESP32MQTTClient(const char* server, int port);
    ~ESP32MQTTClient();
    
    bool connect(const char* clientId) override;
    bool publish(const char* topic, const char* payload) override;
    bool subscribe(const char* topic) override;
    void setCallback(MQTTCallback callback) override;
    bool isConnected() override;
    void loop() override;

private:
    const char* server;
    int port;
    WiFiClient* wifiClient;      // Ponteiros para permitir forward declaration
    PubSubClient* mqttClient;    // Ponteiros para permitir forward declaration
    static void internalCallback(char* topic, uint8_t* payload, unsigned int length);
    static MQTTCallback userCallback;
};

#endif // ESP32_MQTT_H