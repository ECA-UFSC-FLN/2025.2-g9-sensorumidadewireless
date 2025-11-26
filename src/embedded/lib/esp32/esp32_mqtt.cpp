#include "./esp32_mqtt.h"
#include <Arduino.h>

MQTTCallback ESP32MQTTClient::userCallback = nullptr;

ESP32MQTTClient::ESP32MQTTClient(const char* server, int port) : mqttClient(wifiClient) {
    mqttClient.setServer(server, port);
}

bool ESP32MQTTClient::connect(const char* clientId) {
    bool ok = mqttClient.connect(clientId);
    if (!ok) {
        int st = mqttClient.state();
        const char* reason = "UNKNOWN";
        switch (st) {
            case -4: reason = "BUFFER_OVERFLOW"; break;
            case -3: reason = "LOOP_TIMEOUT"; break;
            case -2: reason = "CONNECT_TIMEOUT"; break;
            case -1: reason = "NETWORK_DISCONNECTED"; break;
            case 0:  reason = "SUCCESS"; break;
            case 1:  reason = "CONNECTION_REFUSED"; break;
            case 2:  reason = "CONNECTION_REFUSED_PROTOCOL"; break;
            case 3:  reason = "CONNECTION_REFUSED_ID_REJECTED"; break;
            case 4:  reason = "CONNECTION_REFUSED_SERVER_UNAVAILABLE"; break;
            case 5:  reason = "CONNECTION_REFUSED_BAD_USERNAME_PASSWORD"; break;
            case 6:  reason = "CONNECTION_REFUSED_NOT_AUTHORIZED"; break;
        }
        Serial.printf("[MQTT] connect() failed state=%d (%s)\n", st, reason);
    }
    return ok;
}

bool ESP32MQTTClient::publish(const char* topic, const char* payload) {
    return mqttClient.publish(topic, payload);
}

bool ESP32MQTTClient::publish(const char* topic, const char* payload, bool retain) {
    return mqttClient.publish(topic, payload, retain);
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