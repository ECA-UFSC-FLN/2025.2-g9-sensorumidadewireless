import React from 'react'
import { formatDistanceToNowStrict, parseISO } from 'date-fns'

export default function SensorRow({sensor, thresholdMin, showLast=true}){
  const lastSeen = sensor.timestamp
  let timeAgo = 'N/A'
  try{
    timeAgo = formatDistanceToNowStrict(parseISO(lastSeen), { addSuffix: true })
  }catch(e){ /* ignore */ }

  const inactive = (()=>{
    try{
      const diffMs = Date.now() - new Date(lastSeen).getTime()
      return diffMs > thresholdMin * 60 * 1000
    }catch(e){return true}
  })()

  return (
    <div className={`sensor-row ${inactive? 'inactive':''} ${!showLast? 'no-last':''}`}>
      <div>{sensor.sensor_id}</div>
      <div>{sensor.umidade}%</div>
      <div>{sensor.soc_bateria}%</div>
      {showLast && <div>{timeAgo}</div>}
      <div>{inactive? <span className="badge bad">INATIVO</span> : <span className="badge ok">OK</span>}</div>
    </div>
  )
}
