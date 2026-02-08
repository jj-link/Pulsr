import { useState, useEffect } from 'react'
import { useParams, Link } from 'react-router-dom'
import { useRepositories } from '@/features/core/context/RepositoryContext'
import { useCommands } from '@/features/learning/hooks/useCommands'
import { useRealtimeDb } from '@/features/core/firebase'
import { ref, set } from 'firebase/database'
import { DeviceLayout } from '@/features/core/types'
import './RemotePage.css'

export function RemotePage() {
  const { deviceId } = useParams()
  const { commandRepository, layoutRepository } = useRepositories()
  const { commands } = useCommands(commandRepository, deviceId ?? null)
  const rtdb = useRealtimeDb()
  const [testPanelOpen, setTestPanelOpen] = useState(true)
  const [sendingId, setSendingId] = useState<string | null>(null)
  const [layout, setLayout] = useState<DeviceLayout | null>(null)
  const [layoutLoading, setLayoutLoading] = useState(true)

  useEffect(() => {
    if (!deviceId) {
      setLayout(null)
      setLayoutLoading(false)
      return
    }
    setLayoutLoading(true)
    const unsubscribe = layoutRepository.subscribe(deviceId, (l) => {
      setLayout(l)
      setLayoutLoading(false)
    })
    return unsubscribe
  }, [deviceId, layoutRepository])

  if (!deviceId) {
    return (
      <div className="remote-page">
        <div className="remote-empty-state">
          <h2>Select a Device</h2>
          <p>Use the <strong>Remote ▼</strong> dropdown in the navigation bar to choose a device.</p>
        </div>
      </div>
    )
  }

  const handleSend = async (commandId: string) => {
    if (!deviceId || !rtdb) return
    const cmd = commands.find((c) => c.id === commandId)
    if (!cmd) return

    setSendingId(commandId)
    try {
      await set(ref(rtdb, `devices/${deviceId}/pendingCommand`), {
        protocol: cmd.protocol,
        value: cmd.value,
        bits: cmd.bits,
        timestamp: Date.now(),
      })
    } finally {
      setTimeout(() => setSendingId(null), 500)
    }
  }
  const hasLayout = layout && layout.buttons.length > 0

  return (
    <div className="remote-page">
      {layoutLoading ? (
        <p className="remote-loading">Loading remote...</p>
      ) : hasLayout ? (
        <div
          className="remote-grid"
          style={{
            gridTemplateColumns: `repeat(${layout.gridSize.cols}, 1fr)`,
            gridTemplateRows: `repeat(${layout.gridSize.rows}, 1fr)`,
          }}
        >
          {Array.from({ length: layout.gridSize.rows }).map((_, row) =>
            Array.from({ length: layout.gridSize.cols }).map((_, col) => {
              const btn = layout.buttons.find(
                (b) => b.position.row === row && b.position.col === col
              )
              if (!btn) {
                return <div key={`${row}-${col}`} className="remote-cell-empty" />
              }
              const isSending = sendingId === btn.commandId
              return (
                <button
                  key={`${row}-${col}`}
                  className={`remote-btn ${isSending ? 'sending' : ''}`}
                  style={{ backgroundColor: btn.color }}
                  onClick={() => handleSend(btn.commandId)}
                  disabled={isSending}
                  type="button"
                >
                  <span className="remote-btn-label">{btn.label}</span>
                </button>
              )
            })
          )}
        </div>
      ) : (
        <div className="remote-empty-state">
          <h2>No Remote Layout</h2>
          <p>This device doesn't have a remote layout yet. Go to <Link to="/designer">Designer</Link> to set up your remote.</p>
        </div>
      )}

      <div className="test-panel">
        <button
          className="test-panel-toggle"
          onClick={() => setTestPanelOpen(!testPanelOpen)}
        >
          {testPanelOpen ? '▼' : '▶'} Test Transmit
        </button>

        {testPanelOpen && (
          <div className="test-panel-content">
            {commands.length === 0 ? (
              <p className="test-panel-empty">No learned commands. Go to <Link to="/learn">Learn</Link> to teach commands first.</p>
            ) : (
              <div className="test-command-list">
                {commands.map((cmd) => (
                  <div key={cmd.id} className="test-command-row">
                    <div className="test-command-info">
                      <span className="test-command-name">{cmd.name}</span>
                      <span className="test-command-protocol">{cmd.protocol}</span>
                    </div>
                    <button
                      className={`test-send-btn ${sendingId === cmd.id ? 'sending' : ''}`}
                      onClick={() => handleSend(cmd.id)}
                      disabled={sendingId === cmd.id}
                    >
                      {sendingId === cmd.id ? 'Sending...' : 'Send'}
                    </button>
                  </div>
                ))}
              </div>
            )}
          </div>
        )}
      </div>
    </div>
  )
}
