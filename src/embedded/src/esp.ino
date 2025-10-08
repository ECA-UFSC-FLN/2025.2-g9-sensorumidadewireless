#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <esp_system.h>

// ===== CONFIGURAÇÕES DE REDE/MQTT =====
const char* ssid = "wifi";
const char* password = "senha";
const char* mqtt_server = "127.0.0.1";  // IP do broker
const int mqtt_port = 1883;

// ===== MQTT =====
WiFiClient espClient;
PubSubClient client(espClient);

// ===== TOPICOS MQTT =====
#define TOPICO_MEDICAO        "sensores/medicao"
#define TOPICO_PROCESSO       "sensores/processo"
#define TOPICO_BIND_REQUEST   "sensores/bind/request"
#define TOPICO_BIND_RESPONSE  "sensores/bind/response"
#define TOPICO_UNBIND         "sensores/bind/unbind"
#define TOPICO_STATUS         "sensores/status"

// ===== ESTADOS =====
enum Estado {
  CONEXAO_MQTT,
  BIND,
  AGUARDE,
  MEDICAO,
  DEEP_SLEEP,
  CLEANUP,
  SHUTDOWN
};

Estado estadoAtual = CONEXAO_MQTT;

// ===== VARIÁVEIS =====
bool processoAtivo = false;
bool processoFinalizado = false;
bool bindOk = false;
char reqId[37];
char idFinal[32]; // ID real atribuído pelo broker

// ===== STRUCTS =====
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

// ====== GERA UUID SIMPLES ======
String generateUUID() {
  char uuid[37];
  snprintf(uuid, sizeof(uuid),
           "%08lx-%04lx-%04lx-%04lx-%012lx",
           esp_random(), esp_random() & 0xFFFF, esp_random() & 0xFFFF,
           esp_random() & 0xFFFF, ((uint64_t)esp_random() << 32) | esp_random());
  return String(uuid);
}

// ===== CALLBACK MQTT =====
void callback(char* topic, byte* message, unsigned int length) {
  String msg;
  for (int i = 0; i < length; i++) {
    msg += (char)message[i];
  }

  if (String(topic) == TOPICO_PROCESSO) {
    if (msg == "iniciar") processoAtivo = true;
    if (msg == "finalizar") processoFinalizado = true;
  }

  if (String(topic) == TOPICO_BIND_RESPONSE) {
    StaticJsonDocument<256> doc;
    DeserializationError error = deserializeJson(doc, msg);
    if (error) return;

    const char* resp_req_id = doc["req_id"];
    const char* resp_id = doc["id"];
    const char* status = doc["status"];

    if (strcmp(resp_req_id, reqId) == 0 && strcmp(status, "ok") == 0) {
      strcpy(idFinal, resp_id);
      bindOk = true;
      Serial.printf("[BIND] ID atribuído: %s\n", idFinal);
    } else if (strcmp(status, "fail") == 0) {
      Serial.println("[BIND] Falha ao obter ID.");
    }
  }
}

// ===== CONEXÕES =====
void conectaWiFi() {
  Serial.printf("Conectando ao Wi-Fi %s...\n", ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi conectado!");
}

void conectaMQTT() {
  while (!client.connected()) {
    Serial.println("Conectando ao broker MQTT...");
    if (client.connect("ESP32Client")) {
      Serial.println("Conectado ao MQTT.");
      client.subscribe(TOPICO_PROCESSO);
      client.subscribe(TOPICO_BIND_RESPONSE);
    } else {
      Serial.println("Falha, tentando novamente em 1s...");
      delay(1000);
    }
  }
}

// ===== MEDIÇÃO =====
void realizaMedicao() {
  MensagemMedicao med;
  med.medicao = analogRead(34) * (3.3 / 4095.0); // Ex: tensão
  strcpy(med.id, idFinal);

  StaticJsonDocument<128> doc;
  doc["medicao"] = med.medicao;
  doc["id"] = med.id;
  doc["soc"] = 100.0; // Placeholder para estado de carga

  char buffer[128];
  serializeJson(doc, buffer);
  client.publish(TOPICO_MEDICAO, buffer);

  Serial.printf("[MEDICAO] %.2f V enviada com ID %s\n", med.medicao, med.id);
}

// ===== SETUP =====
void setup() {
  Serial.begin(115200);
  conectaWiFi();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
}

// ===== LOOP =====
void loop() {
  client.loop();

  switch (estadoAtual) {
    case CONEXAO_MQTT:
      conectaMQTT();
      estadoAtual = BIND;
      break;

    case BIND: {
      // Gera req_id e envia bind request
      String uuid = generateUUID();
      uuid.toCharArray(reqId, sizeof(reqId));

      MensagemBindRequest req;
      strcpy(req.req_id, reqId);
      strcpy(req.nome, "esp32_1");

      StaticJsonDocument<256> doc;
      doc["req_id"] = req.req_id;
      doc["nome"] = req.nome;

      char buffer[256];
      serializeJson(doc, buffer);

      client.publish(TOPICO_BIND_REQUEST, buffer);
      Serial.printf("[BIND] Solicitando ID com req_id=%s\n", req.req_id);

      unsigned long start = millis();
      while (!bindOk && millis() - start < 5000) {
        client.loop();
        delay(10);
      }

      if (bindOk) {
        estadoAtual = AGUARDE;
      } else {
        Serial.println("[BIND] Timeout ao aguardar resposta, tentando novamente...");
        delay(2000);
      }
      break;
    }

    case AGUARDE:
      if (processoAtivo) estadoAtual = MEDICAO;
      break;

    case MEDICAO:
      realizaMedicao();
      if (processoFinalizado) {
        estadoAtual = CLEANUP;
      } else {
        estadoAtual = DEEP_SLEEP;
      }
      break;

    case DEEP_SLEEP:
      Serial.println("[SLEEP] Entrando em deep sleep por 10s...");
      esp_sleep_enable_timer_wakeup(10 * 1000000);
      esp_deep_sleep_start();
      break;

    case CLEANUP:
      Serial.println("[CLEANUP] Limpando recursos...");
      client.publish(TOPICO_STATUS, "cleanup");
      estadoAtual = SHUTDOWN;
      // Envia UNBIND
      MensagemUnbind unbind;
      strcpy(unbind.id, idFinal);

      StaticJsonDocument<128> doc;
      doc["id"] = unbind.id;

      char buffer[128];
      serializeJson(doc, buffer);

      client.publish(TOPICO_UNBIND, buffer);
      Serial.printf("[UNBIND] ID %s liberado.\n", unbind.id);
      break;

    case SHUTDOWN: {
      client.publish(TOPICO_STATUS, "shutdown");
      esp_deep_sleep_start();
      break;
    }
  }
}
