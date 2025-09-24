import React, { useMemo, useRef, useState, useEffect } from 'react'
import { LineChart, Line, XAxis, YAxis, Tooltip, CartesianGrid, Legend, ResponsiveContainer, ReferenceArea } from 'recharts'
import { format } from 'date-fns'

/**
 * TimeSeriesChart - Um componente reutilizável para visualização de séries temporais
 * 
 * @param {Object} props
 * @param {Array} props.data - Array de objetos com { timestamp: Date|string, values: { [key: string]: number } }
 * @param {Array} props.series - Array de objetos definindo as séries { key: string, label: string, color: string }
 * @param {string} props.mainSeries - Chave da série principal (linha mais grossa)
 * @param {number} props.defaultWindowMs - Janela de tempo padrão em milissegundos
 * @param {boolean} props.autoFollow - Se deve seguir automaticamente os novos dados
 */
export default function TimeSeriesChart({ 
  data = [], 
  series = [], 
  mainSeries = null,
  defaultWindowMs = 10 * 60 * 1000, // 10 min
  autoFollow = true
}) {
  // Processa os dados para garantir timestamps consistentes
  const processedData = useMemo(() => {
    if (!data || !Array.isArray(data)) return []
    return data
      .filter(item => item && item.timestamp && item.values)
      .map(item => {
        const timestamp = new Date(item.timestamp).getTime()
        if (isNaN(timestamp)) return null
        return {
          ...item.values,
          tMs: timestamp
        }
      })
      .filter(Boolean)
      .sort((a, b) => a.tMs - b.tMs)
  }, [data])

  // Calcula o range total de tempo
  const timeRange = useMemo(() => ({
    min: processedData.length ? processedData[0].tMs : Date.now(),
    max: processedData.length ? processedData[processedData.length - 1].tMs : Date.now()
  }), [processedData])

  // Estados para interatividade
  const [xDomain, setXDomain] = useState(['auto', 'auto'])
  const [yDomain, setYDomain] = useState(['auto', 'auto'])
  const [follow, setFollow] = useState(autoFollow)
  const [windowMs, setWindowMs] = useState(defaultWindowMs)
  const [sliderPct, setSliderPct] = useState(1000)
  const [selectedSeries, setSelectedSeries] = useState(new Set(['avg']))
  
  // Estados para zoom/seleção
  const [selecting, setSelecting] = useState(null) // { type: 'x'|'y', start: number, current: number }
  
  // Ref para dimensões do container
  const containerRef = useRef(null)
  const [dimensions, setDimensions] = useState({ width: 0, height: 300 })

  // Observer para dimensões do container
  useEffect(() => {
    const element = containerRef.current
    if (!element) return

    const observer = new ResizeObserver(entries => {
      for (const entry of entries) {
        const { width, height } = entry.contentRect
        setDimensions({ width, height: height || 300 })
      }
    })

    observer.observe(element)
    return () => observer.disconnect()
  }, [])

  // Efeito para seguir novos dados
  useEffect(() => {
    if (!follow || processedData.length === 0) return
    const end = timeRange.max
    const start = Math.max(timeRange.min, end - windowMs)
    setXDomain([start, end])
    setSliderPct(1000)
  }, [follow, processedData, windowMs, timeRange])

  // Funções auxiliares
  const resetZoom = () => {
    setXDomain(['auto', 'auto'])
    setYDomain(['auto', 'auto'])
    setFollow(true)
  }

  const fitToVisible = () => {
    const [start, end] = xDomain[0] === 'auto' ? [timeRange.min, timeRange.max] : xDomain
    const visibleData = processedData.filter(d => d.tMs >= start && d.tMs <= end)
    if (visibleData.length === 0) return

    const values = visibleData.flatMap(d => 
      series
        .filter(s => selectedSeries.has(s.key))
        .map(s => d[s.key])
        .filter(v => v != null)
    )

    if (values.length === 0) return
    const min = Math.floor(Math.min(...values))
    const max = Math.ceil(Math.max(...values))
    setYDomain([min, max])
  }

  // Handlers de interação
  const handleMouseDown = (e) => {
    if (!e?.chartX) return
    const isYAxis = e.chartX <= 40 // área do eixo Y
    setSelecting({
      type: isYAxis ? 'y' : 'x',
      start: isYAxis ? e.chartY : e.activeLabel || e.chartX,
      current: isYAxis ? e.chartY : e.activeLabel || e.chartX
    })
  }

  const handleMouseMove = (e) => {
    if (!selecting || !e?.chartX) return
    setSelecting(prev => ({
      ...prev,
      current: selecting.type === 'y' ? e.chartY : (e.activeLabel || e.chartX)
    }))
  }

  const handleMouseUp = () => {
    if (!selecting) return

    if (selecting.type === 'y') {
      const { start, current } = selecting
      if (Math.abs(current - start) > 5) {
        const chartHeight = dimensions.height - 40 // altura menos margens
        const toValue = (pixel) => {
          const ratio = (chartHeight - pixel) / chartHeight
          return yDomain[0] === 'auto' 
            ? ratio * 100 // default range 0-100
            : yDomain[0] + ratio * (yDomain[1] - yDomain[0])
        }
        setYDomain([
          Math.min(toValue(start), toValue(current)),
          Math.max(toValue(start), toValue(current))
        ])
      }
    } else {
      const { start, current } = selecting
      if (Math.abs(current - start) > 10) {
        const min = Math.min(start, current)
        const max = Math.max(start, current)
        setXDomain([min, max])
        setFollow(false)
      }
    }

    setSelecting(null)
  }

  // Renderização do componente
  return (
    <div className="time-series-chart">
      <div className="chart-toolbar">
        <div className="left">
          <button className={`btn ${follow ? 'active' : ''}`} onClick={() => setFollow(v => !v)}>
            Tempo real
          </button>
          <button className="btn" onClick={resetZoom}>Reset</button>
          <span className="muted">Janela:</span>
          <select 
            className="btn" 
            value={windowMs} 
            onChange={e => {
              const ms = Number(e.target.value)
              setWindowMs(ms)
              if (!follow && xDomain[0] !== 'auto') {
                const start = xDomain[0]
                setXDomain([start, start + ms])
              }
            }}
          >
            <option value={60000}>1 min</option>
            <option value={600000}>10 min</option>
            <option value={1800000}>30 min</option>
            <option value={3600000}>1 hora</option>
            <option value={18000000}>5 horas</option>
            <option value={72000000}>20 horas</option>
          </select>
        </div>
        <div className="right">
          <span className="muted">Y:</span>
          <button className="btn" onClick={() => setYDomain([0, 100])}>0–100</button>
          <button className="btn" onClick={fitToVisible}>Fit</button>
          <button className="btn" onClick={() => setYDomain(['auto', 'auto'])}>Auto</button>
        </div>
      </div>

      <div ref={containerRef} style={{ width: '100%', height: 300 }}>
        <ResponsiveContainer>
          <LineChart
            data={processedData}
            margin={{ left: 40, right: 20, top: 10, bottom: 10 }}
            onMouseDown={handleMouseDown}
            onMouseMove={handleMouseMove}
            onMouseUp={handleMouseUp}
            onMouseLeave={() => setSelecting(null)}
          >
            <CartesianGrid strokeDasharray="3 3" stroke="rgba(255,255,255,0.1)" />
            <XAxis
              dataKey="tMs"
              type="number"
              domain={xDomain}
              tickFormatter={v => format(new Date(v), 'HH:mm')}
              stroke="var(--muted)"
            />
            <YAxis
              domain={yDomain}
              tickFormatter={v => `${v.toFixed(2)}%`}
              stroke="var(--muted)"
            />
            <Tooltip
              labelFormatter={v => format(new Date(v), 'yyyy-MM-dd HH:mm')}
              formatter={(v, k) => [
                `${v.toFixed(2)}%`,
                series.find(s => s.key === k)?.label || k
              ]}
              contentStyle={{
                background: 'var(--surface)',
                border: '1px solid var(--border)',
                borderRadius: '4px'
              }}
            />
            
            {/* Séries secundárias primeiro (para ficarem atrás da média) */}
            {series
              .filter(s => s.key !== 'avg')
              .map(s => (
                <Line
                  key={s.key}
                  type="monotone"
                  dataKey={s.key}
                  name={s.label || s.key}
                  stroke={s.color}
                  strokeWidth={1.5}
                  strokeDasharray="5 5"
                  dot={false}
                  isAnimationActive={false}
                  opacity={selectedSeries.has(s.key) ? 1 : 0.1}
                />
              ))}

            {/* Média por último (para ficar sempre visível por cima) */}
            {series
              .filter(s => s.key === 'avg')
              .map(s => (
                <Line
                  key={s.key}
                  type="monotone"
                  dataKey="avg"
                  name="Média"
                  stroke={s.color || 'var(--accent)'}
                  strokeWidth={2.5}
                  dot={false}
                  isAnimationActive={false}
                  opacity={selectedSeries.has('avg') ? 1 : 0.1}
                />
              ))}

            {/* Área de seleção */}
            {selecting && (
              <ReferenceArea
                {...(selecting.type === 'x'
                  ? {
                      x1: Math.min(selecting.start, selecting.current),
                      x2: Math.max(selecting.start, selecting.current)
                    }
                  : {
                      y1: Math.min(selecting.start, selecting.current),
                      y2: Math.max(selecting.start, selecting.current)
                    }
                )}
                fill="var(--accent)"
                fillOpacity={0.1}
                stroke="var(--accent)"
                strokeOpacity={0.2}
              />
            )}

            <Legend
              verticalAlign="bottom"
              height={36}
              content={({ payload = [] }) => (
                <div className="chart-legend">
                  {payload.map(entry => {
                    const serie = series.find(s => s.key === entry.dataKey) || {
                      key: entry.dataKey,
                      label: entry.dataKey,
                      color: entry.color
                    }
                    const isSelected = selectedSeries.has(serie.key)
                    return (
                      <div
                        key={serie.key}
                        className={`legend-item ${isSelected ? 'active' : ''}`}
                        onClick={() => {
                          setSelectedSeries(prev => {
                            const next = new Set(prev)
                            if (isSelected) {
                              next.delete(serie.key)
                            } else {
                              next.add(serie.key)
                            }
                            return next
                          })
                        }}
                      >
                        <span
                          className="color-mark"
                          style={{
                            backgroundColor: serie.key === mainSeries
                              ? 'var(--accent)'
                              : serie.color
                          }}
                        />
                        <span className="label">{serie.label || serie.key}</span>
                      </div>
                    )
                  })}
                </div>
              )}
            />
          </LineChart>
        </ResponsiveContainer>
      </div>

      <div className="chart-scroll">
        <input
          type="range"
          min="0"
          max="1000"
          value={sliderPct}
          onChange={e => {
            const pct = Number(e.target.value)
            setSliderPct(pct)
            const totalSpan = timeRange.max - timeRange.min
            const maxStart = Math.max(0, totalSpan - windowMs)
            const start = timeRange.min + (pct / 1000) * maxStart
            setXDomain([start, Math.min(timeRange.max, start + windowMs)])
            setFollow(false)
          }}
        />
      </div>
    </div>
  )
}
