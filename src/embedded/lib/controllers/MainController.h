#ifndef MAIN_CONTROLLER_H
#define MAIN_CONTROLLER_H

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "../utils/string_utils.h"
#include "../hal/hardware_interface.h"
#include "../comm/mqtt_interface.h"
#include "../utils/json_interface.h"
#include "../utils/logger_interface.h"

class MainController {
public:
    // Message structs
    struct MensagemMedicao {
        float medicao;
        float soc;
        char id[32];
        
        bool serialize(char* output, size_t maxSize) const {
            int len = snprintf(output, maxSize,
                             "{\"medicao\":%.2f,\"soc\":%.2f,\"id\":\"%s\"}",
                             medicao, soc, id);
            return len > 0 && len < (int)maxSize;
        }
    };

    struct MensagemBindRequest {
        char req_id[37];
        char nome[32];
        
        bool serialize(char* output, size_t maxSize) const {
            int len = snprintf(output, maxSize,
                             "{\"req_id\":\"%s\",\"nome\":\"%s\"}",
                             req_id, nome);
            return len > 0 && len < (int)maxSize;
        }
    };

    struct MensagemBindResponse {
        char req_id[37];
        char id[32];
        char status[8];
        
        bool deserialize(const char* input) {
            // Robust extraction helper: finds a string value for a given JSON key
            auto extract_string = [&](const char* src, const char* key, char* dest, size_t destSize) -> bool {
                const char* p = strstr(src, key);
                if (!p) return false;
                p += strlen(key); // move past the key
                // find ':' after key
                p = strchr(p, ':');
                if (!p) return false;
                p++; // move past ':'
                // skip whitespace
                while (*p && (*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n')) p++;
                // value should be either quoted string or unquoted token
                if (*p == '\"') {
                    p++; // skip opening quote
                    const char* end = strchr(p, '"');
                    if (!end) return false;
                    size_t len = end - p;
                    if (len >= destSize) return false;
                    strncpy(dest, p, len);
                    dest[len] = '\0';
                    return true;
                } else {
                    // unquoted value (read until comma or closing brace)
                    const char* end = p;
                    while (*end && *end != ',' && *end != '}' && !isspace((unsigned char)*end)) end++;
                    size_t len = end - p;
                    if (len == 0 || len >= destSize) return false;
                    strncpy(dest, p, len);
                    dest[len] = '\0';
                    return true;
                }
            };

            // Try to extract the required fields in a whitespace-tolerant way
            if (!extract_string(input, "\"req_id\"", req_id, sizeof(req_id))) return false;
            if (!extract_string(input, "\"id\"", id, sizeof(id))) return false;
            if (!extract_string(input, "\"status\"", status, sizeof(status))) return false;
            return true;
        }
    };

    struct MensagemUnbind {
        char id[32];
        
        bool serialize(char* output, size_t maxSize) const {
            int len = snprintf(output, maxSize,
                             "{\"id\":\"%s\"}",
                             id);
            return len > 0 && len < (int)maxSize;
        }
    };

    // Estados da máquina de estados
    enum Estado {
        CONEXAO_MQTT,
        AGUARDE,
        BIND,
        MEDICAO,
        DEEP_SLEEP,
        CLEANUP,
        SHUTDOWN
    };

    // Constructor with dependency injection
    MainController(
        IHardware& hardware,
        IMQTTClient& mqtt,
        IJsonSerializer& json,
        ILogger& logger
    );

    // Main loop method
    void loop();

    // Get current state
    Estado getState() const { return estadoAtual; }

private:
    // Interfaces injected
    IHardware& hardware;
    IMQTTClient& mqtt;
    IJsonSerializer& json;
    ILogger& logger;

    // State machine and control variables
    Estado estadoAtual;
    bool processoAtivo;
    bool processoFinalizado;
    bool bindOk;
    char reqId[37];
    char idFinal[32];

    // MQTT Topics
    static constexpr const char* TOPICO_MEDICAO = "sensores/medicao";
    static constexpr const char* TOPICO_PROCESSO = "sensores/processo";
    static constexpr const char* TOPICO_BIND_REQUEST = "sensores/bind/request";
    static constexpr const char* TOPICO_BIND_RESPONSE = "sensores/bind/response";
    static constexpr const char* TOPICO_UNBIND = "sensores/bind/unbind";
    static constexpr const char* TOPICO_STATUS = "sensores/status";

    // Private methods for state machine actions
    void handleMQTTCallback(const char* topic, const uint8_t* payload, unsigned int length);
    void handleConexaoMQTT();
    void handleBind();
    void handleAguarde();
    void handleMedicao();
    void handleDeepSleep();
    void handleCleanup();
    void handleShutdown();
    void realizaMedicao();
    StringView generateUUID();

    // Static callback wrapper
    static void staticMQTTCallback(const char* topic, const uint8_t* payload, unsigned int length);
    static MainController* instance; // For callback handling
};

#endif // MAIN_CONTROLLER_H