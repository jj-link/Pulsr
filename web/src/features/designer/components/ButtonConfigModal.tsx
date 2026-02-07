import { useState, useEffect } from 'react'
import { IRCommand, LayoutButton, PendingSignal } from '@/features/core/types'
import './ButtonConfigModal.css'

const COLOR_PRESETS = [
  '#e74c3c', '#e67e22', '#f1c40f', '#2ecc71',
  '#3498db', '#9b59b6', '#1abc9c', '#95a5a6',
]

interface ButtonConfigModalProps {
  isOpen: boolean
  onClose: () => void
  onSave: (button: LayoutButton) => void
  onDelete?: () => void
  commands: IRCommand[]
  position: { row: number; col: number }
  existingButton?: LayoutButton | null
  // Inline learning props (Phase 1.5)
  onStartLearning?: () => void
  onStopLearning?: () => void
  onSaveNewCommand?: (name: string) => Promise<string | undefined>
  pendingSignal?: PendingSignal | null
}

export function ButtonConfigModal({
  isOpen,
  onClose,
  onSave,
  onDelete,
  commands,
  position,
  existingButton,
  onStartLearning,
  onStopLearning,
  onSaveNewCommand,
  pendingSignal,
}: ButtonConfigModalProps) {
  const [selectedCommandId, setSelectedCommandId] = useState('')
  const [label, setLabel] = useState('')
  const [color, setColor] = useState(COLOR_PRESETS[4])
  const [learningMode, setLearningMode] = useState(false)
  const [newCommandName, setNewCommandName] = useState('')
  const [savingCommand, setSavingCommand] = useState(false)

  useEffect(() => {
    if (existingButton) {
      setSelectedCommandId(existingButton.commandId)
      setLabel(existingButton.label)
      setColor(existingButton.color)
    } else {
      setSelectedCommandId('')
      setLabel('')
      setColor(COLOR_PRESETS[4])
    }
    setLearningMode(false)
    setNewCommandName('')
    setSavingCommand(false)
  }, [existingButton, isOpen])

  if (!isOpen) return null

  const handleSave = () => {
    if (!selectedCommandId) return
    const buttonLabel = label || commands.find((c) => c.id === selectedCommandId)?.name || 'Button'
    onSave({
      id: existingButton?.id || `btn_${Date.now()}`,
      commandId: selectedCommandId,
      label: buttonLabel,
      color,
      position,
    })
    onClose()
  }

  const handleCommandChange = (commandId: string) => {
    setSelectedCommandId(commandId)
    if (!label) {
      const cmd = commands.find((c) => c.id === commandId)
      if (cmd) setLabel(cmd.name)
    }
  }

  const handleStartLearning = () => {
    setLearningMode(true)
    onStartLearning?.()
  }

  const handleCancelLearning = () => {
    setLearningMode(false)
    setNewCommandName('')
    onStopLearning?.()
  }

  const handleSaveNewCommand = async () => {
    if (!newCommandName.trim() || !onSaveNewCommand) return
    setSavingCommand(true)
    const commandId = await onSaveNewCommand(newCommandName.trim())
    setSavingCommand(false)
    if (commandId) {
      setSelectedCommandId(commandId)
      if (!label) setLabel(newCommandName.trim())
      setLearningMode(false)
      setNewCommandName('')
    }
  }

  const canLearn = !!onStartLearning && !!onSaveNewCommand

  return (
    <div className="modal-overlay" onClick={onClose}>
      <div className="modal-content btn-config-modal" onClick={(e) => e.stopPropagation()}>
        <h3>{existingButton ? 'Edit Button' : 'Add Button'}</h3>
        <p className="btn-config-position">
          Row {position.row + 1}, Column {position.col + 1}
        </p>

        {learningMode ? (
          <div className="btn-config-learning">
            {pendingSignal ? (
              <div className="btn-config-learning-captured">
                <div className="btn-config-learning-status success">Signal captured!</div>
                <p className="btn-config-learning-info">
                  {pendingSignal.protocol} | Address: {pendingSignal.address} | Command: {pendingSignal.command}
                </p>
                <div className="btn-config-field">
                  <label htmlFor="new-command-name">Command Name</label>
                  <input
                    id="new-command-name"
                    type="text"
                    value={newCommandName}
                    onChange={(e) => setNewCommandName(e.target.value)}
                    placeholder="e.g., Power, Volume Up"
                    autoFocus
                  />
                </div>
                <div className="btn-config-learning-actions">
                  <button
                    className="btn-config-save"
                    onClick={handleSaveNewCommand}
                    disabled={!newCommandName.trim() || savingCommand}
                    type="button"
                  >
                    {savingCommand ? 'Saving...' : 'Save Command'}
                  </button>
                  <button
                    className="btn-config-cancel"
                    onClick={handleCancelLearning}
                    type="button"
                  >
                    Cancel
                  </button>
                </div>
              </div>
            ) : (
              <div className="btn-config-learning-waiting">
                <div className="btn-config-learning-status waiting">
                  <span className="btn-config-learning-pulse"></span>
                  Waiting for IR signal...
                </div>
                <p className="btn-config-learning-hint">Point your remote at the Pulsr receiver and press a button</p>
                <button
                  className="btn-config-cancel"
                  onClick={handleCancelLearning}
                  type="button"
                >
                  Cancel
                </button>
              </div>
            )}
          </div>
        ) : (
          <div className="btn-config-field">
            <label htmlFor="command-select">Command</label>
            <select
              id="command-select"
              value={selectedCommandId}
              onChange={(e) => handleCommandChange(e.target.value)}
            >
              <option value="">Select a command...</option>
              {commands.map((cmd) => (
                <option key={cmd.id} value={cmd.id}>
                  {cmd.name} ({cmd.protocol})
                </option>
              ))}
            </select>
            {canLearn && (
              <button
                className="btn-config-learn-new"
                onClick={handleStartLearning}
                type="button"
              >
                + Learn New Command
              </button>
            )}
          </div>
        )}

        <div className="btn-config-field">
          <label htmlFor="label-input">Label</label>
          <input
            id="label-input"
            type="text"
            value={label}
            onChange={(e) => setLabel(e.target.value)}
            placeholder="Button label"
          />
        </div>

        <div className="btn-config-field">
          <label>Color</label>
          <div className="btn-config-colors">
            {COLOR_PRESETS.map((c) => (
              <button
                key={c}
                className={`btn-config-color-swatch ${color === c ? 'selected' : ''}`}
                style={{ backgroundColor: c }}
                onClick={() => setColor(c)}
                type="button"
                aria-label={`Color ${c}`}
              />
            ))}
          </div>
        </div>

        <div className="btn-config-preview">
          <label>Preview</label>
          <div
            className="btn-config-preview-cell"
            style={{ backgroundColor: color }}
          >
            {label || 'Button'}
          </div>
        </div>

        <div className="btn-config-actions">
          {existingButton && onDelete && (
            <button
              className="btn-config-delete"
              onClick={() => { onDelete(); onClose() }}
              type="button"
            >
              Delete
            </button>
          )}
          <div className="btn-config-actions-right">
            <button className="btn-config-cancel" onClick={onClose} type="button">
              Cancel
            </button>
            <button
              className="btn-config-save"
              onClick={handleSave}
              disabled={!selectedCommandId}
              type="button"
            >
              {existingButton ? 'Update' : 'Add'}
            </button>
          </div>
        </div>
      </div>
    </div>
  )
}
