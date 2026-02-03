# Transmission - Web UI Implementation

**Purpose:** Provide remote control buttons that trigger IR transmission.

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

## Dependencies

- React
- Firebase SDK
- UI library (shadcn/ui, Lucide icons)
- TailwindCSS

## Integration

See `docs/contracts/transmission.md` for Firestore data model and integration flow.
