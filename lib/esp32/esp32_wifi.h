#ifndef ESP32_WIFI_H
#define ESP32_WIFI_H

#include "../comm/wifi_interface.h"
#include "../utils/logger_interface.h"

class ESP32WiFi : public IWiFi {
public:
    /**
     * @brief Construtor
     * @param logger Interface de logging para registro de eventos
     */
    ESP32WiFi(ILogger& logger);

    /**
     * @brief Conecta ao WiFi com as credenciais configuradas
     * @return true se conectado com sucesso, false caso contrário
     */
    bool connect() override;

    /**
     * @brief Desconecta do WiFi
     */
    void disconnect() override;

    /**
     * @brief Verifica se está conectado ao WiFi
     * @return true se conectado, false caso contrário
     */
    bool isConnected() override;

    /**
     * @brief Obtém o endereço IP atual
     * @return String contendo o endereço IP
     */
    const char* getIPAddress() override;

    /**
     * @brief Configura as credenciais do WiFi
     * @param ssid Nome da rede
     * @param password Senha da rede
     */
    void setCredentials(const char* ssid, const char* password) override;

private:
    ILogger& _logger;
    const char* _ssid;
    const char* _password;
};

#endif // ESP32_WIFI_H