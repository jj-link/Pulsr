# Remote - Web UI Status

**Last Updated:** 2026-02-07  
**Phase:** Migrating to RTDB Direct Command Dispatch

## Plan Evolution

The original plan assumed a Firestore queue pattern (enqueue document → ESP32 polls → processes). This is being replaced with **direct RTDB writes** — the web writes `pendingCommand` to RTDB and the ESP32 picks it up instantly via its existing RTDB stream (~100ms latency, same pattern as `isLearning`).

The component architecture is simpler than planned — logic is inlined in `RemotePage.tsx` rather than split into many small components.

## Progress

### Foundation
- [x] React app scaffold (shared)
- [x] Firebase SDK setup (shared)
- [x] `FirestoreQueueRepository` (legacy — being replaced by RTDB dispatch)
- [ ] `RTDBCommandDispatch` (write `pendingCommand` to RTDB)

### Navigation
- [x] **Nav dropdown** (`AppLayout.tsx`) — "Remote ▼" lists devices, navigates to `/remote/:deviceId`
- [x] URL-based device selection (`/remote/:deviceId`)
- [x] Tab label shows selected device: "Remote: {name} ▼"

### Components (inlined in RemotePage)
- [x] **Layout grid** — renders buttons from Designer layout (CSS grid, label + color)
- [x] **Button click → enqueue** — adds to Firestore queue on click (legacy)
- [x] **Recent transmissions list** — shows last 5 queue items with status badges (legacy)
- [x] **Test Transmit panel** — collapsible debug panel listing all learned commands with Send buttons
- [ ] **Button click → RTDB dispatch** — write `pendingCommand` directly to RTDB on click
- [ ] **Remove Firestore queue writes** — replace `FirestoreQueueRepository` with `RTDBCommandDispatch`
- [ ] Optimistic "pressed" state on button click
- [ ] Error handling with retry option

### Testing
- [ ] Unit tests for button/queue logic
- [ ] Integration tests with mock Firestore
- [ ] Playwright E2E tests

**Test count: 0 Remote-specific tests (queue repo covered by shared tests)**

## Blockers

None.

## Next Steps

1. **Replace `FirestoreQueueRepository` with `RTDBCommandDispatch`** — write `pendingCommand` to RTDB instead of Firestore queue
2. **Update `RemotePage.tsx`** to use new dispatch (load command details from Firestore, send via RTDB)
3. Add optimistic press feedback (button shows "sending" state briefly)
4. Remove or hide Test Transmit panel behind a dev flag
5. Add unit tests for dispatch logic
6. Add Playwright E2E tests
