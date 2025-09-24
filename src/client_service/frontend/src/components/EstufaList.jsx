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
    <div className="estufa-column">
      {list.map(e=> (
        <EstufaCard key={e.id} estufa={e} onOpen={onOpen} compact={true} />
      ))}
    </div>
  )
}
