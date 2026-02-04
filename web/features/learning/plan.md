# Learning - Web UI Implementation

**Purpose:** Provide UI for triggering learning mode and viewing captured commands.

## Components

### LearningModal
Modal dialog for learning mode workflow.

**States:**
- Waiting: "Point remote at device and press button..."
- Success: "Command captured! Name your button:"
- Timeout: "No signal detected. Try again?"

**Responsibilities:**
- Trigger learning mode (set `device.isLearning = true`)
- Listen for new command documents in real-time
- Display status indicator
- Prompt for command name
- Cancel learning mode

### CommandList
Display all learned commands for a device.

**Responsibilities:**
- Fetch commands from Firestore
- Display protocol, address, command code
- Show capture timestamp
- Delete command action
- Filter/search commands

### DeviceSelector
Choose which device to learn commands for.

**Responsibilities:**
- List available devices
- Create new device
- Select active device for learning

## Testing Strategy

### Unit Tests (TDD)
- Learning modal state transitions
- Command list filtering logic
- Form validation

### Integration Tests
- Mock Firestore to test real-time updates
- Test learning flow end-to-end

### E2E Tests (Playwright)
- User can trigger learning mode
- User sees "Waiting..." indicator
- User receives success notification
- User can name and save command

## Tech Stack

- **React 18** with TypeScript (strict mode)
- **Firebase SDK** - Firestore real-time listeners
- **shadcn/ui** - Accessible UI components (Radix UI primitives)
- **Lucide React** - Icons
- **TailwindCSS** - Styling
- **Vitest** - Unit tests
- **React Testing Library** - Component tests
- **Playwright** - E2E tests

## Implementation Pattern

### Repository Pattern

All Firestore interactions use repository interfaces:

```typescript
interface ICommandRepository {
  getCommands(deviceId: string): Promise<Command[]>
  addCommand(command: Command): Promise<void>
  deleteCommand(id: string): Promise<void>
  listenToCommands(
    deviceId: string,
    callback: (commands: Command[]) => void
  ): () => void
}

interface IDeviceRepository {
  getDevices(): Promise<Device[]>
  getDevice(id: string): Promise<Device>
  setLearningMode(deviceId: string, isLearning: boolean): Promise<void>
  createDevice(device: Omit<Device, 'id'>): Promise<string>
}
```

### Custom Hooks

Components use hooks that wrap repositories:

```typescript
// useCommands.ts
function useCommands(deviceId: string) {
  const [commands, setCommands] = useState<Command[]>([])
  const [loading, setLoading] = useState(true)
  const repo = useCommandRepository()
  
  useEffect(() => {
    setLoading(true)
    return repo.listenToCommands(deviceId, (cmds) => {
      setCommands(cmds)
      setLoading(false)
    })
  }, [deviceId, repo])
  
  const deleteCommand = useCallback(
    (id: string) => repo.deleteCommand(id),
    [repo]
  )
  
  return { commands, loading, deleteCommand }
}
```

### Mock-First Development

Build with `InMemoryCommandRepository` before ESP32 ready:

```typescript
class InMemoryCommandRepository implements ICommandRepository {
  private commands = new Map<string, Command[]>()
  
  async getCommands(deviceId: string) {
    return this.commands.get(deviceId) || []
  }
  
  listenToCommands(deviceId: string, callback) {
    callback(this.commands.get(deviceId) || [])
    return () => {} // unsubscribe
  }
}
```

## Firestore Schema

```
devices/{deviceId}
  - name: string
  - isLearning: boolean
  - createdAt: timestamp
  - ownerId: string
  
  commands/{commandId}
    - name: string
    - protocol: 'NEC' | 'Sony' | 'Samsung' | 'RC5' | 'RC6'
    - address: string (hex)
    - command: string (hex)
    - capturedAt: timestamp
```

## File Structure

```
features/learning/
├── components/
│   ├── LearningModal.tsx
│   ├── CommandList.tsx
│   ├── DeviceSelector.tsx
│   └── index.ts
├── hooks/
│   ├── useCommands.ts
│   ├── useLearningMode.ts
│   ├── useDevices.ts
│   └── index.ts
├── repositories/
│   ├── ICommandRepository.ts
│   ├── IDeviceRepository.ts
│   ├── FirestoreCommandRepository.ts
│   ├── FirestoreDeviceRepository.ts
│   ├── InMemoryCommandRepository.ts
│   ├── InMemoryDeviceRepository.ts
│   └── index.ts
├── types/
│   └── index.ts
└── __tests__/
    ├── LearningModal.test.tsx
    ├── CommandList.test.tsx
    ├── useCommands.test.ts
    └── repositories.test.ts
```
