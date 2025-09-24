import React, { useMemo } from 'react'

export default function EstufaCard({estufa, onOpen, compact=false}){
  const { id, broker, sensors, processes = [] } = estufa

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
  const activeProcess = useMemo(()=> processes.find(p=>p.active), [processes])

  const handleOpen = ()=> onOpen && onOpen(estufa)
  const handleKey = (e)=>{
    if(e.key === 'Enter' || e.key === ' '){
      e.preventDefault(); handleOpen()
    }
  }

  return (
    <div className={`panel estufa-card clickable ${compact? 'compact':''}`} role="button" tabIndex={0} onClick={handleOpen} onKeyDown={handleKey}>
      <div className="estufa-header">
        <div>
          <h2 className="title">{id}</h2>
          {!compact && <div className="muted">Broker: {broker}</div>}
        </div>
        {compact ? (
          <div className="status-only">
            <span className={`badge ${activeProcess? 'ok':'bad'}`}>{activeProcess? 'ATIVO':'INATIVO'}</span>
          </div>
        ) : (
          <div className="estufa-kpis">
            <div className="kpi"><span className="kpi-label">Humidade média</span><span className="kpi-value">{humidityAverage}%</span></div>
            <div className="kpi"><span className="kpi-label">Sensores</span><span className="kpi-value">{sensors.length}</span></div>
            <div className="kpi"><span className="kpi-label">Bateria mínima</span><span className="kpi-value">{minBattery}%</span></div>
            <div className="kpi"><span className="kpi-label">Inativos</span><span className={`kpi-value ${inactiveCount? 'warn':''}`}>{inactiveCount}</span></div>
          </div>
        )}
      </div>
      <span className="card-chevron" aria-hidden>›</span>
    </div>
  )
}
