# Transmitter - ESP32 Status

**Last Updated:** 2026-02-07  
**Phase:** RTDB Direct Command Dispatch

## Progress

### Foundation
- [x] Interface abstractions defined
- [ ] Hardware test script (`ir_transmitter_test`)

### Testing Infrastructure
- [x] Native testing framework configured (shared with receiver)
- [x] Protocol encoder tests with round-trip symmetry validation
- [ ] Mock transmitter for integration tests

### Core Classes (TDD)
- [x] **ProtocolEncoders** (IRLibProtocolEncoders)
  - [x] NEC encoder + tests
  - [x] Samsung encoder + tests
  - [x] Sony encoder + tests
  - [x] RAW passthrough + tests
  - [x] Round-trip symmetry tests (encode → decode → verify all passing)
- [x] **ESP32IRTransmitter**
  - [x] IRsend wrapper implementation
  - [x] Carrier frequency configuration (38kHz)
  - [x] Production build verified (17.1% Flash, 14.8% RAM)
  - [ ] Hardware tested via `ir_transmitter_test`
- [x] **RTDB Command Dispatch**
  - [x] Handle `pendingCommand` in RTDB stream callback
  - [x] Extract protocol/address/command/bits from stream data
  - [x] Encode and transmit IR via ProtocolEncoders + ESP32IRTransmitter
  - [x] Clear `pendingCommand` from RTDB after transmission
  - [x] Removed QueueProcessor and Firestore queue code

### Firebase Integration
- [x] RTDB streaming for `isLearning` (working, ~100ms latency)
- [x] RTDB streaming for `pendingCommand` (working, ~100ms latency)
- [x] Removed Firestore queue reads/writes from ESP32

## Blockers

None.

## Next Steps

1. Create `ir_transmitter_test` hardware validation script
2. Add unit tests for protocol encoders
3. Monitor WiFi stability in long-running sessions
