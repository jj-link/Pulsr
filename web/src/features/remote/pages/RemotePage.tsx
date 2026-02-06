import { useState, useEffect } from 'react'
import { useParams, Link } from 'react-router-dom'
import { useRepositories } from '@/features/core/context/RepositoryContext'
import { useCommands } from '@/features/learning/hooks/useCommands'
import { useQueue } from '../hooks/useQueue'
import { DeviceLayout } from '@/features/core/types'
import './RemotePage.css'

export function RemotePage() {
  const { deviceId } = useParams()
  const { commandRepository, queueRepository, layoutRepository } = useRepositories()
  const { commands } = useCommands(commandRepository, deviceId ?? null)
  const { queue, enqueue } = useQueue(queueRepository, deviceId ?? null)
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
    setSendingId(commandId)
    try {
      await enqueue(commandId)
    } finally {
      setTimeout(() => setSendingId(null), 500)
    }
  }

  const recentQueue = queue.slice(0, 5)
  const hasLayout = layout && layout.buttons.length > 0

  const getCommandName = (commandId: string): string => {
    const cmd = commands.find((c) => c.id === commandId)
    return cmd?.name || commandId.split('/').pop() || 'Unknown'
  }

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

      {recentQueue.length > 0 && (
        <div className="remote-queue-status">
          <h4>Recent</h4>
          {recentQueue.map((item) => (
            <div key={item.id} className="test-queue-item">
              <span className="test-queue-command">{getCommandName(item.commandId)}</span>
              <span className={`test-queue-badge ${item.status}`}>{item.status}</span>
            </div>
          ))}
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
