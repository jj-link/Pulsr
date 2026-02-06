import { ILayoutRepository } from '@/core/types/repositories'
import { DeviceLayout } from '@/core/types'

export class InMemoryLayoutRepository implements ILayoutRepository {
  private layouts: Map<string, DeviceLayout> = new Map()
  private listeners: Map<string, Set<(layout: DeviceLayout | null) => void>> = new Map()

  async getByDevice(deviceId: string): Promise<DeviceLayout | null> {
    return this.layouts.get(deviceId) || null
  }

  async save(deviceId: string, layout: DeviceLayout): Promise<void> {
    this.layouts.set(deviceId, layout)
    this.notifyListeners(deviceId)
  }

  subscribe(deviceId: string, callback: (layout: DeviceLayout | null) => void): () => void {
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
      this.getByDevice(deviceId).then(layout => {
        deviceListeners.forEach(callback => callback(layout))
      })
    }
  }

  seedMockData(deviceId: string, commandIds: string[]): void {
    if (commandIds.length < 3) return

    const mockLayout: DeviceLayout = {
      gridSize: { rows: 4, cols: 3 },
      buttons: [
        {
          id: 'btn_1',
          commandId: commandIds[0],
          label: 'Power',
          color: '#e74c3c',
          position: { row: 0, col: 1 },
        },
        {
          id: 'btn_2',
          commandId: commandIds[1],
          label: 'Vol+',
          color: '#3498db',
          position: { row: 1, col: 2 },
        },
        {
          id: 'btn_3',
          commandId: commandIds[2],
          label: 'Vol-',
          color: '#3498db',
          position: { row: 2, col: 2 },
        },
      ],
    }
    this.save(deviceId, mockLayout)
  }
}
