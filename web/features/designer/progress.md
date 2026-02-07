# Designer - Web UI Status

**Last Updated:** 2026-02-06  
**Phase:** Phase 1 Complete (Click-to-Place Grid)

## Plan Evolution

The original plan assumed drag-and-drop as the primary editor. We pivoted to a **phased approach**:
- **Phase 1 (done):** Click-to-place grid with configurable size
- **Phase 2 (future):** Drag-and-drop upgrade with `@dnd-kit/core`, multi-cell spans, icon picker

This keeps the same Firestore schema — Phase 2 is a pure UX upgrade, no data migration needed.

## Progress

### Foundation
- [x] React app scaffold (shared with Learning/Remote)
- [x] Firebase SDK setup (shared)
- [x] Repository pattern (`FirestoreLayoutRepository`, `InMemoryLayoutRepository`)
- [x] `useLayout` hook (add/update/remove buttons, grid resize, save/reset)

### Components
- [x] **DesignerPage** (`DesignerPage.tsx`)
  - [x] Click-to-place grid (configurable rows × cols, default 3×4)
  - [x] Grid size selector in toolbar
  - [x] Save/cancel actions with "✓ Saved" feedback
  - [x] Device selector (reuses `DeviceSelector`, without "+ New Device" button)
  - [ ] ~~Drag-and-drop~~ (deferred to Phase 2)
  - [ ] ~~Edit mode toggle~~ (not needed — grid is always editable)
- [x] **ButtonConfigModal** (`ButtonConfigModal.tsx`)
  - [x] Command assignment (dropdown of learned commands)
  - [x] Label input
  - [x] Color selector (preset swatches)
  - [x] Delete button option
  - [ ] ~~Icon picker~~ (deferred to Phase 2)
- [ ] **GridCell** (inlined in DesignerPage — extract if needed)
- [ ] ~~**CommandPicker**~~ (replaced by simple dropdown in ButtonConfigModal)

### Data Layer (TDD)
- [x] Layout validation logic (`layoutValidation.ts`)
- [x] No overlapping buttons
- [x] Positions within bounds
- [x] Grid resize preserves buttons within new bounds
- [x] Default layout creation
- [x] Firestore persistence via `FirestoreLayoutRepository`

### Testing
- [x] Unit tests for validation logic (13 tests)
- [ ] Integration tests for save/load round-trip
- [ ] Playwright E2E tests

**Test count: 13 layout validation tests (part of 46 total)**

## Blockers

~~Requires Learning feature (command library) and Remote feature.~~ **RESOLVED** — all dependencies met.

No current blockers.

## Next Steps

1. **Phase 1.5: Inline Learning in ButtonConfigModal** — add "Learn New Command" option so users can capture IR signals directly while building a layout (reuses `LearningModal` logic)
2. Extract `GridCell` component from `DesignerPage` (optional cleanup)
3. Add integration tests for save/load round-trip
4. Add Playwright E2E tests (click cell, configure button, save layout)
5. Phase 2: Drag-and-drop upgrade (`@dnd-kit/core`, multi-cell spans, Lucide icon picker)
