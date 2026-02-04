# Remote - Web UI Status

**Last Updated:** 2026-02-02  
**Phase:** Not Started

## Progress

### Foundation
- [ ] React app scaffold
- [ ] Firebase SDK setup

### Components
- [ ] **RemoteButton**
  - [ ] Command enqueue on click
  - [ ] Optimistic "pressed" state
  - [ ] Status listener (pending/sent/failed)
  - [ ] Error handling with retry
- [ ] **RemoteGrid**
  - [ ] Responsive layout
  - [ ] Button rendering from layout data
  - [ ] Edit mode toggle
- [ ] **QueueMonitor**
  - [ ] Pending count display
  - [ ] Recent transmission list
  - [ ] Error notifications

### Testing
- [ ] Unit tests for button logic
- [ ] Integration tests with mock Firestore
- [ ] Playwright E2E tests

## Blockers

Requires:
- ESP32 Transmitter track (queue processor)
- Designer track (layout data)

## Next Steps

1. Wait for ESP32 transmitter and Designer tracks
2. Implement RemoteButton with TDD
3. Add real-time queue status listeners
