import { ICommandRepository } from '@/core/types/repositories'
import { IRCommand } from '@/core/types'

export class InMemoryCommandRepository implements ICommandRepository {
  private commands: Map<string, IRCommand> = new Map()
  private listeners: Map<string, Set<(commands: IRCommand[]) => void>> = new Map()

  async getByDevice(deviceId: string): Promise<IRCommand[]> {
    return Array.from(this.commands.values())
      .filter(cmd => cmd.deviceId === deviceId)
      .sort((a, b) => b.capturedAt.getTime() - a.capturedAt.getTime())
  }

  async getById(id: string): Promise<IRCommand | null> {
    return this.commands.get(id) || null
  }

  async create(command: Omit<IRCommand, 'id' | 'capturedAt'>): Promise<IRCommand> {
    const newCommand: IRCommand = {
      ...command,
      id: this.generateId(),
      capturedAt: new Date(),
    }
    this.commands.set(newCommand.id, newCommand)
    this.notifyListeners(command.deviceId)
    return newCommand
  }

  async update(id: string, updates: Partial<IRCommand>): Promise<void> {
    const command = this.commands.get(id)
    if (!command) {
      throw new Error(`Command ${id} not found`)
    }
    const updated = { ...command, ...updates }
    this.commands.set(id, updated)
    this.notifyListeners(command.deviceId)
  }

  async delete(id: string): Promise<void> {
    const command = this.commands.get(id)
    if (!command) return
    this.commands.delete(id)
    this.notifyListeners(command.deviceId)
  }

  subscribe(deviceId: string, callback: (commands: IRCommand[]) => void): () => void {
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
      this.getByDevice(deviceId).then(commands => {
        deviceListeners.forEach(callback => callback(commands))
      })
    }
  }

  private generateId(): string {
    return `cmd_${Date.now()}_${Math.random().toString(36).substr(2, 9)}`
  }

  seedMockData(deviceId: string): void {
    const mockCommands: Omit<IRCommand, 'id' | 'capturedAt'>[] = [
      {
        deviceId,
        name: 'Power',
        protocol: 'NEC',
        address: '0x00',
        command: '0x01',
        value: '16724175',
        bits: 32,
      },
      {
        deviceId,
        name: 'Volume Up',
        protocol: 'NEC',
        address: '0x00',
        command: '0x10',
        value: '16712445',
        bits: 32,
      },
      {
        deviceId,
        name: 'Volume Down',
        protocol: 'NEC',
        address: '0x00',
        command: '0x11',
        value: '16750695',
        bits: 32,
      },
      {
        deviceId,
        name: 'Channel Up',
        protocol: 'NEC',
        address: '0x00',
        command: '0x20',
        value: '16718055',
        bits: 32,
      },
      {
        deviceId,
        name: 'Channel Down',
        protocol: 'NEC',
        address: '0x00',
        command: '0x21',
        value: '16756815',
        bits: 32,
      },
    ]

    mockCommands.forEach(cmd => this.create(cmd))
  }
}
