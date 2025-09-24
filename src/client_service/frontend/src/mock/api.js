// Mock API response to emulate backend /api/estufas
export async function fetchEstufasMock(){
  // simulate network
  await new Promise(r=>setTimeout(r,300))
  const now = new Date()
  const iso = d=> new Date(d).toISOString()

  return [
    {
      id: 'estufa_test',
      broker: 'localhost:1883',
      sensors: Array.from({length:8}).map((_,i)=>({
        sensor_id: `ESP32_${String(i+1).padStart(3,'0')}`,
        umidade: +(50 + Math.random()*30).toFixed(1),
        soc_bateria: Math.floor(30 + Math.random()*70),
        timestamp: iso(new Date(now.getTime() - Math.random()*1000*60*15))
      }))
    },
    {
      id: 'estufa_1',
      broker: '10.0.0.50:1883',
      sensors: Array.from({length:8}).map((_,i)=>({
        sensor_id: `ESP32_${String(i+9).padStart(3,'0')}`,
        umidade: +(45 + Math.random()*40).toFixed(1),
        soc_bateria: Math.floor(20 + Math.random()*80),
        timestamp: iso(new Date(now.getTime() - Math.random()*1000*60*60))
      }))
    }
  ]
}
