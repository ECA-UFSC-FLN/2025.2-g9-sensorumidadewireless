FROM eclipse-mosquitto:2.0.18

# Criar usuário mosquitto se não existir
USER root
RUN addgroup -g 1883 -S mosquitto 2>/dev/null || true
RUN adduser -u 1883 -D -S -G mosquitto mosquitto 2>/dev/null || true

# Criar diretórios necessários
RUN mkdir -p /mosquitto/config /mosquitto/data /mosquitto/log
RUN chown -R mosquitto:mosquitto /mosquitto

# Copiar arquivo de configuração personalizado
COPY mosquitto.conf /mosquitto/config/mosquitto.conf

# Definir usuário
USER mosquitto

# Expor portas
EXPOSE 1883 9001

# Comando padrão
CMD ["/usr/sbin/mosquitto", "-c", "/mosquitto/config/mosquitto.conf"]
