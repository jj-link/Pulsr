# Decoder - Web UI Implementation

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

## Dependencies

- React
- Firebase SDK (Firestore real-time listeners)
- UI library (shadcn/ui, Lucide icons)
- TailwindCSS

## Integration

See `docs/contracts/decoder.md` for Firestore data model and integration flow.
