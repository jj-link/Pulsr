interface LearningModalProps {
  isOpen: boolean
  onClose: () => void
  onComplete: () => void
  deviceName: string
}

export function LearningModal({ isOpen, onClose, deviceName }: LearningModalProps) {
  if (!isOpen) return null

  return (
    <div onClick={onClose}>
      <div onClick={(e) => e.stopPropagation()}>
        <div>
          <h3>Learning Mode</h3>
          <button onClick={onClose}>×</button>
        </div>
        
        <div>
          <div>
            <p>Waiting for IR signal...</p>
          </div>
          <p>Point your remote at {deviceName} and press a button</p>
        </div>
        
        <div>
          <button onClick={onClose}>Cancel</button>
        </div>
      </div>
    </div>
  )
}
