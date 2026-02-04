# Receiver - ESP32 Status

**Last Updated:** 2026-02-03  
**Phase:** Core Classes Implementation

## Progress

### Foundation
- [x] Interface abstractions defined
- [x] Hardware test script working (`ir_receiver_test`)
- [x] Production firmware scaffold

### Testing Infrastructure
- [x] Native testing framework configured (runs on desktop, ~1 second)
- [x] Mock Arduino types for native tests (`test/mock_arduino.h`)
- [x] Test fixtures with known IR signal data
- [ ] Mock ISignalCapture for tests
- **Testing Strategy:** Native tests for fast TDD, hardware tests for integration verification

### Core Classes (TDD)
- [x] **IRLibProtocolDecoder**
  - [x] NEC protocol decoder + tests
  - [x] Samsung protocol decoder + tests
  - [x] Sony protocol decoder + tests
  - [x] Raw fallback + tests
- [x] **ESP32SignalCapture**
  - [x] IRrecv wrapper implementation
  - [x] Hardware tested via `ir_decoder_test`
- [x] **LearningStateMachine**
  - [x] State transition logic (IDLE → LEARNING → CAPTURED/TIMEOUT → IDLE)
  - [x] Configurable timeout (default 30 seconds)
  - [x] Callback system for state changes and signal capture
  - [ ] Firestore state listener integration (requires Firebase SDK)

### Firebase Integration
- [ ] Firebase ESP32 SDK setup
- [ ] WiFi configuration
- [ ] Device authentication
- [ ] Upload commands to Firestore
- [ ] Listen for `isLearning` state changes

## Blockers

None

## Next Steps

1. Configure Unity testing framework in `platformio.ini`
2. Create `test/test_protocol_decoder/` directory
3. Write first failing test for NEC decoder
4. Implement `IRLibProtocolDecoder::decodeNEC()` to pass test
5. Iterate on Samsung and Sony protocols
