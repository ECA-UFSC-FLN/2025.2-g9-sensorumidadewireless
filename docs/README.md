# Documentação do Projeto

Esta pasta contém toda a documentação técnica do projeto de sensores de umidade wireless.

## Arquitetura da solução

graph LR
    subgraph Embedded["Sistema Embarcado"]
        E1["ESP32 / Sensor"]
    end

    subgraph Broker["Broker MQTT"]
        M["Tópicos / Mensagens"]
    end

    subgraph Backend["API (FastAPI / Backend)"]
        A["API REST"]
        DB[(PostgreSQL)]
    end

    subgraph Frontend["Interface Web / Dashboard"]
        F["Frontend"]
    end

    %% Fluxos de dados
    E1 -->|Publica medições| M
    M -->|Encaminha dados| A
    A -->|Armazena / Consulta| DB
    F -->|Consome endpoints| A

    %% Fluxo de controle (comandos)
    A -->|"Publica comandos: iniciar ou parar"| M
    M -->|"Entrega comandos ao embarcado"| E1

## Tech stack

- **Sistema Embarcado**: ESP32, C++ (PlatformIO)
- **Broker MQTT**: Eclipse Mosquitto (Docker)
- **Backend / API**: Python, FastAPI, SQLAlchemy, PostgreSQL
- **Frontend / Dashboard**: React (Vite), Axios, TailwindCSS

## Documentos Disponíveis

- [**MQTT Broker**](mqtt-broker.md) - Configuração e uso do broker Mosquitto
- Mais documentos serão adicionados conforme o desenvolvimento do projeto
- [**Frontend**](frontend.md) - Dashboard de monitoramento do processo
- [**Embarcado**](embarcado.md) - Código embarcado do sensor de umidade wireless

## Estrutura do Projeto

```
docs/
├── README.md           # Este arquivo
├── mqtt-broker.md     # Documentação do broker MQTT
└── ...                # Futuras documentações
```
