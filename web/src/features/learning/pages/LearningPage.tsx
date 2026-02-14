import { useState } from 'react'
import { useRepositories } from '@/features/core/context/RepositoryContext'
import { useDevices } from '../hooks/useDevices'
import { useCommands } from '../hooks/useCommands'
import { DeviceSelector } from '../components/DeviceSelector'
import { LearningModal } from '../components/LearningModal'
import { CommandList } from '../components/CommandList'
import { OnboardingModal } from '../components/OnboardingModal'
import './LearningPage.css'

export function LearningPage() {
  const { deviceRepository, commandRepository } = useRepositories()
  const { devices, setLearningMode, clearPendingSignal } = useDevices(deviceRepository)
  const [selectedDeviceId, setSelectedDeviceId] = useState<string | null>(null)
  const [showLearningModal, setShowLearningModal] = useState(false)
  const [showOnboardingModal, setShowOnboardingModal] = useState(false)
  const { commands, deleteCommand, updateCommand } = useCommands(commandRepository, selectedDeviceId)

  const selectedDevice = devices.find((d) => d.id === selectedDeviceId)

  const handleStartLearning = async () => {
    if (!selectedDeviceId) return
    await setLearningMode(selectedDeviceId, true)
    setShowLearningModal(true)
  }

  const handleStopLearning = async () => {
    if (!selectedDeviceId) return
    await clearPendingSignal(selectedDeviceId)
    await setLearningMode(selectedDeviceId, false)
    setShowLearningModal(false)
  }

  const handleSaveCommand = async (name: string) => {
    if (!selectedDeviceId || !selectedDevice?.pendingSignal) return
    const signal = selectedDevice.pendingSignal
    await commandRepository.create({
      deviceId: selectedDeviceId,
      name,
      protocol: signal.protocol,
      address: signal.address,
      command: signal.command,
      value: signal.value,
      bits: signal.bits,
    })
    await clearPendingSignal(selectedDeviceId)
    await setLearningMode(selectedDeviceId, false)
    setShowLearningModal(false)
  }

  return (
    <div className="learning-page">
      <div className="page-header">
        <h2>Device List</h2>
        <p>Manage your IR devices and learn commands from remote controls.</p>
      </div>

      <div className="device-section">
        <DeviceSelector
          devices={devices}
          selectedDeviceId={selectedDeviceId}
          onSelectDevice={setSelectedDeviceId}
          onCreateDevice={() => setShowOnboardingModal(true)}
        />

        {selectedDevice && (
          <div className="learning-controls">
            <button
              onClick={handleStartLearning}
              className="btn-learning"
              disabled={selectedDevice.isLearning}
            >
              {selectedDevice.isLearning ? 'Learning Mode Active...' : 'Start Learning'}
            </button>
            {selectedDevice.isLearning && (
              <button onClick={handleStopLearning} className="btn-stop-learning">
                Stop Learning
              </button>
            )}
          </div>
        )}
      </div>

      {selectedDeviceId && (
        <CommandList commands={commands} onDelete={deleteCommand} onEdit={updateCommand} />
      )}

      <LearningModal
        isOpen={showLearningModal}
        onClose={handleStopLearning}
        onSave={handleSaveCommand}
        deviceName={selectedDevice?.name || ''}
        pendingSignal={selectedDevice?.pendingSignal}
      />

      <OnboardingModal
        isOpen={showOnboardingModal}
        onClose={() => setShowOnboardingModal(false)}
      />
    </div>
  )
}
