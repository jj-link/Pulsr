import { useState, useEffect } from 'react'
import { PendingSignal } from '@/features/core/types'

interface LearningModalProps {
  isOpen: boolean
  onClose: () => void
  onSave: (name: string) => void
  deviceName: string
  pendingSignal?: PendingSignal | null
}

export function LearningModal({ isOpen, onClose, onSave, deviceName, pendingSignal }: LearningModalProps) {
  const [commandName, setCommandName] = useState('')

  useEffect(() => {
    if (isOpen) {
      setCommandName('')
    }
  }, [isOpen])

  if (!isOpen) return null

  const handleSave = () => {
    if (commandName.trim()) {
      onSave(commandName.trim())
    }
  }

  return (
    <div onClick={onClose}>
      <div onClick={(e) => e.stopPropagation()}>
        <div>
          <h3>Learning Mode - {deviceName}</h3>
          <button onClick={onClose}>×</button>
        </div>
        
        <div>
          {pendingSignal ? (
            <div>
              <p>Signal captured!</p>
              <div>
                <label>
                  Command Name:
                  <input
                    type="text"
                    value={commandName}
                    onChange={(e) => setCommandName(e.target.value)}
                    placeholder="e.g., Power, Volume Up"
                    autoFocus
                  />
                </label>
              </div>
              <p>Protocol: {pendingSignal.protocol} | Address: {pendingSignal.address} | Command: {pendingSignal.command}</p>
            </div>
          ) : (
            <div>
              <div>
                <p>Waiting for IR signal...</p>
              </div>
              <p>Point your remote at the Pulsr receiver and press a button</p>
            </div>
          )}
        </div>
        
        <div>
          {pendingSignal ? (
            <>
              <button onClick={handleSave} disabled={!commandName.trim()}>
                Save Command
              </button>
              <button onClick={onClose}>Cancel</button>
            </>
          ) : (
            <button onClick={onClose}>Cancel</button>
          )}
        </div>
      </div>
    </div>
  )
}
