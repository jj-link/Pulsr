# Designer - Web UI Implementation

**Purpose:** Provide drag-and-drop interface for custom remote layouts.

## Components

### LayoutEditor
Main editor component with drag-and-drop.

**Responsibilities:**
- Render button grid
- Handle drag-and-drop interactions
- Edit mode toggle
- Save/cancel layout changes
- Optimistic UI updates

### ButtonConfigModal
Configure individual button properties.

**Responsibilities:**
- Select command from library
- Choose icon (Lucide icon picker)
- Set color
- Set label text

### CommandPicker
Select command to assign to button.

**Responsibilities:**
- Display available commands
- Filter by device
- Show protocol information
- Search functionality

## Testing Strategy

### Unit Tests (TDD)
- Layout validation (no overlapping buttons)
- Position within grid bounds
- JSON serialization/deserialization
- Command filtering by device

### Integration Tests
- Save/load layout round-trip
- Mock Firestore persistence

### E2E Tests (Playwright)
- User can drag button to new position
- User can add new button
- User can assign command to button
- Layout saves and persists

**Note:** Drag-and-drop interactions difficult to unit test - rely on E2E tests and manual testing for UX validation.

## Tech Stack

- **React 18** with TypeScript (strict mode)
- **@dnd-kit/core** - Modern drag-and-drop library (recommended over react-dnd)
- **Firebase SDK** - Firestore persistence
- **shadcn/ui** - Accessible UI components (Radix UI primitives)
- **Lucide React** - Icons (icon picker for button config)
- **TailwindCSS** - Styling
- **Vitest** - Unit tests (validation logic)
- **React Testing Library** - Component tests
- **Playwright** - E2E tests (drag-and-drop workflows)

## Implementation Pattern

### Repository Pattern

All Firestore interactions use repository interfaces:

```typescript
interface ILayoutRepository {
  getLayout(deviceId: string): Promise<Layout>
  saveLayout(deviceId: string, layout: Layout): Promise<void>
  validateLayout(layout: Layout): ValidationResult
}

interface ICommandRepository {
  getCommands(deviceId: string): Promise<Command[]>
}
```

### Custom Hooks

Components use hooks that wrap repositories:

```typescript
// useLayout.ts
function useLayout(deviceId: string) {
  const [layout, setLayout] = useState<Layout | null>(null)
  const [isDirty, setIsDirty] = useState(false)
  const repo = useLayoutRepository()
  
  useEffect(() => {
    repo.getLayout(deviceId).then(setLayout)
  }, [deviceId, repo])
  
  const updateLayout = useCallback((newLayout: Layout) => {
    setLayout(newLayout)
    setIsDirty(true)
  }, [])
  
  const save = useCallback(async () => {
    if (!layout) return
    const validation = repo.validateLayout(layout)
    if (!validation.valid) throw new Error(validation.error)
    await repo.saveLayout(deviceId, layout)
    setIsDirty(false)
  }, [deviceId, layout, repo])
  
  return { layout, updateLayout, save, isDirty }
}
```

### Validation Logic (TDD)

Layout validation is pure logic, perfect for TDD:

```typescript
// layoutValidation.ts
interface ValidationResult {
  valid: boolean
  error?: string
}

export function validateLayout(layout: Layout): ValidationResult {
  // Check for overlapping buttons
  for (let i = 0; i < layout.buttons.length; i++) {
    for (let j = i + 1; j < layout.buttons.length; j++) {
      if (buttonsOverlap(layout.buttons[i], layout.buttons[j])) {
        return { valid: false, error: 'Buttons cannot overlap' }
      }
    }
  }
  
  // Check all buttons within grid bounds
  for (const button of layout.buttons) {
    if (!withinBounds(button, layout.gridSize)) {
      return { valid: false, error: 'Button outside grid bounds' }
    }
  }
  
  return { valid: true }
}
```

### Mock-First Development

Build with `InMemoryLayoutRepository` before ESP32 ready:

```typescript
class InMemoryLayoutRepository implements ILayoutRepository {
  private layouts = new Map<string, Layout>()
  
  async getLayout(deviceId: string): Promise<Layout> {
    return this.layouts.get(deviceId) || createDefaultLayout()
  }
  
  async saveLayout(deviceId: string, layout: Layout): Promise<void> {
    const validation = this.validateLayout(layout)
    if (!validation.valid) throw new Error(validation.error)
    this.layouts.set(deviceId, layout)
  }
  
  validateLayout(layout: Layout): ValidationResult {
    return validateLayout(layout)
  }
}
```

## Firestore Schema

```
devices/{deviceId}
  - name: string
  - layout: {
      gridSize: { rows: number, cols: number }
      buttons: [
        {
          id: string
          position: { row: number, col: number }
          size: { rows: number, cols: number }
          commandId: string
          icon: string (Lucide icon name)
          color: string (hex)
          label: string
        }
      ]
    }
```

## File Structure

```
features/designer/
├── components/
│   ├── LayoutEditor.tsx
│   ├── ButtonConfigModal.tsx
│   ├── CommandPicker.tsx
│   ├── IconPicker.tsx
│   ├── ColorPicker.tsx
│   └── index.ts
├── hooks/
│   ├── useLayout.ts
│   ├── useDragAndDrop.ts
│   ├── useCommands.ts
│   └── index.ts
├── repositories/
│   ├── ILayoutRepository.ts
│   ├── FirestoreLayoutRepository.ts
│   ├── InMemoryLayoutRepository.ts
│   └── index.ts
├── utils/
│   ├── layoutValidation.ts
│   ├── gridHelpers.ts
│   └── index.ts
├── types/
│   └── index.ts
└── __tests__/
    ├── LayoutEditor.test.tsx
    ├── layoutValidation.test.ts
    ├── gridHelpers.test.ts
    └── useLayout.test.ts
```

## Integration

Reads:
- Commands from Decoder track (via ICommandRepository)
- Layout data from Firestore (via ILayoutRepository)

Writes:
- Layout configuration to Firestore (via ILayoutRepository)

Triggers:
- Transmission track when button clicked (uses Transmission's IQueueRepository)
