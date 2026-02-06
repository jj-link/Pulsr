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
## Future: Shared Device Selection

The Remote tab uses URL-based device selection (`/remote/:deviceId`). In a future iteration, the Learning page could adopt the same pattern (`/learn/:deviceId`) or read the last-selected device from the URL to default its inline `DeviceSelector`. This would allow seamless device context when switching between tabs. For now, the Learning page keeps its own independent `DeviceSelector`.

## Device Pairing Strategy

### Problem
Web app and ESP32 use different device IDs, preventing communication.

### Solutions

#### Phase 1: Hardcoded Test (Immediate)
For quick testing with matching IDs:
1. Manually set device ID in both web app and ESP32 config.h
2. Verify learning mode works end-to-end
3. Confirm IR signal capture and display

**Purpose:** Validate hardware/software integration before implementing pairing.

#### Phase 2: WiFi Hotspot Pairing (Consumer UX)
Industry-standard setup for end users:

**Flow:**
1. ESP32 boots in setup mode → Creates WiFi hotspot "Pulsr-Setup"
2. User connects phone to hotspot
3. Opens browser to 192.168.4.1
4. Web page displays: pairing code + WiFi config form
5. User enters home WiFi credentials
6. ESP32 connects, gets device ID assignment
7. Stores ID in NVS, reboots to normal mode

**Implementation:**
- ESP32: AP mode + DNS server + HTTP server (200-300 lines)
- Web app: Add "Pair New Device" page
- Firestore: Link MAC address to device ID

**Alternative: PIN Code Pairing** (Simpler hardware)
- ESP32 displays 6-digit code via LED pattern or serial
- User enters code when creating device
- Code links MAC to device ID
- Requires display for consumer-friendly UX

### Decision
- **Developer mode:** Serial output + hardcoded IDs + temp Firebase creds in config.h
- **Consumer mode:** WiFi hotspot or PIN code pairing + Firebase Secrets via Cloud Functions

## Security: Firebase Credentials

### Current (Temporary)
Hardcoded Firebase credentials in `esp32/include/config.h` for testing.

### Phase 3: Firebase Secrets (Production)
Remove hardcoded credentials, use secure token exchange:

**Implementation:**
1. Firebase Cloud Functions with environment variables for API key
2. HTTPS endpoint: `POST /getDeviceToken` with device ID + MAC address
3. Cloud Function verifies device, returns short-lived Firebase Custom Token
4. ESP32 uses token to authenticate (no credentials stored)
5. Token expires after 1 hour, device refreshes via same endpoint

**Security Benefits:**
- API key never leaves server
- Revocable tokens per device
- MAC address validation prevents spoofing

**Files to Create:**
- `functions/src/deviceAuth.ts` - Cloud Function
- `functions/.env` - API key secrets (gitignored)
- Update `FirebaseManager.cpp` to use token endpoint

## Current Status
- Web UI: Complete with TDD (33 tests passing)
- ESP32: Firestore integration ready with pendingSignal flow
- WiFi: Working with setTxPower fix
- Learning flow: ESP32 writes pendingSignal → web modal shows it → user names + saves → command created
- Edit/delete commands: Working with composite deviceId/commandId format
- NeoPixel feedback: Blue = learning, green = captured, orange = timeout
- Nav: "Devices" tab renamed to "Learn"
