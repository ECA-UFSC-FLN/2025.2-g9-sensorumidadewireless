import React from 'react'
import EstufaCard from './EstufaCard'

export default function EstufaList({estufas, onOpen}){
  const list = Array.isArray(estufas) ? estufas : []

  if(!list.length) return (
    <div className="estufa-list">
      <div>Nenhuma estufa disponível.</div>
    </div>
  )

  return (
    <div className="estufa-list">
      {list.map(e=> (
        <div key={e.id} className="estufa-list-item">
          <EstufaCard estufa={e} />
          <button className="btn" onClick={()=>onOpen && onOpen(e)}>Abrir {e.id}</button>
        </div>
      ))}
    </div>
  )
}
