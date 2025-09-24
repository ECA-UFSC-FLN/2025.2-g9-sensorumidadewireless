import React, { useMemo } from 'react'

export default function EstufaCard({estufa}){
  const { id, broker, sensors } = estufa

  const humidityAverage = useMemo(()=>{
    if(!sensors || sensors.length===0) return 0
    const sum = sensors.reduce((acc,s)=> acc + (s.umidade||0),0)
    return +(sum / sensors.length).toFixed(2)
  },[sensors])

  const minBattery = useMemo(()=>{
    if(!sensors || sensors.length===0) return 0
    return sensors.reduce((min,s)=> Math.min(min, s.soc_bateria ?? 100), 100)
  },[sensors])

  const inactiveCount = useMemo(()=>{
    const thresholdMin = 10
    return sensors.filter(s=>{
      try{
        const diffMs = Date.now() - new Date(s.timestamp).getTime()
        return diffMs > thresholdMin*60*1000
      }catch(_){return true}
    }).length
  },[sensors])

  const inactiveThresholdMin = 10 // minutes

  return (
    <div className="panel estufa-card">
      <div className="estufa-header">
        <div>
          <h2 className="title">{id}</h2>
          <div className="muted">Broker: {broker}</div>
        </div>
        <div className="estufa-kpis">
          <div className="kpi"><span className="kpi-label">Humidade média</span><span className="kpi-value">{humidityAverage}%</span></div>
          <div className="kpi"><span className="kpi-label">Sensores</span><span className="kpi-value">{sensors.length}</span></div>
          <div className="kpi"><span className="kpi-label">Bateria mínima</span><span className="kpi-value">{minBattery}%</span></div>
          <div className="kpi"><span className="kpi-label">Inativos</span><span className={`kpi-value ${inactiveCount? 'warn':''}`}>{inactiveCount}</span></div>
        </div>
      </div>
    </div>
  )
}
