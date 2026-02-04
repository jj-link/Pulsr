import { IQueueRepository } from '@/core/types/repositories'
import { QueueItem } from '@/core/types'

export class InMemoryQueueRepository implements IQueueRepository {
  private queue: Map<string, QueueItem> = new Map()
  private listeners: Map<string, Set<(queue: QueueItem[]) => void>> = new Map()

  async getByDevice(deviceId: string): Promise<QueueItem[]> {
    return Array.from(this.queue.values())
      .filter(item => item.deviceId === deviceId)
      .sort((a, b) => a.createdAt.getTime() - b.createdAt.getTime())
  }

  async enqueue(deviceId: string, commandId: string): Promise<QueueItem> {
    const newItem: QueueItem = {
      id: this.generateId(),
      deviceId,
      commandId,
      status: 'pending',
      createdAt: new Date(),
    }
    this.queue.set(newItem.id, newItem)
    this.notifyListeners(deviceId)
    
    setTimeout(() => {
      this.simulateProcessing(newItem.id)
    }, 1000)
    
    return newItem
  }

  async updateStatus(id: string, status: QueueItem['status'], error?: string): Promise<void> {
    const item = this.queue.get(id)
    if (!item) {
      throw new Error(`Queue item ${id} not found`)
    }
    const updated: QueueItem = {
      ...item,
      status,
      processedAt: status === 'completed' || status === 'failed' ? new Date() : item.processedAt,
      error,
    }
    this.queue.set(id, updated)
    this.notifyListeners(item.deviceId)
  }

  subscribe(deviceId: string, callback: (queue: QueueItem[]) => void): () => void {
    if (!this.listeners.has(deviceId)) {
      this.listeners.set(deviceId, new Set())
    }
    this.listeners.get(deviceId)!.add(callback)

    return () => {
      const deviceListeners = this.listeners.get(deviceId)
      if (deviceListeners) {
        deviceListeners.delete(callback)
        if (deviceListeners.size === 0) {
          this.listeners.delete(deviceId)
        }
      }
    }
  }

  private notifyListeners(deviceId: string): void {
    const deviceListeners = this.listeners.get(deviceId)
    if (deviceListeners) {
      this.getByDevice(deviceId).then(queue => {
        deviceListeners.forEach(callback => callback(queue))
      })
    }
  }

  private generateId(): string {
    return `queue_${Date.now()}_${Math.random().toString(36).substr(2, 9)}`
  }

  private async simulateProcessing(id: string): Promise<void> {
    await this.updateStatus(id, 'processing')
    setTimeout(async () => {
      const success = Math.random() > 0.1
      if (success) {
        await this.updateStatus(id, 'completed')
      } else {
        await this.updateStatus(id, 'failed', 'Simulated transmission error')
      }
    }, 2000)
  }
}
