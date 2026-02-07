export interface IRCommand {
  id: string
  deviceId: string
  name: string
  protocol: string
  address: string
  command: string
  capturedAt: Date
}

export interface PendingSignal {
  protocol: string
  address: string
  command: string
  value: string
  bits: number
  isKnownProtocol: boolean
  capturedAt: Date
}

export interface Device {
  id: string
  name: string
  isLearning: boolean
  layout?: DeviceLayout
  ownerId: string
  pendingSignal?: PendingSignal | null
}

export interface DeviceLayout {
  gridSize: { rows: number; cols: number }
  buttons: LayoutButton[]
}

export interface LayoutButton {
  id: string
  commandId: string
  label: string
  color: string
  position: { row: number; col: number }
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
