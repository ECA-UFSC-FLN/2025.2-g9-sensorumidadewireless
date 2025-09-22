#!/bin/bash

# Script de teste simples para o broker MQTT
# Sistema de Sensores de Umidade Wireless

BROKER_HOST="localhost"
BROKER_PORT="1883"
ESTUFA="estufa_test"

echo "🌿 Teste do Broker MQTT - $ESTUFA"
echo "======================================"

# Função para mostrar ajuda
show_help() {
    echo "Uso: $0 [comando]"
    echo ""
    echo "Comandos:"
    echo "  pub-dados    - Publica dados de sensor"
    echo "  sub-dados    - Escuta dados dos sensores"
    echo "  sub-all      - Escuta todos os tópicos"
    echo "  test         - Executa teste automático"
    echo "  help         - Mostra esta ajuda"
}

# Função para verificar se mosquitto-clients está instalado
check_tools() {
    if ! command -v mosquitto_pub &> /dev/null; then
        echo "❌ mosquitto_pub não encontrado"
        echo "💡 Instale com: sudo apt-get install mosquitto-clients"
        exit 1
    fi
}

# Função para publicar dados de sensor
publish_data() {
    local sensor_id=${1:-"ESP32_001"}
    local umidade=${2:-$(awk 'BEGIN{srand(); print int(rand()*40)+40}')}
    local soc=${3:-$(awk 'BEGIN{srand(); print int(rand()*80)+20}')}
    
    local topic="$ESTUFA/sensores/$sensor_id/dados"
    local message="{\"sensor_id\":\"$sensor_id\",\"timestamp\":\"$(date -Iseconds)\",\"umidade\":$umidade,\"soc_bateria\":$soc}"
    
    echo "📤 Publicando dados do sensor $sensor_id..."
    if mosquitto_pub -h $BROKER_HOST -p $BROKER_PORT -t "$topic" -m "$message"; then
        echo "✅ Dados publicados:"
        echo "   Tópico: $topic"
        echo "   Umidade: $umidade%"
        echo "   SOC Bateria: $soc%"
    else
        echo "❌ Erro ao publicar dados"
    fi
}



# Função para escutar dados
subscribe_data() {
    echo "👂 Escutando dados dos sensores..."
    echo "💡 Pressione Ctrl+C para parar"
    echo "---"
    mosquitto_sub -h $BROKER_HOST -p $BROKER_PORT -t "$ESTUFA/sensores/+/dados" -v
}

# Função para escutar tudo
subscribe_all() {
    echo "👂 Escutando todos os tópicos da $ESTUFA..."
    echo "💡 Pressione Ctrl+C para parar"
    echo "---"
    mosquitto_sub -h $BROKER_HOST -p $BROKER_PORT -t "$ESTUFA/+" -v
}

# Função de teste automático
run_test() {
    echo "🧪 Executando teste automático..."
    echo ""
    
    # Verificar se broker está rodando
    if ! timeout 3s mosquitto_pub -h $BROKER_HOST -p $BROKER_PORT -t "test/ping" -m "ping" &>/dev/null; then
        echo "❌ Broker não está acessível em $BROKER_HOST:$BROKER_PORT"
        echo "💡 Verifique se o Docker Compose está rodando: docker-compose ps"
        exit 1
    fi
    
    echo "✅ Broker está rodando"
    echo ""
    
    # Publicar dados de teste de múltiplos sensores
    echo "📡 Simulando dados de sensores em deep sleep..."
    publish_data "ESP32_001" "68" "92"
    echo ""
    
    publish_data "ESP32_002" "71" "87"
    echo ""
    
    publish_data "ESP32_003" "65" "76"
    echo ""
    
    echo "✅ Teste concluído!"
    echo ""
    echo "📋 Para monitorar mensagens, execute:"
    echo "   $0 sub-all"
}

# Verificar ferramentas
check_tools

# Processar comando
case "$1" in
    "pub-dados")
        publish_data "$2" "$3" "$4"
        ;;
    "sub-dados")
        subscribe_data
        ;;
    "sub-all")
        subscribe_all
        ;;
    "test")
        run_test
        ;;
    "help"|"")
        show_help
        ;;
    *)
        echo "❌ Comando desconhecido: $1"
        echo ""
        show_help
        exit 1
        ;;
esac
