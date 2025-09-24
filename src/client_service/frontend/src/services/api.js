import axios from 'axios'
import { fetchEstufasMock } from '../mock/api'

export async function getEstufas(){
  try{
    const res = await axios.get('/api/estufas')
    if(res && res.data){
      const d = res.data
      // if backend returns array directly
      if(Array.isArray(d)) return d
      // if backend wraps result: { estufas: [...] }
      if(d && Array.isArray(d.estufas)) return d.estufas
      // single-object response -> wrap into array
      if(d && typeof d === 'object') return [d]
    }
  }catch(e){
    // fallback to mock
  }
  return fetchEstufasMock()
}
