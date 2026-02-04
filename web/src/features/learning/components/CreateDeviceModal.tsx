import { useState } from 'react'
import './CreateDeviceModal.css'

interface CreateDeviceModalProps {
  isOpen: boolean
  onClose: () => void
  onCreate: (name: string) => void
}

export function CreateDeviceModal({ isOpen, onClose, onCreate }: CreateDeviceModalProps) {
  const [deviceName, setDeviceName] = useState('')

  if (!isOpen) return null

  const handleSubmit = (e: React.FormEvent) => {
    e.preventDefault()
    if (deviceName.trim()) {
      onCreate(deviceName.trim())
      setDeviceName('')
    }
  }

  const handleClose = () => {
    setDeviceName('')
    onClose()
  }

  return (
    <div className="modal-overlay" onClick={handleClose}>
      <div className="modal-content" onClick={(e) => e.stopPropagation()}>
        <div className="modal-header">
          <h3>Create New Device</h3>
          <button className="modal-close" onClick={handleClose}>×</button>
        </div>
        
        <form onSubmit={handleSubmit}>
          <div className="modal-body">
            <label htmlFor="device-name">
              Device Name
              <input
                id="device-name"
                type="text"
                value={deviceName}
                onChange={(e) => setDeviceName(e.target.value)}
                placeholder="e.g., Living Room TV"
                autoFocus
              />
            </label>
          </div>
          
          <div className="modal-footer">
            <button type="button" onClick={handleClose} className="btn-cancel">
              Cancel
            </button>
            <button type="submit" className="btn-primary">
              Create
            </button>
          </div>
        </form>
      </div>
    </div>
  )
}
