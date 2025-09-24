Estufa Dashboard (frontend)

Este é um scaffold React (Vite) para o dashboard de monitoramento de estufas.

Como rodar localmente:

1. cd src/client_service/frontend
2. npm install
3. npm run dev

Contrato mínimo de API esperado (backend a implementar):
GET /api/estufas -> [
  {
    id: string,
    broker: string,
    sensors: [
      { sensor_id, umidade, soc_bateria, timestamp }
    ]
  }
]

Por enquanto o frontend usa dados mock em `src/mock/api.js`. Quando o backend estiver pronto, substitua `fetchEstufasMock` por uma chamada real com `axios.get('/api/estufas')`.
