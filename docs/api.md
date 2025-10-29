# Documentação da API

API REST para monitoramento de estufas através de sensores wireless.

## Base URL

```
http://localhost:8007
```

## Documentação Interativa

Swagger UI: `http://localhost:8007/docs`
Redoc: `http://localhost:8007/redoc`

## Endpoints

### Health Check

#### `GET /health`
Verifica o status da API.

**Resposta:**
```json
{
  "status": "ok",
  "version": "0.1.0",
  "uptime": 1234.56
}
```

---

### Processos

#### `GET /processes`
Lista todos os processos de monitoramento.

**Resposta:** `list[Process]`

#### `GET /processes/{process_id}`
Busca um processo pelo ID.

**Parâmetros:**
- `process_id` (path): ID do processo

**Resposta:** `Process`

#### `POST /processes/start`
Inicia um novo processo.

**Parâmetros:**
- `process_name` (query): Nome do processo

**Resposta:** `Process`

#### `POST /processes/end/{process_id}`
Finaliza um processo.

**Parâmetros:**
- `process_id` (path): ID do processo

**Resposta:** `200 OK`

#### `GET /processes/{process_id}/measurements`
Lista todas as medições de um processo.

**Parâmetros:**
- `process_id` (path): ID do processo

**Resposta:** `list[Measurement]`

---

### Sensores

#### `GET /sensors/{sensor_id}`
Busca um sensor pelo ID.

**Parâmetros:**
- `sensor_id` (path): ID do sensor

**Resposta:** `SensorRegistry`

#### `GET /sensors/{sensor_id}/measurements`
Lista todas as medições de um sensor.

**Parâmetros:**
- `sensor_id` (path): ID do sensor

**Resposta:** `list[Measurement]`

---

## Modelos de Dados

### Process
```json
{
  "id": 1,
  "name": "Processo 1",
  "started_at": "2025-10-29T09:00:00-03:00",
  "ended_at": "2025-10-29T10:00:00-03:00"
}
```

**Campos:**
- `id` (integer): Identificador único
- `name` (string): Nome do processo
- `started_at` (datetime): Data/hora de início (timezone São Paulo)
- `ended_at` (datetime): Data/hora de término (timezone São Paulo)

### Measurement
```json
{
  "id": 1,
  "process_id": 1,
  "sensor_id": 1,
  "rh": 65.5,
  "soc": 85.0,
  "timestamp": "2025-10-29T09:15:00-03:00"
}
```

**Campos:**
- `id` (integer, optional): Identificador único
- `process_id` (integer): ID do processo
- `sensor_id` (integer): ID do sensor
- `rh` (float): Umidade relativa (%)
- `soc` (float): Estado de carga da bateria (%)
- `timestamp` (datetime): Data/hora da medição (timezone São Paulo)

### SensorRegistry
```json
{
  "process_id": 1,
  "sensor_id": 1,
  "position": "Entrada"
}
```

**Campos:**
- `process_id` (integer): ID do processo associado
- `sensor_id` (integer): Identificador único do sensor
- `position` (string): Posição/localização do sensor

---

## Observações

- Todos os timestamps utilizam timezone **America/Sao_Paulo** (UTC-3)
- A API utiliza CORS habilitado para todas as origens
- Erros seguem o padrão HTTP (404, 500, etc.)

