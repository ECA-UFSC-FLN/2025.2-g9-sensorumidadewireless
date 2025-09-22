# MQTT Broker - Mosquitto

## Visão Geral

Este projeto utiliza o broker MQTT Eclipse Mosquitto para comunicação entre os sensores de umidade wireless e outros componentes do sistema. O Mosquitto é um broker MQTT leve e open-source que implementa o protocolo MQTT versões 5.0, 3.1.1 e 3.1.

## Configuração do Broker

### Arquitetura

```
┌─────────────────┐    MQTT    ┌──────────────────┐    MQTT    ┌─────────────────┐
│   Sensores de   │ ────────── │   Mosquitto      │ ────────── │   Aplicação     │
│   Umidade       │            │   Broker         │            │   Cliente       │
│   (ESP32/etc)   │            │   (Docker)       │            │                 │
└─────────────────┘            └──────────────────┘            └─────────────────┘
```

### Configurações do Mosquitto

O broker está configurado com as seguintes especificações:

- **Porta**: 1883 (MQTT padrão)
- **Porta WebSocket**: 9001 (para clientes web)
- **Autenticação**: Desabilitada (para desenvolvimento)
- **Logs**: Habilitados para debugging
- **Persistência**: Habilitada

### Estrutura de Tópicos

```
estufa_1/
├── sensores/
│   └── {sensor_id}/
│       └── dados          # Dados do sensor (umidade, timestamp, SOC)
├── configuracao/
│   ├── intervalo          # Intervalo de leitura
│   └── calibracao         # Parâmetros de calibração
└── sistema/
    └── logs               # Logs do sistema
```

**Exemplo para ambiente de teste:**
```
estufa_test/
├── sensores/
│   └── {sensor_id}/
│       └── dados
└── sistema/
    └── logs
```

### Formato das Mensagens

#### Dados do Sensor
```json
{
  "sensor_id": "ESP32_001",
  "timestamp": "2025-09-22T10:30:00Z",
  "umidade": 65.5,
  "soc_bateria": 85
}
```

**Notas:**
- A localização não é incluída nos dados pois está implícita no tópico (ex: `estufa_1` ou `estufa_test`)
- Não há tópico de status separado - o próprio envio dos dados serve como heartbeat
- Sensores em deep sleep enviam dados periodicamente e depois adormecem para economizar bateria

### Funcionamento com Deep Sleep

Os sensores ESP32 operam em modo **deep sleep** para maximizar a duração da bateria:

1. **Acordar** - Sensor acorda por timer ou evento
2. **Conectar** - Conecta ao WiFi 
3. **Medir** - Lê umidade e verifica SOC da bateria
4. **Transmitir** - Envia dados via MQTT
5. **Adormecer** - Entra em deep sleep por período configurado

**Vantagens:**
- ✅ Maior duração da bateria
- ✅ Menor tráfego de rede
- ✅ Simplicidade no protocolo

**Implicações:**
- ⚠️ Sensor não responde a comandos durante o sleep
- ⚠️ Detecção de falha baseada em timeout do último dado recebido
- ⚠️ Configurações devem ser enviadas quando sensor acordar

## Instalação e Execução

### Pré-requisitos

- Docker
- Docker Compose

### Executando o Broker

1. **Clone o repositório** (se ainda não fez):
   ```bash
   git clone <repository-url>
   cd 2025.2-g9-sensorumidadewireless
   ```

2. **Inicie o broker**:
   ```bash
   docker-compose up -d mosquitto
   ```

3. **Verifique se está rodando**:
   ```bash
   docker-compose ps
   docker-compose logs mosquitto
   ```

### Testando a Conexão

#### Usando mosquitto_pub e mosquitto_sub

**Terminal 1 - Subscriber:**
```bash
# Instalar mosquitto-clients se necessário
sudo apt-get install mosquitto-clients

# Escutar mensagens de todos os sensores da estufa de teste
mosquitto_sub -h localhost -p 1883 -t "estufa_test/sensores/+/dados"
```

**Terminal 2 - Publisher:**
```bash
# Publicar uma mensagem de teste de um sensor
mosquitto_pub -h localhost -p 1883 -t "estufa_test/sensores/ESP32_001/dados" -m '{"sensor_id":"ESP32_001","timestamp":"2025-09-22T10:30:00Z","umidade":67.5,"soc_bateria":85}'
```

