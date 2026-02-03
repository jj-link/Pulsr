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

## Dependencies

- React
- dnd-kit or react-dnd (drag-and-drop)
- Firebase SDK
- UI library (shadcn/ui, Lucide icons)
- TailwindCSS

## Integration

Reads:
- Commands from Decoder track (command library)
- Layout data from Firestore

Writes:
- Layout configuration to Firestore

Triggers:
- Transmission track when button clicked
