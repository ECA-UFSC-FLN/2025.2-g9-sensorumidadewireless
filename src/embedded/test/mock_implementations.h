#ifndef MOCK_IMPLEMENTATIONS_H
#define MOCK_IMPLEMENTATIONS_H

#include "../lib/hal/hardware_interface.h"
#include "../lib/comm/mqtt_interface.h"
#include "../lib/utils/json_interface.h"
#include "../lib/utils/logger_interface.h"
#include <unistd.h>
#include <time.h>
#include <random>
#include <iostream>
#include <nlohmann/json.hpp>

// Mock Hardware Implementation
class MockHardware : public IHardware {
public:
    void delay(unsigned long ms) override {
        usleep(ms * 1000);  // Convert to microseconds
    }

    void deepSleep(unsigned long microseconds) override {
        // No teste, reduzimos o deep sleep para 100ms para acelerar a execução
        std::cout << "[MockHardware] Deep sleep for " << microseconds << " microseconds\n";
        usleep(100000); // 100ms
    }

    unsigned long generateRandomNumber() override {
        static std::random_device rd;
        static std::mt19937 gen(rd());
        static std::uniform_int_distribution<unsigned long> dis;
        return dis(gen);
    }

    unsigned long getCurrentTime() override {
        return static_cast<unsigned long>(time(nullptr) * 1000);  // Convert to milliseconds
    }

    float readAnalog(uint8_t pin) override {
        return 1.5f;  // Mock voltage reading
    }
};

// Mock MQTT Implementation
class MockMQTT : public IMQTTClient {
public:
    using PublishInterceptor = std::function<void(const char*, const char*)>;

    void setPublishInterceptor(PublishInterceptor interceptor) {
        publishInterceptor = interceptor;
    }

    // Simulate receiving a message
    void simulateMessage(const char* topic, const char* payload) {
        if (callback) {
            callback(topic, reinterpret_cast<const uint8_t*>(payload), strlen(payload));
        }
    }

    bool connect(const char* clientId) override {
        std::cout << "[MockMQTT] Connected with ID: " << clientId << "\n";
        connected = true;
        return true;
    }

    bool publish(const char* topic, const char* payload) override {
        std::cout << "[MockMQTT] Published to " << topic << ": " << payload << "\n";
        
        if (publishInterceptor) {
            publishInterceptor(topic, payload);
        }
        
        // Simulate bind response if this is a bind request
        if (strcmp(topic, "sensores/bind/request") == 0) {
            try {
                // Parse the request
                auto req = nlohmann::json::parse(payload);
                if (req.contains("req_id") && req.contains("nome")) {
                    std::string reqId = req["req_id"];
                    
                    // Create response
                    nlohmann::json resp;
                    resp["req_id"] = reqId;
                    resp["id"] = "TEST_ID_001";
                    resp["status"] = "ok";
                    
                    std::string respStr = resp.dump();
                    std::cout << "[MockMQTT] Sending bind response: " << respStr << "\n";
                    
                    if (callback) {
                        callback("sensores/bind/response", 
                                reinterpret_cast<const uint8_t*>(respStr.c_str()), 
                                respStr.length());
                    }
                }
            } catch (const nlohmann::json::exception& e) {
                std::cerr << "[MockMQTT] JSON parse error: " << e.what() << "\n";
            }
        }
        return true;
    }

    bool publish(const char* topic, const char* payload, bool retain) override {
        std::cout << "[MockMQTT] Published to " << topic << " (retain=" << (retain?"true":"false") << "): " << payload << "\n";
        if (publishInterceptor) {
            publishInterceptor(topic, payload);
        }
        // Reuse same behavior as non-retain publish for tests
        if (strcmp(topic, "sensores/bind/request") == 0) {
            try {
                auto req = nlohmann::json::parse(payload);
                if (req.contains("req_id") && req.contains("nome")) {
                    std::string reqId = req["req_id"];
                    nlohmann::json resp;
                    resp["req_id"] = reqId;
                    resp["id"] = "TEST_ID_001";
                    resp["status"] = "ok";
                    std::string respStr = resp.dump();
                    std::cout << "[MockMQTT] Sending bind response: " << respStr << "\n";
                    if (callback) {
                        callback("sensores/bind/response", reinterpret_cast<const uint8_t*>(respStr.c_str()), respStr.length());
                    }
                }
            } catch (const nlohmann::json::exception& e) {
                std::cerr << "[MockMQTT] JSON parse error: " << e.what() << "\n";
            }
        }
        return true;
    }

    bool subscribe(const char* topic) override {
        std::cout << "[MockMQTT] Subscribed to: " << topic << "\n";
        return true;
    }

    void setCallback(MQTTCallback cb) override {
        callback = cb;
    }

    bool isConnected() override {
        return connected;
    }

    void loop() override {
        // Simulate incoming messages here if needed
    }

private:
    bool connected = false;
    MQTTCallback callback = nullptr;
    PublishInterceptor publishInterceptor = nullptr;
};

// Mock JSON Implementation
class MockJSON : public IJsonSerializer {
public:
    bool serialize(const void* data, char* output, size_t maxSize) override {
        try {
            nlohmann::json j;
            const auto* msg = static_cast<const MainController::MensagemBindRequest*>(data);
            j["req_id"] = msg->req_id;
            j["nome"] = msg->nome;
            
            std::string jsonStr = j.dump();
            if (jsonStr.length() >= maxSize) {
                return false;
            }
            strcpy(output, jsonStr.c_str());
            return true;
        } catch (const std::exception& e) {
            std::cerr << "[MockJSON] Serialization error: " << e.what() << "\n";
            return false;
        }
    }

    bool deserialize(const char* input, void* output) override {
        try {
            auto j = nlohmann::json::parse(input);
            auto* resp = static_cast<MainController::MensagemBindResponse*>(output);
            
            if (j.contains("req_id")) {
                strncpy(resp->req_id, j["req_id"].get<std::string>().c_str(), sizeof(resp->req_id) - 1);
                resp->req_id[sizeof(resp->req_id) - 1] = '\0';
            }
            
            if (j.contains("id")) {
                strncpy(resp->id, j["id"].get<std::string>().c_str(), sizeof(resp->id) - 1);
                resp->id[sizeof(resp->id) - 1] = '\0';
            }
            
            if (j.contains("status")) {
                strncpy(resp->status, j["status"].get<std::string>().c_str(), sizeof(resp->status) - 1);
                resp->status[sizeof(resp->status) - 1] = '\0';
            }
            
            return true;
        } catch (const nlohmann::json::exception& e) {
            std::cerr << "[MockJSON] Deserialization error: " << e.what() << "\n";
            return false;
        }
    }
};

// Mock Logger Implementation
class MockLogger : public ILogger {
public:
    void debug(const char* message) override {
        std::cout << "[DEBUG] " << message << std::endl;
    }

    void info(const char* message) override {
        std::cout << "[INFO] " << message << std::endl;
    }

    void error(const char* message) override {
        std::cerr << "[ERROR] " << message << std::endl;
    }

    void printf(const char* format, ...) override {
        va_list args;
        va_start(args, format);
        vprintf(format, args);
        va_end(args);
    }
};

#endif // MOCK_IMPLEMENTATIONS_H