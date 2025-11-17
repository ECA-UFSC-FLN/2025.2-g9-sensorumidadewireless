#ifndef MAIN_CONTROLLER_H
#define MAIN_CONTROLLER_H

#include <stddef.h>
#include <stdint.h>
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
            // Simple JSON parsing for bind response
            const char* req_id_start = strstr(input, "\"req_id\":\"");
            const char* id_start = strstr(input, "\"id\":\"");
            const char* status_start = strstr(input, "\"status\":\"");
            
            if (!req_id_start || !id_start || !status_start) {
                return false;
            }
            
            // Extract req_id
            req_id_start += 10; // Skip "req_id":"
            const char* req_id_end = strchr(req_id_start, '"');
            if (!req_id_end || (req_id_end - req_id_start) >= (int)sizeof(req_id)) {
                return false;
            }
            strncpy(req_id, req_id_start, req_id_end - req_id_start);
            req_id[req_id_end - req_id_start] = '\0';
            
            // Extract id
            id_start += 5; // Skip "id":"
            const char* id_end = strchr(id_start, '"');
            if (!id_end || (id_end - id_start) >= (int)sizeof(id)) {
                return false;
            }
            strncpy(id, id_start, id_end - id_start);
            id[id_end - id_start] = '\0';
            
            // Extract status
            status_start += 9; // Skip "status":"
            const char* status_end = strchr(status_start, '"');
            if (!status_end || (status_end - status_start) >= (int)sizeof(status)) {
                return false;
            }
            strncpy(status, status_start, status_end - status_start);
            status[status_end - status_start] = '\0';
            
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