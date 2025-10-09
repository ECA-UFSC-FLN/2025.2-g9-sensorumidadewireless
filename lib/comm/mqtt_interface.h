#ifndef MQTT_INTERFACE_H
#define MQTT_INTERFACE_H

#include "mqtt_types.h"

class IMQTTClient {
public:
    virtual ~IMQTTClient() = default;
    
    /**
     * @brief Connect to MQTT broker
     * @param clientId Unique client identifier
     * @return true if connection successful, false otherwise
     */
    virtual bool connect(const char* clientId) = 0;
    
    /**
     * @brief Publish message to a topic
     * @param topic Topic to publish to
     * @param payload Message payload
     * @return true if publish successful, false otherwise
     */
    virtual bool publish(const char* topic, const char* payload) = 0;
    
    /**
     * @brief Subscribe to a topic
     * @param topic Topic to subscribe to
     * @return true if subscription successful, false otherwise
     */
    virtual bool subscribe(const char* topic) = 0;
    
    /**
     * @brief Set callback for incoming messages
     * @param callback Function to call when message arrives
     */
    virtual void setCallback(MQTTCallback callback) = 0;
    
    /**
     * @brief Check if connected to broker
     * @return true if connected, false otherwise
     */
    virtual bool isConnected() = 0;
    
    /**
     * @brief Process incoming messages and maintain connection
     */
    virtual void loop() = 0;
};

#endif // MQTT_INTERFACE_H