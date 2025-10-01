#ifndef ESP32_MQTT_H
#define ESP32_MQTT_H

#include "../../include/comm/mqtt_interface.h"
#include <PubSubClient.h>
#include <WiFiClient.h>

class ESP32MQTTClient : public IMQTTClient {
public:
    ESP32MQTTClient(const char* server, int port);
    
    bool connect(const char* clientId) override;
    bool publish(const char* topic, const char* payload) override;
    bool subscribe(const char* topic) override;
    void setCallback(MQTTCallback callback) override;
    bool isConnected() override;
    void loop() override;

private:
    WiFiClient wifiClient;
    PubSubClient mqttClient;
    static void internalCallback(char* topic, byte* payload, unsigned int length);
    static MQTTCallback userCallback;
};

#endif // ESP32_MQTT_H