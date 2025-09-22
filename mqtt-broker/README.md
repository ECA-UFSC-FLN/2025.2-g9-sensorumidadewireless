# MQTT Broker - Mosquitto

## Descrição

Esta pasta contém a configuração e arquivos do broker MQTT Eclipse Mosquitto utilizado no projeto de sensores de umidade wireless.

## Estrutura

## Estrutura

```
mqtt-broker/
├── Dockerfile           # Imagem Docker personalizada do Mosquitto
├── mosquitto.conf      # Arquivo de configuração do broker
├── test-mqtt.sh        # Script de teste interativo
├── test_broker.sh      # Script de teste automático
├── .gitignore          # Arquivos a serem ignorados pelo Git
└── README.md           # Este arquivo
```

## Arquivos

### Dockerfile
Baseado na imagem oficial `eclipse-mosquitto:2.0.18`, com configurações personalizadas para o projeto.

### mosquitto.conf
Arquivo de configuração do broker com:
- Conexões anônimas habilitadas (desenvolvimento)
- Listener MQTT na porta 1883
- Listener WebSocket na porta 9001
- Logs detalhados
- Persistência habilitada

## Uso

### Iniciar o Broker
```bash
# Subir o broker (do diretório raiz do projeto)
docker-compose up -d mosquitto

# Ver logs
docker-compose logs -f mosquitto
```

### Testar o Broker
```bash
# Teste automático
./mqtt-broker/test_broker.sh

# Teste interativo - ver opções
./mqtt-broker/test-mqtt.sh help

# Exemplos de uso
./mqtt-broker/test-mqtt.sh test           # Teste completo
./mqtt-broker/test-mqtt.sh sub-all        # Escutar tudo
./mqtt-broker/test-mqtt.sh pub-dados      # Publicar dados
```

### Estrutura de Tópicos
```
estufa_test/
├── sensores/
│   └── {sensor_id}/
│       └── dados    # {"sensor_id":"ESP32_001","timestamp":"...","umidade":67.5,"soc_bateria":85}
└── sistema/
    └── logs
```

**Nota:** Sensores em deep sleep enviam apenas dados. O próprio envio serve como heartbeat.

## Configuração de Desenvolvimento vs Produção

### Desenvolvimento (atual)
- ✅ Conexões anônimas
- ✅ Logs verbosos
- ✅ Sem TLS/SSL

### Produção (recomendado)
- ❌ Autenticação obrigatória
- ❌ TLS/SSL habilitado
- ❌ ACL configurado
- ❌ Rate limiting

Para mais detalhes, consulte a [documentação completa](../docs/mqtt-broker.md).
