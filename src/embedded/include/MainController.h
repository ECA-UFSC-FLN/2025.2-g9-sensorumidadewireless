#ifndef MAIN_CONTROLLER_H
#define MAIN_CONTROLLER_H

#include <stddef.h>
#include <stdint.h>
#include "../include/utils/string_utils.h"
#include "../include/hal/hardware_interface.h"
#include "../include/comm/mqtt_interface.h"
#include "../include/utils/json_interface.h"
#include "../include/utils/logger_interface.h"

class MainController {
public:
    // Message structs
    struct MensagemMedicao {
        float medicao;
        float soc;
        char id[32];
    };

    struct MensagemBindRequest {
        char req_id[37];
        char nome[32];
    };

    struct MensagemBindResponse {
        char req_id[37];
        char id[32];
        char status[8];
    };

    struct MensagemUnbind {
        char id[32];
    };

    // Estados da máquina de estados
    enum Estado {
        CONEXAO_MQTT,
        BIND,
        AGUARDE,
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