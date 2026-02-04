import { useState, useEffect } from 'react'
import { IRCommand } from '@/features/core/types'
import { ICommandRepository } from '@/features/core/types/repositories'

export function useCommands(repository: ICommandRepository, deviceId: string | null) {
  const [commands, setCommands] = useState<IRCommand[]>([])
  const [loading, setLoading] = useState(true)
  const [error, setError] = useState<string | null>(null)

  useEffect(() => {
    if (!deviceId) {
      setCommands([])
      setLoading(false)
      return
    }

    setLoading(true)
    const unsubscribe = repository.subscribe(deviceId, (updatedCommands) => {
      setCommands(updatedCommands)
      setLoading(false)
    })

    repository.getByDevice(deviceId)
      .then(setCommands)
      .catch((err) => setError(err.message))
      .finally(() => setLoading(false))

    return unsubscribe
  }, [repository, deviceId])

  const deleteCommand = async (commandId: string) => {
    try {
      await repository.delete(commandId)
    } catch (err) {
      setError(err instanceof Error ? err.message : 'Failed to delete command')
      throw err
    }
  }

  return {
    commands,
    loading,
    error,
    deleteCommand,
  }
}
