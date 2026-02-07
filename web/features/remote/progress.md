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
- [x] RTDB command dispatch (write `pendingCommand` directly from RemotePage)

### Navigation
- [x] **Nav dropdown** (`AppLayout.tsx`) — "Remote ▼" lists devices, navigates to `/remote/:deviceId`
- [x] URL-based device selection (`/remote/:deviceId`)
- [x] Tab label shows selected device: "Remote: {name} ▼"

### Components (inlined in RemotePage)
- [x] **Layout grid** — renders buttons from Designer layout (CSS grid, label + color)
- [x] **Button click → RTDB dispatch** — writes `pendingCommand` to RTDB on click
- [x] **Test Transmit panel** — collapsible debug panel listing all learned commands with Send buttons
- [x] **Removed Firestore queue** — no more FirestoreQueueRepository, QueueItem, useQueue
- [ ] Optimistic "pressed" state on button click
- [ ] Error handling with retry option

### Testing
- [ ] Unit tests for dispatch logic
- [ ] Playwright E2E tests

**Test count: 0 Remote-specific tests**

## Blockers

None.

## Next Steps

1. Add optimistic press feedback (button shows "sending" state briefly)
2. Remove or hide Test Transmit panel behind a dev flag
3. Add unit tests for dispatch logic
4. Add Playwright E2E tests
