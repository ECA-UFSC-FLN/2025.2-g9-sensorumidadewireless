#ifndef JSON_INTERFACE_H
#define JSON_INTERFACE_H

#include <stddef.h>

class IJsonSerializer {
public:
    virtual ~IJsonSerializer() = default;
    
    /**
     * @brief Serialize data to JSON string
     * @param data Pointer to data structure
     * @param output Buffer to store JSON string
     * @param maxSize Maximum size of output buffer
     * @return true if serialization successful, false otherwise
     */
    virtual bool serialize(const void* data, char* output, size_t maxSize) = 0;
    
    /**
     * @brief Deserialize JSON string to data structure
     * @param input JSON string to deserialize
     * @param output Pointer to data structure to fill
     * @return true if deserialization successful, false otherwise
     */
    virtual bool deserialize(const char* input, void* output) = 0;
};

#endif // JSON_INTERFACE_H