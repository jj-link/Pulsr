# Learning - Web UI Status

**Last Updated:** 2026-02-06  
**Phase:** Complete (v1 — Core Functionality)

## Progress

### Foundation
- [x] React app scaffold
- [x] Firebase SDK setup
- [x] Firestore security rules
- [x] Repository pattern with Firestore & InMemory implementations
- [x] `pendingSignal` flow (ESP32 writes captured signal → web reads & saves)

### Components (Built with TDD)
- [x] **LearningModal** (6 passing tests)
  - [x] State management (waiting/success/timeout)
  - [x] Real-time `pendingSignal` listener
  - [x] Cancel action / clear pending signal
  - [x] Command naming on capture
- [x] **CommandList** (5 passing tests)
  - [x] Display commands with protocol info
  - [x] Edit command (rename)
  - [x] Delete command action
  - [ ] Search/filter functionality (deferred — v2)
- [x] **DeviceSelector** (5 passing tests)
  - [x] Device list dropdown
  - [x] `onCreateDevice` optional (hidden on Designer page)
- [x] **CreateDeviceModal** (6 passing tests)
  - [x] Device name + device ID input
  - [x] Creates device in Firestore

### Hooks (Built with TDD)
- [x] **useCommands** (4 passing tests)
  - [x] Load commands by device
  - [x] Real-time updates via subscribe
  - [x] Delete command
- [x] **useDevices** (5 passing tests)
  - [x] Load all devices
  - [x] Create device (name + deviceId + ownerId)
  - [x] Set learning mode
  - [x] Delete device
  - [x] Real-time updates via subscribe

### Testing
- [x] Unit tests for hooks (9 tests)
- [x] Component tests (22 tests)
- [x] Repository pattern enables isolated testing
- [ ] Playwright E2E tests (deferred)

**Test count: 31 learning tests (part of 46 total)**

## Blockers

~~Requires ESP32 Receiver track.~~ **RESOLVED** — ESP32 Firestore integration complete and verified end-to-end.

No current blockers.

## Next Steps

1. **Reuse LearningModal in Designer's ButtonConfigModal** — "Learn New Command" inline option (Phase 1.5 of Designer plan)
2. Search/filter functionality for command list (v2)
3. Add Playwright E2E tests (learn command, rename, delete)
4. Consider shared device context with Remote/Designer via URL params

## Device Pairing Status

### Current Approach
- **Developer mode:** Device ID manually set in ESP32 `config.h` and matched in web UI when creating a device
- This is working end-to-end for development

### Future: Consumer Pairing
- **WiFi Hotspot:** ESP32 creates "Pulsr-Setup" hotspot, user connects to 192.168.4.1 for pairing
- **PIN Code:** ESP32 displays code via LED/serial, user enters when creating device
- **Decision:** WiFi hotspot pairing planned for consumer release
