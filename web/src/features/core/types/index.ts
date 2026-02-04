export interface IRCommand {
  id: string
  deviceId: string
  name: string
  protocol: string
  address: string
  command: string
  capturedAt: Date
}

export interface Device {
  id: string
  name: string
  isLearning: boolean
  layout?: DeviceLayout
  ownerId: string
}

export interface DeviceLayout {
  buttons: LayoutButton[]
}

export interface LayoutButton {
  id: string
  commandId: string
  label: string
  position: {
    x: number
    y: number
  }
  size: {
    width: number
    height: number
  }
}

export interface QueueItem {
  id: string
  deviceId: string
  commandId: string
  status: 'pending' | 'processing' | 'completed' | 'failed'
  createdAt: Date
  processedAt?: Date
  error?: string
}

export interface KnowledgeArticle {
  id: string
  title: string
  content: string
  keywords: string[]
  category: string
  createdAt: Date
}

export interface ChatMessage {
  id: string
  role: 'user' | 'assistant'
  content: string
  timestamp: Date
}

export interface ChatSession {
  id: string
  userId: string
  messages: ChatMessage[]
  createdAt: Date
}
