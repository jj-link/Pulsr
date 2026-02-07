# Remote - Web UI Implementation

**Purpose:** Provide remote control buttons that trigger IR transmission.

## Navigation & Device Selection

The "Remote" tab in the top nav is a **dropdown menu** listing all devices. Selecting a device navigates to `/remote/:deviceId` and opens the remote view for that device.

**Behavior:**
- Clicking the "Remote" nav item opens a dropdown of available devices
- Selecting a device navigates to `/remote/:deviceId`
- The tab label shows the selected device: "Remote: Living Room TV ▼"
- If no device is selected, shows "Remote ▼" and the dropdown prompts selection
- URL-based routing allows bookmarking and sharing device remotes

**URL scheme:** `/remote/:deviceId`

**Shared device context:** The selected `deviceId` from the URL param can be read by other tabs (Learn, Designer) to default to the same device. This is optional — other tabs keep their own inline `DeviceSelector` for now but can adopt the URL param later.

## Build Strategy

The Remote page depends on the Designer to define button layouts. Since the Designer isn't built yet, we use a phased approach:

### Phase 1: Empty State + Test Transmit Panel (Current)
- **Default behavior:** Remote page shows an empty state: *"No buttons configured. Go to Designer to set up your remote."*
- **Test panel:** A collapsible "Test Transmit" debug panel lists all learned commands for the selected device, each with a "Send" button. This allows testing the full RTDB → ESP32 → IR LED pipeline without a layout.
- **Purpose:** Validate end-to-end transmission before investing in drag-and-drop Designer.
- **Lifecycle:** The test panel is temporary — it will be removed or hidden behind a dev flag once the Designer is complete.

### Phase 2: Layout-Driven Remote (After Designer)
- Remote page reads the device's `layout` from Firestore (created via Designer's click-to-place grid)
- Renders buttons in a CSS grid matching the layout's `gridSize` (default: 3 cols × 4 rows)
- Each button shows its label, color, and dispatches the assigned command via RTDB on click
- The empty state only shows when no layout has been created yet
- Test panel is removed or hidden behind a dev flag

## Components

### RemoteButton
Individual button component for IR commands.

**Props:**
- `commandId`: Reference to Firestore command
- `icon`: Lucide icon name
- `color`: Button color
- `label`: Button text

**Responsibilities:**
- Write `pendingCommand` to RTDB on click (protocol, address, command, bits)
- Show optimistic "pressed" state
- Handle errors with retry option

### RemoteGrid
Layout container for buttons.

**Responsibilities:**
- Render buttons from device layout
- Responsive grid (mobile/desktop)
- Edit mode toggle

### TransmitStatus
Display transmission feedback.

**Responsibilities:**
- Show "sending" flash on button press
- Error notifications on RTDB write failure

## Testing Strategy

### Unit Tests (TDD)
- Button click dispatches command to RTDB
- Error handling logic

### Integration Tests
- Mock RTDB writes
- Test dispatch flow

### E2E Tests (Playwright)
- User can press button
- Button shows "pressed" state
- Success feedback appears
- Failed transmission shows retry option

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
interface ICommandDispatch {
  sendCommand(deviceId: string, command: {
    protocol: string
    address: number
    command: number
    bits: number
  }): Promise<void>
}

interface ILayoutRepository {
  getLayout(deviceId: string): Promise<Layout>
  saveLayout(deviceId: string, layout: Layout): Promise<void>
}
```

### Custom Hooks

Components use hooks that wrap repositories:

```typescript
// useTransmission.ts
function useTransmission(deviceId: string) {
  const dispatch = useCommandDispatch()
  const [sending, setSending] = useState<string | null>(null)
  
  const pressButton = useCallback(async (command: CommandDetails) => {
    setSending(command.id)
    try {
      await dispatch.sendCommand(deviceId, {
        protocol: command.protocol,
        address: command.address,
        command: command.command,
        bits: command.bits,
      })
    } finally {
      setTimeout(() => setSending(null), 300)
    }
  }, [deviceId, dispatch])
  
  return { pressButton, sending }
}
```

### Mock-First Development

Build with `InMemoryCommandDispatch` before ESP32 ready:

```typescript
class InMemoryCommandDispatch implements ICommandDispatch {
  async sendCommand(deviceId: string, command: CommandDetails) {
    console.log(`[Mock] Sending ${command.protocol} to ${deviceId}`)
    // Simulate network delay
    await new Promise(resolve => setTimeout(resolve, 100))
  }
}
```

## Data Schema

### Firestore (persistent storage)
```
devices/{deviceId}
  - name: string
  - layout: object (button positions, icons, colors)
  
  commands/{commandId}
    - protocol: string
    - address: number
    - command: number
    - bits: number
    - label: string
```

### RTDB (real-time dispatch)
```
devices/{deviceId}
  - isLearning: boolean
  - pendingCommand: {
      protocol: string
      address: number
      command: number
      bits: number
      timestamp: number
    }
```

## File Structure

```
features/remote/
├── components/
│   ├── RemoteButton.tsx
│   ├── RemoteGrid.tsx
│   └── index.ts
├── hooks/
│   ├── useTransmission.ts
│   ├── useLayout.ts
│   └── index.ts
├── repositories/
│   ├── ICommandDispatch.ts
│   ├── ILayoutRepository.ts
│   ├── RTDBCommandDispatch.ts
│   ├── FirestoreLayoutRepository.ts
│   ├── InMemoryCommandDispatch.ts
│   ├── InMemoryLayoutRepository.ts
│   └── index.ts
├── types/
│   └── index.ts
└── __tests__/
    ├── RemoteButton.test.tsx
    ├── RemoteGrid.test.tsx
    ├── useTransmission.test.ts
    └── repositories.test.ts
```
