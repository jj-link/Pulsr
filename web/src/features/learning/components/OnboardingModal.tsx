import { useClaim } from '../hooks/useClaim'
import './OnboardingModal.css'

interface OnboardingModalProps {
  isOpen: boolean
  onClose: () => void
}

export function OnboardingModal({ isOpen, onClose }: OnboardingModalProps) {
  const { status, claimResult, error, createClaim, reset } = useClaim()

  const handleClose = () => {
    reset()
    onClose()
  }

  const handleStartOnboarding = async () => {
    await createClaim()
  }

  if (!isOpen) return null

  const getStatusDisplay = () => {
    switch (status) {
      case 'idle':
      case 'creating':
        return (
          <div className="onboarding-step">
            <p>Add a new Pulsr device to your account using the claim code method.</p>
            <button className="btn-primary" onClick={handleStartOnboarding} disabled={status === 'creating'}>
              {status === 'creating' ? 'Generating...' : 'Generate Claim Code'}
            </button>
          </div>
        )
      
      case 'waiting':
        return (
          <div className="onboarding-step">
            <div className="claim-code-display">
              <span className="claim-code">{claimResult?.claimCode}</span>
            </div>
            
            <div className="instructions">
              <h4>Setup Instructions</h4>
              <ol>
                <li>Power on your Pulsr device</li>
                <li>Connect your phone to the WiFi network: <strong>Pulsr-Setup-XXXX</strong></li>
                <li>Open a browser and go to <strong>http://192.168.4.1</strong></li>
                <li>Enter your home WiFi credentials</li>
                <li>Enter the claim code: <strong>{claimResult?.claimCode}</strong></li>
                <li>Click Submit and wait for the device to connect</li>
              </ol>
            </div>
            
            <div className="status-indicator waiting">
              Waiting for device to connect...
            </div>
            
            <p className="hint">
              Once connected, your device will automatically appear in the device list.
            </p>
            
            <button className="btn-secondary" onClick={handleClose}>
              Close
            </button>
          </div>
        )
      
      case 'claimed':
        return (
          <div className="onboarding-step">
            <div className="status-indicator success">
              Device successfully connected!
            </div>
            <p>Your device has been added to your account.</p>
            <button className="btn-primary" onClick={handleClose}>
              Done
            </button>
          </div>
        )
      
      case 'expired':
        return (
          <div className="onboarding-step">
            <div className="status-indicator error">
              Claim code expired
            </div>
            <p>Please try again with a new claim code.</p>
            <button className="btn-primary" onClick={handleStartOnboarding}>
              Generate New Code
            </button>
          </div>
        )
      
      case 'failed':
        return (
          <div className="onboarding-step">
            <div className="status-indicator error">
              {error || 'Failed to create claim'}
            </div>
            <button className="btn-primary" onClick={handleStartOnboarding}>
              Try Again
            </button>
          </div>
        )
    }
  }

  return (
    <div className="modal-overlay" onClick={handleClose}>
      <div className="modal-content onboarding-modal" onClick={(e) => e.stopPropagation()}>
        <div className="modal-header">
          <h3>Add New Device</h3>
          <button className="modal-close" onClick={handleClose}>×</button>
        </div>
        
        <div className="modal-body">
          {getStatusDisplay()}
        </div>
      </div>
    </div>
  )
}
