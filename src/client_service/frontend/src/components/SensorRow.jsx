import React from 'react'
import { formatDistanceToNow, parseISO } from 'date-fns'

export default function SensorRow({ sensor, thresholdMin = 10, showLast = true }) {
  const active = sensor.active !== false
  const battery = typeof sensor.soc_bateria === 'number' ? sensor.soc_bateria : 100
  const humidity = typeof sensor.umidade === 'number' ? sensor.umidade : (sensor.last_umidade ?? null)
  const lastSeen = sensor.last_seen || sensor.ultima_leitura

  const humClass = humidity == null ? '' : (humidity < thresholdMin ? 'bad' : 'ok')

  const status = active ? 'ATIVO' : 'INATIVO'

  return (
    <div className={`sensor-row ${showLast ? '' : 'no-last'}`}>
      <div className="sensor-name">{sensor.name || sensor.sensor_id}</div>
      <div className={`sensor-hum ${humClass}`}>{humidity == null ? '—' : `${humidity}%`}</div>
      <div className="sensor-batt">{battery}%</div>
      <div className={`sensor-status ${active ? 'ok' : 'bad'}`}>{status}</div>
      {showLast && (
        <div className="sensor-last">
          {lastSeen ? formatDistanceToNow(parseISO(lastSeen), { addSuffix: true }) : '—'}
        </div>
      )}
    </div>
  )
}
