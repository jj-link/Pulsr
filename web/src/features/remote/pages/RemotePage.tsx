import { useState, useEffect, type CSSProperties } from 'react'
import { useParams, Link } from 'react-router-dom'
import { useRepositories } from '@/features/core/context/RepositoryContext'
import { useCommands } from '@/features/learning/hooks/useCommands'
import { useRealtimeDb } from '@/features/core/firebase'
import { ref, set } from 'firebase/database'
import { DeviceLayout } from '@/features/core/types'
import {
  ArrowDown,
  ArrowLeft,
  ArrowRight,
  ArrowUp,
  AudioLines,
  Circle,
  FastForward,
  Gauge,
  Grid2x2,
  Home,
  Info,
  Mic,
  Pause,
  Play,
  Power,
  Rewind,
  Settings,
  Square,
  SkipBack,
  SkipForward,
  Volume1,
  Volume2,
  VolumeX,
  type LucideIcon,
} from 'lucide-react'
import './RemotePage.css'

type ButtonIntent =
  | 'power'
  | 'volume-up'
  | 'volume-down'
  | 'mute'
  | 'channel-up'
  | 'channel-down'
  | 'direction-up'
  | 'direction-down'
  | 'direction-left'
  | 'direction-right'
  | 'ok'
  | 'home'
  | 'menu'
  | 'back'
  | 'input'
  | 'play'
  | 'pause'
  | 'stop'
  | 'rewind'
  | 'fast-forward'
  | 'next'
  | 'previous'
  | 'record'
  | 'settings'
  | 'info'
  | 'voice'
  | 'number'
  | 'default'

interface ButtonVisual {
  intent: ButtonIntent
  icon: LucideIcon | null
  primaryLabel: string
  secondaryLabel: string
}

function normalizeLabel(value: string): string {
  return value.trim().toLowerCase()
}

function inferIntent(label: string): ButtonIntent {
  const normalized = normalizeLabel(label)

  if (/^\d$/.test(normalized)) return 'number'
  if (/^ok$|^select$|^enter$|^confirm$/.test(normalized)) return 'ok'
  if (/power|standby|on\/off|off\/on/.test(normalized)) return 'power'
  if (/mute|silence/.test(normalized)) return 'mute'
  if (/^vol\+?$|volume\s*up|\bvol up\b/.test(normalized)) return 'volume-up'
  if (/^vol-$|volume\s*down|\bvol down\b/.test(normalized)) return 'volume-down'
  if (/^ch\+?$|channel\s*up|\bch up\b/.test(normalized)) return 'channel-up'
  if (/^ch-$|channel\s*down|\bch down\b/.test(normalized)) return 'channel-down'
  if (/^up$|arrow up|dpad up|nav up/.test(normalized)) return 'direction-up'
  if (/^down$|arrow down|dpad down|nav down/.test(normalized)) return 'direction-down'
  if (/^left$|arrow left|dpad left|nav left/.test(normalized)) return 'direction-left'
  if (/^right$|arrow right|dpad right|nav right/.test(normalized)) return 'direction-right'
  if (/home/.test(normalized)) return 'home'
  if (/menu/.test(normalized)) return 'menu'
  if (/back|return/.test(normalized)) return 'back'
  if (/source|input|hdmi|tv/.test(normalized)) return 'input'
  if (/^play$/.test(normalized)) return 'play'
  if (/pause/.test(normalized)) return 'pause'
  if (/stop/.test(normalized)) return 'stop'
  if (/rewind|\brw\b/.test(normalized)) return 'rewind'
  if (/fast forward|\bff\b/.test(normalized)) return 'fast-forward'
  if (/next|skip forward/.test(normalized)) return 'next'
  if (/prev|previous|skip back/.test(normalized)) return 'previous'
  if (/record|\brec\b/.test(normalized)) return 'record'
  if (/setting|gear/.test(normalized)) return 'settings'
  if (/info|guide/.test(normalized)) return 'info'
  if (/voice|assistant|mic/.test(normalized)) return 'voice'

  return 'default'
}

function getButtonVisual(label: string): ButtonVisual {
  const intent = inferIntent(label)

  if (intent === 'number') {
    return {
      intent,
      icon: null,
      primaryLabel: label,
      secondaryLabel: '',
    }
  }

  const iconByIntent: Record<Exclude<ButtonIntent, 'number' | 'default'>, LucideIcon> = {
    power: Power,
    'volume-up': Volume2,
    'volume-down': Volume1,
    mute: VolumeX,
    'channel-up': ArrowUp,
    'channel-down': ArrowDown,
    'direction-up': ArrowUp,
    'direction-down': ArrowDown,
    'direction-left': ArrowLeft,
    'direction-right': ArrowRight,
    ok: Gauge,
    home: Home,
    menu: Grid2x2,
    back: ArrowLeft,
    input: AudioLines,
    play: Play,
    pause: Pause,
    stop: Square,
    rewind: Rewind,
    'fast-forward': FastForward,
    next: SkipForward,
    previous: SkipBack,
    record: Circle,
    settings: Settings,
    info: Info,
    voice: Mic,
  }

  if (intent === 'default') {
    return {
      intent,
      icon: null,
      primaryLabel: label.length > 10 ? `${label.slice(0, 10)}...` : label,
      secondaryLabel: '',
    }
  }

  return {
    intent,
    icon: iconByIntent[intent],
    primaryLabel: '',
    secondaryLabel: label,
  }
}

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
          <p>
            Use the <strong>Remote</strong> menu in the top bar to choose a
            device.
          </p>
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

  const hasLayout = layout !== null && layout.buttons.length > 0

  return (
    <div className="remote-page">
      {layoutLoading ? (
        <p className="remote-loading">Loading remote...</p>
      ) : hasLayout && layout ? (
        <div className="remote-shell">
          <div className="remote-shell-top">
            <span className="remote-shell-pill" />
          </div>

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
                const visual = getButtonVisual(btn.label)
                const Icon = visual.icon

                return (
                  <button
                    key={`${row}-${col}`}
                    className={`remote-btn intent-${visual.intent} ${isSending ? 'sending' : ''}`}
                    style={{ '--btn-color': btn.color } as CSSProperties}
                    onClick={() => handleSend(btn.commandId)}
                    disabled={isSending}
                    type="button"
                    aria-label={btn.label}
                  >
                    <span className="remote-btn-content">
                      {Icon ? <Icon className="remote-btn-icon" strokeWidth={2.3} /> : null}
                      {visual.primaryLabel ? (
                        <span className="remote-btn-primary">{visual.primaryLabel}</span>
                      ) : null}
                      {visual.secondaryLabel ? (
                        <span className="remote-btn-label">{visual.secondaryLabel}</span>
                      ) : null}
                    </span>
                  </button>
                )
              })
            )}
          </div>

        </div>
      ) : (
        <div className="remote-empty-state">
          <h2>No Remote Layout</h2>
          <p>
            This device does not have a remote layout yet. Go to{' '}
            <Link to="/designer">Designer</Link> to set up your remote.
          </p>
        </div>
      )}

      <div className="test-panel">
        <button
          className="test-panel-toggle"
          onClick={() => setTestPanelOpen(!testPanelOpen)}
          type="button"
        >
          {testPanelOpen ? 'Hide' : 'Show'} test transmit panel
        </button>

        {testPanelOpen && (
          <div className="test-panel-content">
            {commands.length === 0 ? (
              <p className="test-panel-empty">
                No learned commands. Go to <Link to="/learn">Learn</Link> to
                teach commands first.
              </p>
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
                      type="button"
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
