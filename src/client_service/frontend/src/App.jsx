import React, { useEffect, useState } from 'react'
import EstufaList from './components/EstufaList'
import EstufaDetail from './components/EstufaDetail'
import { fetchEstufasMock } from './mock/api'
import { getEstufas } from './services/api'

export default function App(){
  const [estufas, setEstufas] = useState([])
  const [loading, setLoading] = useState(true)
  const [error, setError] = useState(null)
  const [selected, setSelected] = useState(null)

  useEffect(()=>{
    async function load(){
      try{
  const data = await getEstufas()
  // coerce into array to avoid runtime .map errors
  if(Array.isArray(data)) setEstufas(data)
  else if(data && typeof data === 'object') setEstufas([data])
  else setEstufas([])
      }catch(e){
        setError(e.message)
      }finally{
        setLoading(false)
      }
    }
    load()
  },[])

  return (
    <div className="app">
      <header className="app-header">
        <h1>Estufa Dashboard</h1>
        <p>Monitoramento de múltiplas estufas - Humidade média, SOC e sensores inativos</p>
      </header>

      <main className="app-main">
        {loading && <div>Carregando...</div>}
        {error && <div className="error">Erro: {error}</div>}
        {!loading && !error && !selected && (
          <EstufaList estufas={estufas} onOpen={e=>setSelected(e)} />
        )}

        {!loading && !error && selected && (
          <EstufaDetail estufa={selected} onBack={()=>setSelected(null)} />
        )}
      </main>

      <footer className="app-footer">Fonte de dados: backend /api/estufas (mock)</footer>
    </div>
  )
}
