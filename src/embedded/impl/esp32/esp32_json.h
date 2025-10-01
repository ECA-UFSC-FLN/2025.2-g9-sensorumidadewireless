#ifndef ESP32_JSON_H
#define ESP32_JSON_H

#include "../../include/utils/json_interface.h"
#include <ArduinoJson.h>

class ESP32JsonSerializer : public IJsonSerializer {
public:
    bool serialize(const void* data, char* output, size_t maxSize) override;
    bool deserialize(const char* input, void* output) override;

private:
    StaticJsonDocument<256> doc;  // Adjust size as needed
};

#endif // ESP32_JSON_H