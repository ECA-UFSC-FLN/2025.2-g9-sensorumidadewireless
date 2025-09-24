import React, { useMemo, useState } from 'react'
import TimeSeriesChart from './TimeSeriesChart'
import SensorRow from './SensorRow'
import { format } from 'date-fns'

function ChartProcess({process, sensors}){
  const palette = ['#2ecc71', '#ff6bcb', '#9b59b6']
  const sens = Array.isArray(sensors)? sensors: []
  
  // Prepare chart data and series config
  const chartData = useMemo(() => {
    if (!process?.messages) return []
    return process.messages
      .filter(msg => msg && msg.timestamp && msg.umidade)
      .map(msg => ({
        timestamp: msg.timestamp,
        values: {
          avg: Number(msg.umidade) || 0,
          ...sens.reduce((acc, s, idx) => {
            const base = Number(msg.umidade) || 0
            const phase = (idx % 10) / 10
            const offset = (idx % 3 - 1) * 2
            const amp = 5
            acc[s.sensor_id] = Math.max(0, Math.min(100, 
              base + Math.sin((new Date(msg.timestamp).getTime() / 1000) + phase) * amp + offset
            ))
            return acc
          }, {})
        }
      }))
  }, [process, sens])

  // Configure series appearance
  const series = useMemo(() => [
    { key: 'avg', label: 'Média', color: 'var(--accent)' },
    ...sens.map((s, idx) => ({
      key: s.sensor_id,
      label: s.sensor_id,
      color: palette[idx % palette.length]
    }))
  ], [sens])

  return (
    <TimeSeriesChart
      data={chartData}
      series={series}
      defaultWindowMs={10 * 60 * 1000} // 10 min
      autoFollow={true}
    />
  )
}

function ProcessList({processes, onOpen}){
  if(!Array.isArray(processes) || !processes.length) return <div className="muted">Nenhum processo.</div>
  return (
    <div className="process-list">
      {processes.map(p=>{
        const active = p.active
        const start = new Date(p.startedAt)
        const end = p.endedAt? new Date(p.endedAt) : new Date()
        const durMs = end - start
        const h = Math.floor(durMs/3600000)
        const m = Math.floor((durMs%3600000)/60000)
        return (
          <div key={p.id} className={`process-item ${active?'active':''}`} onClick={()=>onOpen && onOpen(p)}>
            <div className="title-row">
              <div className="title">{p.title}</div>
              <span className={`badge ${active? 'ok':''}`}>{active? 'ATIVO':'Finalizado'}</span>
            </div>
            <div className="muted">{format(start, 'yyyy-MM-dd HH:mm')} · {active? 'ativo' : 'finalizado'} · {h}h {m}m</div>
          </div>
        )
      })}
    </div>
  )
}

function ProcessDetail({process, onClose}){
  if(!process) return null
  return (
    <div className="panel">
      <div className="panel-header">{process.title} {process.active? <span className="badge ok" style={{marginLeft:8}}>ATIVO</span> : <span className="badge" style={{marginLeft:8}}>Finalizado</span>}</div>
      <div className="muted" style={{marginBottom:8}}>
        Início: {format(new Date(process.startedAt), 'yyyy-MM-dd HH:mm')} · {process.endedAt? `Fim: ${format(new Date(process.endedAt), 'yyyy-MM-dd HH:mm')}` : 'Em andamento'}
      </div>
      <div style={{display:'flex', gap:12, flexWrap:'wrap', marginBottom:8}}>
        <div className="kpi"><span className="kpi-label">Humidade média</span><span className="kpi-value">{process.summary?.avgHumidity ?? '--'}%</span></div>
        <div className="kpi"><span className="kpi-label">Bateria mínima</span><span className="kpi-value">{process.summary?.minBattery ?? '--'}%</span></div>
      </div>
      <div className="history-list">
        {process.messages?.map(m=> (
          <div key={m.id} className="history-item">
            <div className="muted">{format(new Date(m.timestamp), 'yyyy-MM-dd HH:mm')}</div>
            <div>
              <span className="chip">{m.umidade}%</span>
              <span className="chip">{m.soc_bateria}%</span>
            </div>
          </div>
        ))}
      </div>
      <div style={{marginTop:10}}>
        <button className="btn" onClick={onClose}>Fechar</button>
      </div>
    </div>
  )
}

export default function EstufaDetail({estufa, onBack}){
  if(!estufa) return null
  const [openProcess, setOpenProcess] = useState(null)
  const showingProcess = !!openProcess

  const handleBack = ()=>{
    if(showingProcess) setOpenProcess(null)
    else onBack()
  }

  return (
    <div className="estufa-detail">
      <div className="page-header">
        <button className="btn back" onClick={handleBack} aria-label={showingProcess? 'Voltar para processos':'Voltar'}>←</button>
        <h2 className="title" style={{margin:0}}>
          {estufa.id} <small className="muted">({estufa.broker})</small>
        </h2>
      </div>

      {!showingProcess && (
        <div className="panel">
          <div className="panel-header">Processos de secagem</div>
          <ProcessList processes={estufa.processes} onOpen={setOpenProcess} />
        </div>
      )}

      {showingProcess && (
        <div className="process-dashboard">
          <div className="panel">
            <div className="panel-header">Monitoramento (umidade) — {openProcess.title}</div>
            <ChartProcess process={openProcess} sensors={estufa.sensors} />
          </div>

          <div className="panel">
            <div className="panel-header">Status dos sensores (últimas medições)</div>
            <div className="sensor-list compact">
              <div className="sensor-list-head">
                <div>Sensor</div>
                <div>Umidade</div>
                <div>Bateria</div>
                <div>Status</div>
              </div>
              {estufa.sensors.map((s)=> (
                <SensorRow key={s.sensor_id} sensor={s} thresholdMin={10} showLast={false} />
              ))}
            </div>
          </div>

          

          <div className="panel">
            <div className="panel-header">Histórico do processo</div>
            <div className="history-list">
              {openProcess.messages?.slice().sort((a,b)=> new Date(b.timestamp)-new Date(a.timestamp)).map(m=> (
                <div key={m.id} className="history-item">
                  <div className="muted">{format(new Date(m.timestamp), 'yyyy-MM-dd HH:mm')}</div>
                  <div>
                    <span className="chip">{m.umidade}%</span>
                    <span className="chip">{m.soc_bateria}%</span>
                  </div>
                </div>
              ))}
            </div>
          </div>

          {/* back handled by header button */}
        </div>
      )}
    </div>
  )
}
