FROM eclipse-mosquitto:2.0.18

# Criar diretórios necessários
RUN mkdir -p /mosquitto/config /mosquitto/data /mosquitto/log

# Copiar arquivo de configuração personalizado
COPY mosquitto.conf /mosquitto/config/mosquitto.conf

# Definir usuário
USER mosquitto

# Expor portas
EXPOSE 1883 9001

# Comando padrão
CMD ["/usr/sbin/mosquitto", "-c", "/mosquitto/config/mosquitto.conf"]
