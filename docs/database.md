# Documentação do Banco de Dados

## Visão Geral

O sistema utiliza PostgreSQL como banco de dados relacional para armazenar processos, sensores e medições de umidade.

## Configuração

**Variável de ambiente:** `DATABASE_URL`

**Formato:** `postgresql://user:password@host:port/database`

**Padrão:** `postgresql://root:root@localhost:5432/estufa`

O banco de dados é criado automaticamente na inicialização se não existir.

## Schema

### Tabelas

#### `processes`
Armazena processos de monitoramento.

| Coluna      | Tipo      | Descrição              |
|-------------|-----------|------------------------|
| `id`        | Integer   | Chave primária         |
| `name`      | String    | Nome do processo       |
| `started_at`| DateTime  | Data/hora de início    |
| `ended_at`  | DateTime  | Data/hora de término   |

#### `sensor_registry`
Registro de sensores associados a processos.

| Coluna      | Tipo      | Descrição                    |
|-------------|-----------|------------------------------|
| `sensor_id` | Integer   | Chave primária               |
| `process_id`| Integer   | FK → `processes.id`          |
| `position`  | String    | Posição do sensor            |

**Nota:** Cada sensor pode estar associado a apenas um processo.

#### `measurements`
Armazena medições de sensores.

| Coluna      | Tipo      | Descrição                           |
|-------------|-----------|-------------------------------------|
| `id`        | Integer   | Chave primária (auto-incremento)    |
| `process_id`| Integer   | FK → `processes.id`                 |
| `sensor_id` | Integer   | FK → `sensor_registry.sensor_id`    |
| `rh`        | Float     | Umidade relativa                    |
| `soc`       | Float     | Estado de carga (bateria)            |
| `timestamp` | DateTime  | Data/hora da medição                |

## Relacionamentos

```mermaid
erDiagram
    processes ||--o{ sensor_registry : "tem"
    processes ||--o{ measurements : "contém"
    sensor_registry ||--o{ measurements : "gera"
    
    processes {
        integer id PK
        string name
        datetime started_at
        datetime ended_at
    }
    
    sensor_registry {
        integer sensor_id PK
        integer process_id FK
        string position
    }
    
    measurements {
        integer id PK
        integer process_id FK
        integer sensor_id FK
        float rh
        float soc
        datetime timestamp
    }
```

## Inicialização

As tabelas são criadas automaticamente na inicialização da API Python através de SQLAlchemy ORM (`Base.metadata.create_all()`).

**Comportamento:**
- Cria banco de dados se não existir
- Cria tabelas se não existirem
- Não modifica tabelas existentes (idempotente)

## Localização do Código

- Modelos: `src/client_service/backend/app/services/database/tables/`
- Cliente DB: `src/client_service/backend/app/services/database/psg_client.py`
- Inicialização: `src/client_service/backend/app/services/database/init_db.py`
