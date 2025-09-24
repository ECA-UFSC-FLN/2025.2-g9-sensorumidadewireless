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

  function genRealisticProcess(baseId, hoursAgoStart, durationHours, active=false){
    const start = new Date(now.getTime() - hoursAgoStart*3600*1000)
    const end = active? now : new Date(start.getTime() + durationHours*3600*1000)
    
    // Gerar pontos a cada minuto
    const totalMinutes = Math.floor((end - start) / (60 * 1000))
    
    // Função para gerar uma curva de secagem realista
    function generateDryingCurve(initialHumidity, minutes) {
      // Parâmetros da curva de secagem
      const finalHumidity = 12 + Math.random() * 3 // Meta final entre 12-15%
      const decayRate = 0.0005 + Math.random() * 0.0003 // Taxa de decaimento variável
      const noiseAmplitude = 2 // Variação aleatória para simular ruído
      const cycleAmplitude = 3 // Amplitude da variação cíclica
      const cyclePeriod = 120 // Período do ciclo em minutos
      
      return Array.from({ length: minutes }).map((_, i) => {
        // Componente exponencial (curva principal de secagem)
        const exponentialComponent = (initialHumidity - finalHumidity) * Math.exp(-decayRate * i) + finalHumidity
        
        // Componente cíclica (variações periódicas)
        const cyclicComponent = cycleAmplitude * Math.sin(2 * Math.PI * i / cyclePeriod)
        
        // Ruído aleatório
        const noise = (Math.random() - 0.5) * 2 * noiseAmplitude
        
        // Combinar componentes
        const humidity = exponentialComponent + cyclicComponent + noise
        
        // Garantir que está dentro dos limites realistas (10-90%)
        return Math.max(10, Math.min(90, humidity))
      })
    }
    
    // Gerar valores de umidade para cada sensor
    const initialHumidity = 75 + Math.random() * 10
    const mainCurve = generateDryingCurve(initialHumidity, totalMinutes)
    
    // Gerar mensagens
    const messages = Array.from({ length: totalMinutes }).map((_, i) => {
      const t = new Date(start.getTime() + i * 60 * 1000) // Incremento de 1 minuto
      const baseHumidity = mainCurve[i]
      
      // Bateria diminui gradualmente ao longo do tempo
      const timeFactor = i / totalMinutes
      const baseBattery = Math.max(20, 100 - (30 * timeFactor) - (Math.random() * 10))
      
      return {
        id: `${baseId}-m${i}`,
        timestamp: iso(t),
        umidade: +baseHumidity.toFixed(2),
        soc_bateria: +baseBattery.toFixed(2)
      }
    })

    // Calcular métricas do processo
    const avgHumidity = +(messages.reduce((sum, m) => sum + m.umidade, 0) / messages.length).toFixed(1)
    const minBattery = Math.floor(Math.min(...messages.map(m => m.soc_bateria)))

    return {
      id: baseId,
      title: `Processo ${baseId.split('_').pop()}`,
      startedAt: iso(start),
      endedAt: active ? null : iso(end),
      active,
      summary: {
        avgHumidity,
        minBattery
      },
      messages
    }
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
        genRealisticProcess('proc_estufa_test_001', 3, 80, true), // Processo atual de 80 horas
        genRealisticProcess('proc_estufa_test_000', 100, 85, false), // Processo anterior de 85 horas
      ]
    },
    {
      id: 'estufa_1',
      broker: '10.0.0.50:1883',
      sensors: genSensors(9),
      processes: [
        genRealisticProcess('proc_estufa_1_002', 6, 72, false),
        genRealisticProcess('proc_estufa_1_001', 90, 82, false),
      ]
    }
  ]
}
