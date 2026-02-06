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
- **Test panel:** A collapsible "Test Transmit" debug panel lists all learned commands for the selected device, each with a "Send" button. This allows testing the full queue → ESP32 → IR LED pipeline without a layout.
- **Purpose:** Validate end-to-end transmission before investing in drag-and-drop Designer.
- **Lifecycle:** The test panel is temporary — it will be removed or hidden behind a dev flag once the Designer is complete.

### Phase 2: Layout-Driven Remote (After Designer)
- Remote page reads the device's `layout` field from Firestore
- Renders buttons according to the layout grid
- The empty state only shows when no layout has been created yet
- Test panel is removed

## Components

### RemoteButton
Individual button component for IR commands.

**Props:**
- `commandId`: Reference to Firestore command
- `icon`: Lucide icon name
- `color`: Button color
- `label`: Button text

**Responsibilities:**
- Enqueue command to Firestore on click
- Show optimistic "pressed" state
- Display status (pending/sent/failed)
- Handle errors with retry option

### RemoteGrid
Layout container for buttons.

**Responsibilities:**
- Render buttons from device layout
- Responsive grid (mobile/desktop)
- Edit mode toggle

### QueueMonitor
Display transmission queue status.

**Responsibilities:**
- Show pending command count
- Display recent transmissions
- Error notifications

## Testing Strategy

### Unit Tests (TDD)
- Button click enqueues command
- Status updates correctly
- Error handling logic

### Integration Tests
- Mock Firestore queue writes
- Test status listener updates

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
interface IQueueRepository {
  enqueueCommand(deviceId: string, commandId: string): Promise<string>
  listenToQueueItem(
    queueItemId: string,
    callback: (status: QueueStatus) => void
  ): () => void
  getRecentTransmissions(deviceId: string, limit: number): Promise<QueueItem[]>
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
  const queueRepo = useQueueRepository()
  const [status, setStatus] = useState<Record<string, QueueStatus>>({})
  
  const pressButton = useCallback(async (commandId: string) => {
    const queueItemId = await queueRepo.enqueueCommand(deviceId, commandId)
    
    // Optimistic update
    setStatus(prev => ({ ...prev, [commandId]: 'pending' }))
    
    // Listen for status updates
    return queueRepo.listenToQueueItem(queueItemId, (newStatus) => {
      setStatus(prev => ({ ...prev, [commandId]: newStatus }))
    })
  }, [deviceId, queueRepo])
  
  return { pressButton, status }
}
```

### Mock-First Development

Build with `InMemoryQueueRepository` before ESP32 ready:

```typescript
class InMemoryQueueRepository implements IQueueRepository {
  private queue = new Map<string, QueueItem>()
  
  async enqueueCommand(deviceId: string, commandId: string) {
    const id = crypto.randomUUID()
    this.queue.set(id, {
      id,
      deviceId,
      commandId,
      status: 'pending',
      createdAt: new Date()
    })
    
    // Simulate ESP32 processing
    setTimeout(() => {
      const item = this.queue.get(id)!
      item.status = 'sent'
      this.queue.set(id, item)
    }, 1000)
    
    return id
  }
}
```

## Firestore Schema

```
devices/{deviceId}
  - name: string
  - layout: object (button positions, icons, colors)
  
  queue/{queueItemId}
    - commandId: string
    - status: 'pending' | 'sent' | 'failed'
    - createdAt: timestamp
    - processedAt: timestamp (optional)
    - error: string (optional)
```

## File Structure

```
features/remote/
├── components/
│   ├── RemoteButton.tsx
│   ├── RemoteGrid.tsx
│   ├── QueueMonitor.tsx
│   └── index.ts
├── hooks/
│   ├── useTransmission.ts
│   ├── useQueue.ts
│   ├── useLayout.ts
│   └── index.ts
├── repositories/
│   ├── IQueueRepository.ts
│   ├── ILayoutRepository.ts
│   ├── FirestoreQueueRepository.ts
│   ├── FirestoreLayoutRepository.ts
│   ├── InMemoryQueueRepository.ts
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
