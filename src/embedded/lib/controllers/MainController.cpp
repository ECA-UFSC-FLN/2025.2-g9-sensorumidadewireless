#include "MainController.h"
#include <string.h>
#include <stdio.h>
#include <esp_sleep.h>

// RTC memory to retain the assigned ID across deep sleep cycles (not across power loss)
static RTC_DATA_ATTR char retainedId[32] = "";
// RTC memory to retain whether the process was started so we continue across deep sleep
static RTC_DATA_ATTR bool retainedProcessActive = false;

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

    // Restore ID from RTC memory if available (survives deep sleep)
    if (retainedId[0] != '\0') {
        strncpy(idFinal, retainedId, sizeof(idFinal) - 1);
        idFinal[sizeof(idFinal) - 1] = '\0';
        bindOk = true; // already bound
        logger.printf("[INIT] Restored ID from RTC: %s\n", idFinal);
    }
    // Restore process active flag
    if (retainedProcessActive) {
        processoAtivo = true;
        logger.printf("[INIT] Restored process active flag from RTC\n");
    }

    // If we already have an ID and the process was active before sleep, start measurements immediately
    if (bindOk && processoAtivo) {
        estadoAtual = MEDICAO;
    }
}

void MainController::staticMQTTCallback(const char* topic, const uint8_t* payload, unsigned int length) {
    if (instance) {
        instance->handleMQTTCallback(topic, payload, length);
    }
}

void MainController::handleMQTTCallback(const char* topic, const uint8_t* payload, unsigned int length) {
    // Safe, bounded copy of incoming payload into a NUL-terminated buffer
    char message[256];
    size_t safeLen = length < (sizeof(message) - 1) ? length : (sizeof(message) - 1);
    memcpy(message, payload, safeLen);
    message[safeLen] = '\0';

    // Use printf-style logger for formatted messages
    logger.printf("[MQTT-CALLBACK] Received message on topic: %s\n", topic);
    logger.printf("[MQTT-CALLBACK] Message content: %s\n", message);

    // --- Debug diagnostics ---
    if (length > safeLen) {
        logger.printf("[MQTT-Debug] payload len=%u safeLen=%u (truncated)\n", length, (unsigned)safeLen);
    } else {
        logger.printf("[MQTT-Debug] payload len=%u safeLen=%u\n", length, (unsigned)safeLen);
    }

    // Hex dump of the first up to 64 bytes
    unsigned int dumpFirst = (length < 64u) ? length : 64u;
    logger.printf("[MQTT-Debug] first %u bytes (hex): ", dumpFirst);
    for (unsigned int i = 0; i < dumpFirst; ++i) {
        logger.printf("%02X", (unsigned char)payload[i]);
        if (i + 1 < dumpFirst) logger.printf(" ");
    }
    logger.printf("\n");

    // If message is large, show last 16 bytes as well
    if (length > 64u) {
        unsigned int tailLen = (length >= 16u) ? 16u : length;
        unsigned int start = (unsigned int)length - tailLen;
        logger.printf("[MQTT-Debug] last %u bytes (hex): ", tailLen);
        for (unsigned int i = 0; i < tailLen; ++i) {
            logger.printf("%02X", (unsigned char)payload[start + i]);
            if (i + 1 < tailLen) logger.printf(" ");
        }
        logger.printf("\n");
    }

    // Check for non-printable characters in the copied NUL-terminated message
    bool nonPrintableFound = false;
    for (size_t i = 0; i < safeLen; ++i) {
        unsigned char c = (unsigned char)message[i];
        if (c < 0x20 || c > 0x7E) { nonPrintableFound = true; break; }
    }
    if (nonPrintableFound) logger.printf("[MQTT-Debug] message contains non-printable bytes\n");
    // --- end diagnostics ---

    if (strcmp(topic, TOPICO_PROCESSO) == 0) {
        if (strcmp(message, "iniciar") == 0) {
            processoAtivo = true;
            retainedProcessActive = true; // persist across deep sleep
            logger.printf("[PROCESS] Received iniciar -> processoAtivo=true\n");
            // If we're already bound, go straight to measurement
            if (bindOk) estadoAtual = MEDICAO; else estadoAtual = BIND;
        }
        if (strcmp(message, "finalizar") == 0) {
            processoFinalizado = true;
            retainedProcessActive = false; // stop persisting the process
            logger.printf("[PROCESS] Received finalizar -> processoFinalizado=true\n");
        }
    }
    else if (strcmp(topic, TOPICO_BIND_RESPONSE) == 0) {
        logger.debug("[BIND-RESPONSE] Processing bind response...");
        MensagemBindResponse response;
        if (response.deserialize(message)) {
            logger.printf("[BIND-RESPONSE] Deserialized - req_id: %s, id: %s, status: %s\n",
                          response.req_id, response.id, response.status);
            logger.printf("[BIND-RESPONSE] Expected req_id: %s\n", reqId);

            if (strcmp(response.req_id, reqId) == 0 && strcmp(response.status, "ok") == 0) {
                strcpy(idFinal, response.id);
                bindOk = true;
                logger.printf("[BIND] ID atribuído: %s\n", idFinal);
            } else if (strcmp(response.status, "fail") == 0) {
                logger.error("[BIND] Falha ao obter ID.");
            } else {
                logger.debug("[BIND-RESPONSE] req_id mismatch or status not ok");
            }
        } else {
            logger.error("[BIND-RESPONSE] Failed to deserialize response");
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
    // Persist ID to RTC memory so we don't rebind after wake
    if (idFinal[0] != '\0') {
        strncpy(retainedId, idFinal, sizeof(retainedId) - 1);
        retainedId[sizeof(retainedId) - 1] = '\0';
        logger.printf("[SLEEP] Persisting ID to RTC: %s\n", retainedId);
    }

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