#### Comandos Úteis para Teste

```bash
# Escutar todos os tópicos da estufa de teste
mosquitto_sub -h localhost -p 1883 -t "estufa_test/+"

# Escutar apenas dados dos sensores (principal)
mosquitto_sub -h localhost -p 1883 -t "estufa_test/sensores/+/dados"

# Publicar dados de sensor específico
mosquitto_pub -h localhost -p 1883 -t "estufa_test/sensores/ESP32_001/dados" \
  -m '{"sensor_id":"ESP32_001","timestamp":"'$(date -Iseconds)'","umidade":72.3,"soc_bateria":88}'

# Escutar sensor específico
mosquitto_sub -h localhost -p 1883 -t "estufa_test/sensores/ESP32_001/dados"
```

## Configurações de Desenvolvimento vs Produção

### Desenvolvimento
- Autenticação desabilitada
- Logs detalhados habilitados
- Porta 1883 exposta

### Produção (Recomendações)
- Habilitar autenticação com usuário/senha
- Configurar TLS/SSL (porta 8883)
- Limitar conexões simultâneas
- Configurar ACL (Access Control List)
- Habilitar logs de auditoria

### Exemplo de Configuração de Produção

```conf
# mosquitto.conf para produção
listener 8883
protocol mqtt
cafile /mosquitto/certs/ca.crt
certfile /mosquitto/certs/server.crt
keyfile /mosquitto/certs/server.key
require_certificate true

password_file /mosquitto/config/passwords
acl_file /mosquitto/config/acl

log_dest file /mosquitto/log/mosquitto.log
log_type error
log_type warning
log_type notice
log_type information
log_timestamp true
```

## Monitoramento e Logs

### Visualizando Logs
```bash
# Logs em tempo real
docker-compose logs -f mosquitto

# Logs das últimas 100 linhas
docker-compose logs --tail=100 mosquitto
```

### Métricas Importantes
- Número de clientes conectados
- Taxa de mensagens por segundo
- Uso de memória
- Latência de mensagens

## Troubleshooting

### Problemas Comuns

1. **Broker não inicia**
   - Verificar se a porta 1883 não está sendo usada
   - Verificar logs: `docker-compose logs mosquitto`

2. **Clientes não conseguem conectar**
   - Verificar firewall
   - Verificar se o container está rodando
   - Testar conectividade: `telnet localhost 1883`

3. **Mensagens não são entregues**
   - Verificar tópicos (case-sensitive)
   - Verificar QoS settings
   - Verificar se cliente está subscrito corretamente

### Comandos Úteis

```bash
# Parar o broker
docker-compose stop mosquitto

# Reiniciar o broker
docker-compose restart mosquitto

# Ver configuração ativa
docker-compose exec mosquitto cat /mosquitto/config/mosquitto.conf

# Entrar no container
docker-compose exec mosquitto sh
```

## Comandos Rápidos para Desenvolvimento

### Iniciar o Sistema
```bash
# Subir o broker
docker-compose up -d mosquitto

# Verificar status
docker-compose ps
```

### Testar Publicação/Subscrição
```bash
# Terminal 1 - Escutar dados dos sensores da estufa de teste
mosquitto_sub -h localhost -p 1883 -t "estufa_test/sensores/+/dados"

# Terminal 2 - Publicar dados de teste
mosquitto_pub -h localhost -p 1883 -t "estufa_test/sensores/ESP32_001/dados" \
  -m '{"sensor_id":"ESP32_001","timestamp":"'$(date -Iseconds)'","umidade":67.5,"soc_bateria":85}'

# Escutar todos os tópicos da estufa de teste
mosquitto_sub -h localhost -p 1883 -t "estufa_test/+"
```

### Executar Script de Teste
```bash
# Executar teste automatizado
./mqtt-broker/test_broker.sh
```

## Recursos Adicionais

````

- [Documentação oficial do Mosquitto](https://mosquitto.org/documentation/)
- [MQTT Protocol Specification](https://mqtt.org/mqtt-specification/)
- [Paho MQTT Clients](https://www.eclipse.org/paho/)
