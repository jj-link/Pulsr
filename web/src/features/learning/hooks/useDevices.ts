import { useState, useEffect } from 'react'
import { Device } from '@/features/core/types'
import { IDeviceRepository } from '@/features/core/types/repositories'

export function useDevices(repository: IDeviceRepository) {
  const [devices, setDevices] = useState<Device[]>([])
  const [loading, setLoading] = useState(true)
  const [error, setError] = useState<string | null>(null)

  useEffect(() => {
    const unsubscribe = repository.subscribe((updatedDevices) => {
      setDevices(updatedDevices)
      setLoading(false)
    })

    repository.getAll()
      .then(setDevices)
      .catch((err) => setError(err.message))
      .finally(() => setLoading(false))

    return unsubscribe
  }, [repository])

  const createDevice = async (name: string, deviceId: string, ownerId: string) => {
    try {
      const device = await repository.create({ id: deviceId, name, isLearning: false, ownerId })
      return device
    } catch (err) {
      setError(err instanceof Error ? err.message : 'Failed to create device')
      throw err
    }
  }

  const setLearningMode = async (deviceId: string, isLearning: boolean) => {
    try {
      await repository.setLearningMode(deviceId, isLearning)
    } catch (err) {
      setError(err instanceof Error ? err.message : 'Failed to set learning mode')
      throw err
    }
  }

  const deleteDevice = async (deviceId: string) => {
    try {
      await repository.delete(deviceId)
    } catch (err) {
      setError(err instanceof Error ? err.message : 'Failed to delete device')
      throw err
    }
  }

  const clearPendingSignal = async (deviceId: string) => {
    try {
      await repository.clearPendingSignal(deviceId)
    } catch (err) {
      setError(err instanceof Error ? err.message : 'Failed to clear pending signal')
      throw err
    }
  }

  return {
    devices,
    loading,
    error,
    createDevice,
    setLearningMode,
    deleteDevice,
    clearPendingSignal,
  }
}
