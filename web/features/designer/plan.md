# Designer - Web UI Implementation

**Purpose:** Provide a click-to-place grid editor for custom remote layouts.

## Build Strategy

### Phase 1: Click-to-Place Grid (Current)
- Fixed grid with configurable size (default: **3 columns × 4 rows**)
- Click an empty cell → modal to assign command, set label, choose color
- Click a filled cell → edit or delete the button
- Grid size selector in the editor toolbar
- No drag-and-drop — simple click-to-place/click-to-edit
- Same Firestore schema as full version (upgrade path preserved)

### Phase 1.5: Inline Learning in ButtonConfigModal (Next)
- Add "Learn New Command" option inside ButtonConfigModal alongside the existing command dropdown
- Reuses `LearningModal` logic (pendingSignal listener, learning mode toggle)
- Flow: click empty cell → modal → "Learn New Command" → ESP32 captures signal → name it → assign color → done
- Eliminates tab-switching between Learn and Designer for the common "build a remote" workflow
- The standalone Learn tab remains for bulk command management and power users

### Phase 2: Drag-and-Drop Upgrade (Future)
- Add `@dnd-kit/core` for rearranging buttons by dragging
- Multi-cell button sizes (span rows/cols)
- Icon picker (Lucide)
- This is an upgrade to Phase 1 — same data model, just richer editor UX

## Components

### DesignerPage
Main page with grid editor and toolbar.

**Responsibilities:**
- Render the grid based on layout data
- Grid size selector (rows × cols)
- Save/cancel layout changes
- Show empty cells as clickable placeholders

### ButtonConfigModal
Configure individual button properties.

**Responsibilities:**
- Select command from learned commands list
- **"Learn New Command" inline option** — triggers learning mode, captures IR signal, saves command, and assigns it to the button in one flow
- Set label text
- Set color
- Delete button option

### GridCell
Individual cell in the layout grid.

**Responsibilities:**
- Show button preview if assigned (label + color)
- Show "+" placeholder if empty
- Click handler opens ButtonConfigModal

## Testing Strategy

### Unit Tests (TDD)
- Layout validation (no overlapping buttons)
- Position within grid bounds
- JSON serialization/deserialization
- Grid size changes preserve existing buttons within bounds

### Integration Tests
- Save/load layout round-trip
- Mock Firestore persistence

### E2E Tests (Playwright)
- User can click empty cell and assign command
- User can edit existing button
- User can delete button
- Layout saves and persists
- Grid size change works correctly

## Tech Stack

- **React 18** with TypeScript (strict mode)
- **Firebase SDK** - Firestore persistence
- **Lucide React** - Icons
- **Vitest** - Unit tests (validation logic)
- **React Testing Library** - Component tests
- **Playwright** - E2E tests

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

Build with `InMemoryLayoutRepository` before Firestore ready:

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
      gridSize: { rows: number, cols: number }  // default: 4 rows × 3 cols
      buttons: [
        {
          id: string
          position: { row: number, col: number }
          commandId: string
          color: string (hex)
          label: string
        }
      ]
    }
```

**Note:** `size` (multi-cell spans) and `icon` fields are reserved for Phase 2 drag-and-drop upgrade.

## File Structure

```
features/designer/
├── components/
│   ├── GridCell.tsx
│   ├── ButtonConfigModal.tsx
│   └── index.ts
├── pages/
│   ├── DesignerPage.tsx
│   ├── DesignerPage.css
│   └── index.ts
├── hooks/
│   ├── useLayout.ts
│   └── index.ts
├── repositories/
│   ├── FirestoreLayoutRepository.ts
│   ├── InMemoryLayoutRepository.ts
│   └── index.ts
├── utils/
│   ├── layoutValidation.ts
│   └── index.ts
└── __tests__/
    ├── layoutValidation.test.ts
    └── useLayout.test.ts
```

## Future: Shared Device Selection

The Remote tab uses URL-based device selection (`/remote/:deviceId`). In a future iteration, the Designer page could adopt the same pattern (`/designer/:deviceId`) or read the last-selected device from the URL to default its device context. This would allow seamless device context when switching between tabs. For now, the Designer page will use its own device selection mechanism.

## Future: Drag-and-Drop Upgrade

Once Phase 1 is working and stable:
- Add `@dnd-kit/core` for drag-to-rearrange
- Add multi-cell button sizes (`size: { rows, cols }`)
- Add Lucide icon picker for buttons
- Add color picker component
- The Firestore schema already supports these fields — no migration needed

## Integration

Reads:
- Commands from Learning track (via existing command repository)
- Layout data from Firestore (via ILayoutRepository)

Writes:
- Layout configuration to Firestore (via ILayoutRepository)
- New commands to Firestore when using inline "Learn New Command" (via IDeviceRepository + ICommandRepository)

Triggers:
- Learning mode on ESP32 when using inline learning (sets `device.isLearning = true`)
- Transmission track when button clicked on Remote page (uses IQueueRepository)
