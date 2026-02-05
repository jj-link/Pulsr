import { IDeviceRepository } from '@/core/types/repositories'
import { Device } from '@/core/types'

export class InMemoryDeviceRepository implements IDeviceRepository {
  private devices: Map<string, Device> = new Map()
  private listeners: Set<(devices: Device[]) => void> = new Set()

  async getAll(): Promise<Device[]> {
    return Array.from(this.devices.values())
  }

  async getById(id: string): Promise<Device | null> {
    return this.devices.get(id) || null
  }

  async create(device: Omit<Device, 'id'>): Promise<Device> {
    const newDevice: Device = {
      ...device,
      id: this.generateId(),
    }
    this.devices.set(newDevice.id, newDevice)
    this.notifyListeners()
    return newDevice
  }

  async update(id: string, updates: Partial<Device>): Promise<void> {
    const device = this.devices.get(id)
    if (!device) {
      throw new Error(`Device ${id} not found`)
    }
    const updated = { ...device, ...updates }
    this.devices.set(id, updated)
    this.notifyListeners()
  }

  async delete(id: string): Promise<void> {
    this.devices.delete(id)
    this.notifyListeners()
  }

  async setLearningMode(deviceId: string, isLearning: boolean): Promise<void> {
    await this.update(deviceId, { isLearning })
  }

  async clearPendingSignal(deviceId: string): Promise<void> {
    await this.update(deviceId, { pendingSignal: null })
  }

  subscribe(callback: (devices: Device[]) => void): () => void {
    this.listeners.add(callback)
    return () => {
      this.listeners.delete(callback)
    }
  }

  private notifyListeners(): void {
    this.getAll().then(devices => {
      this.listeners.forEach(callback => callback(devices))
    })
  }

  private generateId(): string {
    return `device_${Date.now()}_${Math.random().toString(36).substr(2, 9)}`
  }

  seedMockData(ownerId: string): void {
    const mockDevice: Omit<Device, 'id'> = {
      name: 'Living Room TV',
      isLearning: false,
      ownerId,
    }
    this.create(mockDevice)
  }
}
