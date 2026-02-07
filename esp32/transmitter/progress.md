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
- [x] **QueueProcessor** (legacy — being replaced by RTDB direct dispatch)
  - [x] Firestore polling logic (legacy)
  - [x] FIFO queue ordering (legacy)
  - [x] Status updates (legacy)
  - [x] Command loading from Firestore (legacy)
  - [x] Encoding and transmission integration
  - [x] Exponential backoff on errors (legacy)
- [ ] **RTDB Command Dispatch** (replacing QueueProcessor)
  - [ ] Handle `pendingCommand` in RTDB stream callback
  - [ ] Extract protocol/address/command/bits from stream data
  - [ ] Encode and transmit IR via existing ProtocolEncoders + ESP32IRTransmitter
  - [ ] Clear `pendingCommand` from RTDB after transmission
  - [ ] Remove QueueProcessor and Firestore queue dependencies

### Firebase Integration
- [x] RTDB streaming for `isLearning` (working, ~100ms latency)
- [ ] RTDB streaming for `pendingCommand` (in progress)
- [ ] Remove Firestore queue reads/writes from ESP32
- [ ] Latency optimization testing (~100ms target)

## Blockers

None.

## Next Steps

1. **Implement RTDB `pendingCommand` dispatch** — replicate `isLearning` pattern for commands
2. **Remove QueueProcessor** and all Firestore queue polling code
3. **Update web remote** to write `pendingCommand` to RTDB instead of Firestore queue
4. Create `ir_transmitter_test` hardware validation script
5. Latency optimization testing (~100ms target)
