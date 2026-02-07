import { IRCommand, Device, DeviceLayout, ChatSession, KnowledgeArticle } from './index'

export interface ICommandRepository {
  getByDevice(deviceId: string): Promise<IRCommand[]>
  getById(id: string): Promise<IRCommand | null>
  create(command: Omit<IRCommand, 'id' | 'capturedAt'>): Promise<IRCommand>
  update(id: string, updates: Partial<IRCommand>): Promise<void>
  delete(id: string): Promise<void>
  subscribe(deviceId: string, callback: (commands: IRCommand[]) => void): () => void
}

export interface IDeviceRepository {
  getAll(): Promise<Device[]>
  getById(id: string): Promise<Device | null>
  create(device: Omit<Device, 'id'> & { id?: string }): Promise<Device>
  update(id: string, updates: Partial<Device>): Promise<void>
  delete(id: string): Promise<void>
  setLearningMode(deviceId: string, isLearning: boolean): Promise<void>
  clearPendingSignal(deviceId: string): Promise<void>
  subscribe(callback: (devices: Device[]) => void): () => void
}

export interface ILayoutRepository {
  getByDevice(deviceId: string): Promise<DeviceLayout | null>
  save(deviceId: string, layout: DeviceLayout): Promise<void>
  subscribe(deviceId: string, callback: (layout: DeviceLayout | null) => void): () => void
}

export interface IChatRepository {
  getSession(sessionId: string): Promise<ChatSession | null>
  createSession(userId: string): Promise<ChatSession>
  addMessage(sessionId: string, role: 'user' | 'assistant', content: string): Promise<void>
  getUserSessions(userId: string): Promise<ChatSession[]>
}

export interface IKnowledgeRepository {
  search(query: string): Promise<KnowledgeArticle[]>
  getByCategory(category: string): Promise<KnowledgeArticle[]>
  getAll(): Promise<KnowledgeArticle[]>
}
