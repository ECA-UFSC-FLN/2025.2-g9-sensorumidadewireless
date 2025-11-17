#include "MainController.h"
#include <string.h>
#include <stdio.h>

MainController* MainController::instance = nullptr;

MainController::MainController(
    IHardware& hardware,
    IMQTTClient& mqtt,
    IJsonSerializer& json,
    ILogger& logger
) : hardware(hardware),
    mqtt(mqtt),
    json(json),
    logger(logger),
    estadoAtual(CONEXAO_MQTT),
    processoAtivo(false),
    processoFinalizado(false),
    bindOk(false)
{
    instance = this;
    mqtt.setCallback(staticMQTTCallback);
}

void MainController::staticMQTTCallback(const char* topic, const uint8_t* payload, unsigned int length) {
    if (instance) {
        instance->handleMQTTCallback(topic, payload, length);
    }
}

void MainController::handleMQTTCallback(const char* topic, const uint8_t* payload, unsigned int length) {
    char message[256];
    memcpy(message, payload, length);
    message[length] = '\0';

    if (strcmp(topic, TOPICO_PROCESSO) == 0) {
        if (strcmp(message, "iniciar") == 0) processoAtivo = true;
        if (strcmp(message, "finalizar") == 0) processoFinalizado = true;
    }
    else if (strcmp(topic, TOPICO_BIND_RESPONSE) == 0) {
        MensagemBindResponse response;
        if (response.deserialize(message)) {
            if (strcmp(response.req_id, reqId) == 0 && strcmp(response.status, "ok") == 0) {
                strcpy(idFinal, response.id);
                bindOk = true;
                logger.printf("[BIND] ID atribuído: %s\n", idFinal);
            } else if (strcmp(response.status, "fail") == 0) {
                logger.error("[BIND] Falha ao obter ID.");
            }
        }
    }
}

StringView MainController::generateUUID() {
    static char uuid[37];
    unsigned long random1 = hardware.generateRandomNumber();
    unsigned long random2 = hardware.generateRandomNumber();
    snprintf(uuid, sizeof(uuid),
             "%08lx-%04lx-%04lx-%04lx-%012lx",
             random1, random1 & 0xFFFF, random2 & 0xFFFF,
             random2 & 0xFFFF, ((uint64_t)random1 << 32) | random2);
    return StringView(uuid);
}

void MainController::realizaMedicao() {
    MensagemMedicao medicao;
    medicao.medicao = hardware.readAnalog(34);
    medicao.soc = 0.0; // Placeholder for battery level
    strcpy(medicao.id, idFinal);

    char buffer[128];
    if (medicao.serialize(buffer, sizeof(buffer))) {
        mqtt.publish(TOPICO_MEDICAO, buffer);
        logger.printf("[MEDICAO] %.2f V enviada com ID %s\n", medicao.medicao, idFinal);
    }
}

void MainController::handleConexaoMQTT() {
    if (mqtt.connect("ESP32Client")) {
        logger.info("Conectado ao MQTT.");
        mqtt.subscribe(TOPICO_PROCESSO);
        mqtt.subscribe(TOPICO_BIND_RESPONSE);
        estadoAtual = AGUARDE;
    } else {
        logger.info("Falha, tentando novamente em 1s...");
        hardware.delay(1000);
    }
}

void MainController::handleBind() {
    StringView uuid = generateUUID();
    uuid.toCharArray(reqId, sizeof(reqId));

    MensagemBindRequest req;
    strcpy(req.req_id, reqId);
    strcpy(req.nome, "esp32_1");

    char buffer[256];
    if (req.serialize(buffer, sizeof(buffer))) {
        mqtt.publish(TOPICO_BIND_REQUEST, buffer);
        logger.printf("[BIND] Solicitando ID com req_id=%s\n", req.req_id);

        unsigned long start = hardware.getCurrentTime();
        while (!bindOk && hardware.getCurrentTime() - start < 5000) {
            mqtt.loop();
            hardware.delay(10);
        }

        if (bindOk) {
            estadoAtual = MEDICAO;
        } else {
            logger.error("[BIND] Timeout ao aguardar resposta, tentando novamente...");
            hardware.delay(2000);
        }
    } else {
        logger.debug("Bind payload serialization failed");
    }
}

void MainController::handleAguarde() {
    if (processoAtivo) {
        estadoAtual = BIND;
    }
}

void MainController::handleMedicao() {
    realizaMedicao();
    if (processoFinalizado) {
        estadoAtual = CLEANUP;
    } else {
        estadoAtual = DEEP_SLEEP;
    }
}

void MainController::handleDeepSleep() {
    logger.info("[SLEEP] Entrando em deep sleep por 10s...");
    hardware.deepSleep(10 * 1000000);
    // Após acordar do deep sleep, volta para o estado de medição
    estadoAtual = MEDICAO;
}

void MainController::handleCleanup() {
    logger.info("[CLEANUP] Limpando recursos...");
    mqtt.publish(TOPICO_STATUS, "cleanup");

    MensagemUnbind unbind;
    strcpy(unbind.id, idFinal);

    char buffer[128];
    if (unbind.serialize(buffer, sizeof(buffer))) {
        mqtt.publish(TOPICO_UNBIND, buffer);
        logger.printf("[UNBIND] ID %s liberado.\n", unbind.id);
    }
    
    estadoAtual = SHUTDOWN;
}

void MainController::handleShutdown() {
    mqtt.publish(TOPICO_STATUS, "shutdown");
    // Reseta todas as flags para o próximo ciclo
    processoAtivo = false;
    processoFinalizado = false;
    bindOk = false;
    // Volta para o estado inicial
    estadoAtual = CONEXAO_MQTT;
}

void MainController::loop() {
    mqtt.loop();

    switch (estadoAtual) {
        case CONEXAO_MQTT:
            handleConexaoMQTT();
            break;
            
        case BIND:
            handleBind();
            break;
            
        case AGUARDE:
            handleAguarde();
            break;
            
        case MEDICAO:
            handleMedicao();
            break;
            
        case DEEP_SLEEP:
            handleDeepSleep();
            break;
            
        case CLEANUP:
            handleCleanup();
            break;
            
        case SHUTDOWN:
            handleShutdown();
            break;
    }
}