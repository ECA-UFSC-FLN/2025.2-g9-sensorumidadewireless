import React, { useMemo } from 'react'
import SensorRow from './SensorRow'
import { format } from 'date-fns'

function ChartMini({sensors}){
  // simple inline SVG sparkline using last N humidity values (mocked from sensors timestamps)
  const points = useMemo(()=>{
    // Create a synthetic series from sensor.umidade
    const vals = sensors.map(s=> s.umidade || 0)
    if(!vals.length) return ''
    const max = Math.max(...vals, 100)
    const min = Math.min(...vals, 0)
    const w = 280
    const h = 60
    return vals.map((v,i)=>{
      const x = (i/(vals.length-1)) * w
      const y = h - ((v-min)/(max-min||1))*h
      return `${x},${y}`
    }).join(' ')
  },[sensors])
  return (
    <svg viewBox="0 0 280 60" className="spark">
      <polyline fill="none" stroke="var(--accent)" strokeWidth="2" points={points} />
    </svg>
  )
}

export default function EstufaDetail({estufa, onBack}){
  if(!estufa) return null
  return (
    <div className="estufa-detail">
      <button className="btn" onClick={onBack}>← Voltar</button>
      <h2 className="title">{estufa.id} <small className="muted">({estufa.broker})</small></h2>

      <div className="grid">
        <div className="panel">
          <div className="panel-header">Monitoramento (umidade)</div>
          <ChartMini sensors={estufa.sensors} />
        </div>

        <div className="panel">
          <div className="panel-header">Status dos sensores</div>
          <div className="sensor-list compact">
            <div className="sensor-list-head">
              <div>Sensor</div>
              <div>Umidade</div>
              <div>Bateria</div>
              <div>Status</div>
            </div>
            {estufa.sensors.map(s=> (
              <SensorRow key={s.sensor_id} sensor={s} thresholdMin={10} showLast={false} />
            ))}
          </div>
        </div>
      </div>

      <div className="panel">
        <div className="panel-header">Histórico de mensagens</div>
        <div className="history-list">
          {estufa.sensors.map(s=> (
            <div key={`h-${s.sensor_id}`} className="history-item">
              <div className="muted">{s.sensor_id}</div>
              <div>
                <span className="chip">{s.umidade}%</span>
                <span className="chip">{s.soc_bateria}%</span>
                <span className="chip">{format(new Date(s.timestamp), 'yyyy-MM-dd HH:mm')}</span>
              </div>
            </div>
          ))}
        </div>
      </div>
    </div>
  )
}
