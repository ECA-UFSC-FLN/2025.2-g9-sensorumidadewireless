#ifndef WIFI_INTERFACE_H
#define WIFI_INTERFACE_H

#include <stdint.h>

class IWiFi {
public:
    virtual ~IWiFi() = default;

    /**
     * @brief Conecta ao WiFi com as credenciais configuradas
     * @return true se conectado com sucesso, false caso contrário
     */
    virtual bool connect() = 0;

    /**
     * @brief Desconecta do WiFi
     */
    virtual void disconnect() = 0;

    /**
     * @brief Verifica se está conectado ao WiFi
     * @return true se conectado, false caso contrário
     */
    virtual bool isConnected() = 0;

    /**
     * @brief Obtém o endereço IP atual
     * @return String contendo o endereço IP
     */
    virtual const char* getIPAddress() = 0;

    /**
     * @brief Configura as credenciais do WiFi
     * @param ssid Nome da rede
     * @param password Senha da rede
     */
    virtual void setCredentials(const char* ssid, const char* password) = 0;
};

#endif // WIFI_INTERFACE_H