#!/bin/bash

# Script de teste para o broker MQTT
# Autor: Sistema de Sensores de Umidade Wireless - Estufa
# Data: 2025-09-22

echo "=== Teste do Broker MQTT Mosquitto - Estufa ==="
echo

# Verificar se o broker está rodando
echo "1. Verificando se o broker está ativo..."
if docker compose ps mosquitto | grep -q "Up"; then
    echo "✓ Broker Mosquitto está rodando"
else
    echo "✗ Broker não está ativo. Iniciando..."
    docker-compose up -d mosquitto
    sleep 5
fi

echo

# Teste de publicação - dados do sensor ESP32_001
echo "2. Testando publicação de dados do sensor ESP32_001..."
docker exec -it mosquitto_broker mosquitto_pub \
    -h localhost \
    -t "estufa_test/sensores/ESP32_001/dados" \
    -m '{"sensor_id":"ESP32_001","timestamp":"'$(date -Iseconds)'","umidade":67.5,"soc_bateria":85}'

echo "✓ Dados publicados no tópico: estufa_test/sensores/ESP32_001/dados"
echo

# Teste de publicação - dados do sensor ESP32_002
echo "3. Testando publicação de dados do sensor ESP32_002..."
docker exec -it mosquitto_broker mosquitto_pub \
    -h localhost \
    -t "estufa_test/sensores/ESP32_002/dados" \
    -m '{"sensor_id":"ESP32_002","timestamp":"'$(date -Iseconds)'","umidade":71.2,"soc_bateria":78}'

echo "✓ Dados publicados no tópico: estufa_test/sensores/ESP32_002/dados"
echo

# Listar logs do broker
echo "4. Verificando logs do broker..."
docker compose logs --tail=10 mosquitto

echo
echo "=== Teste concluído ==="
echo "Para testar manualmente:"
echo "- Subscribe dados: mosquitto_sub -h localhost -p 1883 -t 'estufa_test/sensores/+/dados'"
echo "- Subscribe tudo: mosquitto_sub -h localhost -p 1883 -t 'estufa_test/+'"
echo "- Publish dados: mosquitto_pub -h localhost -p 1883 -t 'estufa_test/sensores/ESP32_001/dados' -m '{\"sensor_id\":\"ESP32_001\",\"timestamp\":\"$(date -Iseconds)\",\"umidade\":70.0,\"soc_bateria\":90}'"
