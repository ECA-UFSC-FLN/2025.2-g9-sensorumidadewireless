#ifndef MQTT_TYPES_H
#define MQTT_TYPES_H

#include <stdint.h>

struct MQTTMessage {
    const char* topic;
    const uint8_t* payload;
    unsigned int length;
};

// Callback function type definition
using MQTTCallback = void (*)(const char* topic, const uint8_t* payload, unsigned int length);

#endif // MQTT_TYPES_H