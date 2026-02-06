import { useState, useEffect, useCallback } from 'react'
import { IQueueRepository } from '@/features/core/types/repositories'
import { QueueItem } from '@/features/core/types'

export function useQueue(queueRepository: IQueueRepository, deviceId: string | null) {
  const [queue, setQueue] = useState<QueueItem[]>([])
  const [loading, setLoading] = useState(false)

  useEffect(() => {
    if (!deviceId) {
      setQueue([])
      return
    }

    setLoading(true)
    const unsubscribe = queueRepository.subscribe(deviceId, (items) => {
      setQueue(items)
      setLoading(false)
    })

    return unsubscribe
  }, [deviceId, queueRepository])

  const enqueue = useCallback(async (commandId: string) => {
    if (!deviceId) return
    await queueRepository.enqueue(deviceId, commandId)
  }, [deviceId, queueRepository])

  return { queue, loading, enqueue }
}
