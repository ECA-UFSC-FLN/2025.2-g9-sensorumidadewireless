#include "./esp32_json.h"

bool ESP32JsonSerializer::serialize(const void* data, char* output, size_t maxSize) {
    // Note: This is a basic implementation. In real usage, you'd need to
    // implement specific serialization for each data type you want to handle
    const JsonObject* obj = static_cast<const JsonObject*>(data);
    size_t len = serializeJson(*obj, output, maxSize);
    return len > 0 && len < maxSize;
}

bool ESP32JsonSerializer::deserialize(const char* input, void* output) {
    // Note: This is a basic implementation. In real usage, you'd need to
    // implement specific deserialization for each data type
    DeserializationError error = deserializeJson(doc, input);
    if (error) {
        return false;
    }
    
    JsonObject* obj = static_cast<JsonObject*>(output);
    *obj = doc.as<JsonObject>();
    return true;
}