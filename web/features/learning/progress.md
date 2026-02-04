# Learning - Web UI Status

**Last Updated:** 2026-02-02  
**Phase:** Not Started

## Progress

### Foundation
- [ ] React app scaffold
- [ ] Firebase SDK setup
- [ ] Firestore security rules

### Components
- [ ] **LearningModal**
  - [ ] State management (waiting/success/timeout)
  - [ ] Real-time command listener
  - [ ] Command naming form
  - [ ] Cancel action
- [ ] **CommandList**
  - [ ] Display commands with protocol info
  - [ ] Delete command action
  - [ ] Search/filter functionality
- [ ] **DeviceSelector**
  - [ ] Device list dropdown
  - [ ] Create new device

### Testing
- [ ] Unit tests for state logic
- [ ] Integration tests with mock Firestore
- [ ] Playwright E2E tests

## Blockers

Requires ESP32 Receiver track to implement Firestore integration first.

## Next Steps

1. Wait for ESP32 Firebase integration
2. Create React app scaffold
3. Implement LearningModal with TDD
4. Add real-time Firestore listeners
