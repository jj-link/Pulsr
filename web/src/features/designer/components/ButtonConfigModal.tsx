import { useState, useEffect } from 'react'
import { IRCommand, LayoutButton } from '@/features/core/types'
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
}

export function ButtonConfigModal({
  isOpen,
  onClose,
  onSave,
  onDelete,
  commands,
  position,
  existingButton,
}: ButtonConfigModalProps) {
  const [selectedCommandId, setSelectedCommandId] = useState('')
  const [label, setLabel] = useState('')
  const [color, setColor] = useState(COLOR_PRESETS[4])

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

  return (
    <div className="modal-overlay" onClick={onClose}>
      <div className="modal-content btn-config-modal" onClick={(e) => e.stopPropagation()}>
        <h3>{existingButton ? 'Edit Button' : 'Add Button'}</h3>
        <p className="btn-config-position">
          Row {position.row + 1}, Column {position.col + 1}
        </p>

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
        </div>

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
