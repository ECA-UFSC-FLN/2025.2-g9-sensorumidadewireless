#include "./esp32_mqtt.h"

MQTTCallback ESP32MQTTClient::userCallback = nullptr;

ESP32MQTTClient::ESP32MQTTClient(const char* server, int port) : mqttClient(wifiClient) {
    mqttClient.setServer(server, port);
}

bool ESP32MQTTClient::connect(const char* clientId) {
    return mqttClient.connect(clientId);
}

bool ESP32MQTTClient::publish(const char* topic, const char* payload) {
    return mqttClient.publish(topic, payload);
}

bool ESP32MQTTClient::subscribe(const char* topic) {
    return mqttClient.subscribe(topic);
}

void ESP32MQTTClient::setCallback(MQTTCallback callback) {
    userCallback = callback;
    mqttClient.setCallback(internalCallback);
}

bool ESP32MQTTClient::isConnected() {
    return mqttClient.connected();
}

void ESP32MQTTClient::loop() {
    mqttClient.loop();
}

void ESP32MQTTClient::internalCallback(char* topic, byte* payload, unsigned int length) {
    if (userCallback) {
        userCallback(topic, payload, length);
    }
}