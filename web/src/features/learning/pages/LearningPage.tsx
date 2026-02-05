import { useState } from 'react'
import { useRepositories } from '@/features/core/context/RepositoryContext'
import { useDevices } from '../hooks/useDevices'
import { useCommands } from '../hooks/useCommands'
import { DeviceSelector } from '../components/DeviceSelector'
import { LearningModal } from '../components/LearningModal'
import { CommandList } from '../components/CommandList'
import { CreateDeviceModal } from '../components/CreateDeviceModal'
import './LearningPage.css'

export function LearningPage() {
  const { deviceRepository, commandRepository } = useRepositories()
  const { devices, setLearningMode, createDevice, clearPendingSignal } = useDevices(deviceRepository)
  const [selectedDeviceId, setSelectedDeviceId] = useState<string | null>(null)
  const [showLearningModal, setShowLearningModal] = useState(false)
  const [showCreateDeviceModal, setShowCreateDeviceModal] = useState(false)
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
    })
    await clearPendingSignal(selectedDeviceId)
    await setLearningMode(selectedDeviceId, false)
    setShowLearningModal(false)
  }

  const handleCreateDevice = async (name: string, deviceId: string) => {
    const device = await createDevice(name, deviceId, 'user_123')
    setSelectedDeviceId(device.id)
    setShowCreateDeviceModal(false)
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
          onCreateDevice={() => setShowCreateDeviceModal(true)}
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

      <CreateDeviceModal
        isOpen={showCreateDeviceModal}
        onClose={() => setShowCreateDeviceModal(false)}
        onCreate={handleCreateDevice}
      />
    </div>
  )
}
