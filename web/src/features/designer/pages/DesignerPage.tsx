import { useState } from 'react'
import { useRepositories } from '@/features/core/context/RepositoryContext'
import { useDevices } from '@/features/learning/hooks/useDevices'
import { useCommands } from '@/features/learning/hooks/useCommands'
import { useLayout } from '../hooks/useLayout'
import { DeviceSelector } from '@/features/learning/components/DeviceSelector'
import { ButtonConfigModal } from '../components/ButtonConfigModal'
import { LayoutButton } from '@/features/core/types'
import './DesignerPage.css'

export function DesignerPage() {
  const { deviceRepository, commandRepository } = useRepositories()
  const { devices } = useDevices(deviceRepository)
  const [selectedDeviceId, setSelectedDeviceId] = useState<string | null>(null)
  const { commands } = useCommands(commandRepository, selectedDeviceId)
  const {
    layout,
    loading,
    isDirty,
    error,
    addButton,
    updateButton,
    removeButton,
    changeGridSize,
    save,
    reset,
  } = useLayout(selectedDeviceId ?? undefined)

  const [saveStatus, setSaveStatus] = useState<'idle' | 'saving' | 'saved'>('idle')
  const [modalOpen, setModalOpen] = useState(false)
  const [editingPosition, setEditingPosition] = useState<{ row: number; col: number }>({ row: 0, col: 0 })
  const [editingButton, setEditingButton] = useState<LayoutButton | null>(null)

  const handleCellClick = (row: number, col: number) => {
    const existing = layout?.buttons.find(
      (btn) => btn.position.row === row && btn.position.col === col
    )
    setEditingPosition({ row, col })
    setEditingButton(existing || null)
    setModalOpen(true)
  }

  const handleSave = async () => {
    setSaveStatus('saving')
    await save()
    setSaveStatus('saved')
    setTimeout(() => setSaveStatus('idle'), 2000)
  }

  const handleSaveButton = (button: LayoutButton) => {
    if (editingButton) {
      updateButton(editingButton.id, button)
    } else {
      addButton(button)
    }
  }

  const handleDeleteButton = () => {
    if (editingButton) {
      removeButton(editingButton.id)
    }
  }

  const getButtonAt = (row: number, col: number): LayoutButton | undefined => {
    return layout?.buttons.find(
      (btn) => btn.position.row === row && btn.position.col === col
    )
  }

  return (
    <div className="designer-page">
      <div className="page-header">
        <h2>Layout Designer</h2>
        <p>Click cells to assign commands and create your remote layout.</p>
      </div>

      <div className="designer-device-section">
        <DeviceSelector
          devices={devices}
          selectedDeviceId={selectedDeviceId}
          onSelectDevice={setSelectedDeviceId}
        />
      </div>

      {selectedDeviceId && layout && !loading && (
        <>
          <div className="designer-toolbar">
            <div className="designer-grid-size">
              <label>Grid Size:</label>
              <select
                value={`${layout.gridSize.rows}x${layout.gridSize.cols}`}
                onChange={(e) => {
                  const [rows, cols] = e.target.value.split('x').map(Number)
                  changeGridSize(rows, cols)
                }}
              >
                <option value="3x3">3 × 3</option>
                <option value="4x3">4 × 3</option>
                <option value="5x3">5 × 3</option>
                <option value="6x3">6 × 3</option>
                <option value="4x4">4 × 4</option>
                <option value="5x4">5 × 4</option>
                <option value="6x4">6 × 4</option>
              </select>
            </div>
            <div className="designer-toolbar-actions">
              {isDirty ? (
                <>
                  <button className="designer-btn-cancel" onClick={reset} type="button">
                    Cancel
                  </button>
                  <button
                    className="designer-btn-save"
                    onClick={handleSave}
                    disabled={saveStatus === 'saving'}
                    type="button"
                  >
                    {saveStatus === 'saving' ? 'Saving...' : 'Save Layout'}
                  </button>
                </>
              ) : saveStatus === 'saved' ? (
                <span className="designer-saved-badge">✓ Saved</span>
              ) : null}
            </div>
          </div>

          {error && <div className="designer-error">{error}</div>}

          <div
            className="designer-grid"
            style={{
              gridTemplateColumns: `repeat(${layout.gridSize.cols}, 1fr)`,
              gridTemplateRows: `repeat(${layout.gridSize.rows}, 1fr)`,
            }}
          >
            {Array.from({ length: layout.gridSize.rows }).map((_, row) =>
              Array.from({ length: layout.gridSize.cols }).map((_, col) => {
                const button = getButtonAt(row, col)
                return (
                  <button
                    key={`${row}-${col}`}
                    className={`designer-cell ${button ? 'filled' : 'empty'}`}
                    style={button ? { backgroundColor: button.color } : undefined}
                    onClick={() => handleCellClick(row, col)}
                    type="button"
                  >
                    {button ? (
                      <span className="designer-cell-label">{button.label}</span>
                    ) : (
                      <span className="designer-cell-plus">+</span>
                    )}
                  </button>
                )
              })
            )}
          </div>

          {commands.length === 0 && (
            <p className="designer-no-commands">
              No commands learned yet. <a href="/learn">Go to Learn</a> to capture IR commands first.
            </p>
          )}

          <ButtonConfigModal
            isOpen={modalOpen}
            onClose={() => setModalOpen(false)}
            onSave={handleSaveButton}
            onDelete={editingButton ? handleDeleteButton : undefined}
            commands={commands}
            position={editingPosition}
            existingButton={editingButton}
          />
        </>
      )}

      {selectedDeviceId && loading && (
        <p className="designer-loading">Loading layout...</p>
      )}
    </div>
  )
}
