#include "esp32_mqtt.h"
#include <WiFiClient.h>
#include <PubSubClient.h>

MQTTCallback ESP32MQTTClient::userCallback = nullptr;

ESP32MQTTClient::ESP32MQTTClient(const char* server, int port) 
    : server(server), port(port) {
    // Primeiro cria o WiFiClient
    wifiClient = new WiFiClient();
    // Depois cria o PubSubClient passando uma REFERÊNCIA ao WiFiClient
    mqttClient = new PubSubClient(*wifiClient);
    // Configura o servidor MQTT
    mqttClient->setServer(server, port);
}

ESP32MQTTClient::~ESP32MQTTClient() {
    delete mqttClient;
    delete wifiClient;
}

bool ESP32MQTTClient::connect(const char* clientId) {
    return mqttClient->connect(clientId);
}

bool ESP32MQTTClient::publish(const char* topic, const char* payload) {
    return mqttClient->publish(topic, payload);
}

bool ESP32MQTTClient::subscribe(const char* topic) {
    return mqttClient->subscribe(topic);
}

void ESP32MQTTClient::setCallback(MQTTCallback callback) {
    userCallback = callback;
    mqttClient->setCallback(internalCallback);
}

bool ESP32MQTTClient::isConnected() {
    return mqttClient->connected();
}

void ESP32MQTTClient::loop() {
    mqttClient->loop();
}

void ESP32MQTTClient::internalCallback(char* topic, uint8_t* payload, unsigned int length) {
    if (userCallback) {
        userCallback(topic, payload, length);
    }
}