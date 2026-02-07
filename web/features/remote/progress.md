# Remote - Web UI Status

**Last Updated:** 2026-02-06  
**Phase:** Phase 2 Complete (Layout-Driven Remote)

## Plan Evolution

The original plan assumed separate `RemoteButton`, `RemoteGrid`, and `QueueMonitor` components. In practice, we built:
- **Phase 1:** Empty state + "Test Transmit" collapsible panel for validating the queue → ESP32 → IR pipeline
- **Phase 2:** Layout-driven grid from Designer, with the test panel still present as a debug fallback

The component architecture is simpler than planned — logic is inlined in `RemotePage.tsx` rather than split into many small components. This can be refactored if complexity grows.

## Progress

### Foundation
- [x] React app scaffold (shared)
- [x] Firebase SDK setup (shared)
- [x] `FirestoreQueueRepository` (enqueue, subscribe, recent transmissions)
- [x] `InMemoryQueueRepository` (mock for testing)

### Navigation
- [x] **Nav dropdown** (`AppLayout.tsx`) — "Remote ▼" lists devices, navigates to `/remote/:deviceId`
- [x] URL-based device selection (`/remote/:deviceId`)
- [x] Tab label shows selected device: "Remote: {name} ▼"

### Components (inlined in RemotePage)
- [x] **Layout grid** — renders buttons from Designer layout (CSS grid, label + color)
- [x] **Button click → enqueue** — adds to Firestore queue on click
- [x] **Recent transmissions list** — shows last 5 queue items with status badges
- [x] **Test Transmit panel** — collapsible debug panel listing all learned commands with Send buttons
- [ ] **RemoteButton** as standalone component (inlined — extract if needed)
- [ ] **RemoteGrid** as standalone component (inlined — extract if needed)
- [ ] **QueueMonitor** as standalone component (inlined as recent list)
- [ ] Optimistic "pressed" state on button click
- [ ] Error handling with retry option
- [ ] Real-time status listener per queue item

### Testing
- [ ] Unit tests for button/queue logic
- [ ] Integration tests with mock Firestore
- [ ] Playwright E2E tests

**Test count: 0 Remote-specific tests (queue repo covered by shared tests)**

## Blockers

~~Requires ESP32 Transmitter track and Designer track.~~ **RESOLVED** — both complete.

No current blockers.

## Next Steps

1. Remove or hide Test Transmit panel behind a dev flag (Phase 1 artifact)
2. Add optimistic press feedback (button shows "sending" state briefly)
3. Extract `RemoteButton` / `RemoteGrid` components if complexity grows
4. Add unit tests for enqueue logic
5. Add Playwright E2E tests (press button, verify queue item created, status updates)
