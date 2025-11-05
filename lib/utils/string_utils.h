#ifndef STRING_UTILS_H
#define STRING_UTILS_H

#include <stddef.h>

// Simple string class that doesn't depend on Arduino
class StringView {
public:
    StringView(const char* str) : data_(str) {}
    
    void toCharArray(char* buffer, size_t size) const {
        size_t i = 0;
        while (i < size - 1 && data_[i] != '\0') {
            buffer[i] = data_[i];
            i++;
        }
        buffer[i] = '\0';
    }

    const char* c_str() const { return data_; }

private:
    const char* data_;
};

#endif // STRING_UTILS_H