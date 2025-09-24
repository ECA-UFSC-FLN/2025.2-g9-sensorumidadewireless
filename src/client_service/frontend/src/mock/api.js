// Mock API response to emulate backend /api/estufas
export async function fetchEstufasMock(){
  // simulate network
  await new Promise(r=>setTimeout(r,300))
  const now = new Date()
  const iso = d=> new Date(d).toISOString()

  function genSensors(startIndex=1){
    return Array.from({length:8}).map((_,i)=>({
      sensor_id: `ESP32_${String(startIndex+i).padStart(3,'0')}`,
      umidade: +(45 + Math.random()*40).toFixed(1),
      soc_bateria: Math.floor(20 + Math.random()*80),
      timestamp: iso(new Date(now.getTime() - Math.random()*1000*60*30))
    }))
  }

  function genProcess(baseId, hoursAgoStart, durationHours, active=false){
    const start = new Date(now.getTime() - hoursAgoStart*3600*1000)
    const end = active? null : new Date(start.getTime() + durationHours*3600*1000)
    const messages = Array.from({length:12}).map((_,i)=>{
      const t = new Date((end||now).getTime() - i*15*60*1000)
      return {
        id: `${baseId}-m${i}`,
        timestamp: iso(t),
        umidade: +(45 + Math.random()*40).toFixed(1),
        soc_bateria: Math.floor(20 + Math.random()*80)
      }
    }).reverse()
    return {
      id: baseId,
      title: `Processo ${baseId.split('_').pop()}`,
      startedAt: iso(start),
      endedAt: end? iso(end) : null,
      active,
      summary: {
        avgHumidity: +(50 + Math.random()*20).toFixed(1),
        minBattery: Math.floor(20 + Math.random()*50)
      },
      messages
    }
  }

  return [
    {
      id: 'estufa_test',
      broker: 'localhost:1883',
      sensors: genSensors(1),
      processes: [
        genProcess('proc_estufa_test_001', 3, 3, true),
        genProcess('proc_estufa_test_000', 28, 4, false),
      ]
    },
    {
      id: 'estufa_1',
      broker: '10.0.0.50:1883',
      sensors: genSensors(9),
      processes: [
        genProcess('proc_estufa_1_002', 6, 2, false),
        genProcess('proc_estufa_1_001', 50, 6, false),
      ]
    }
  ]
}
