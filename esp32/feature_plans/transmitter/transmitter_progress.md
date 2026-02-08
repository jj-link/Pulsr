# Transmitter - ESP32 Status

**Last Updated:** 2026-02-07  
**Phase:** Native Sender Migration

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
  - [x] Clear `pendingCommand` from RTDB after transmission
  - [x] Removed QueueProcessor and Firestore queue code
- [ ] **Native Sender Migration**
  - [ ] Update `PendingCommand` struct: replace `address`/`command` with `value` (uint64_t)
  - [ ] Update `FirebaseManager` RTDB parsing: read `value` instead of `address`/`command`
  - [ ] Update `onCommandReceived`: dispatch to native senders (`transmitSamsung`/`transmitNEC`/`transmitSony`) with raw `value`
  - [ ] Remove custom `ProtocolEncoders` from production transmit path
  - [x] Hardware test: verify Samsung TV responds to native sender (confirmed 2026-02-07)

### Firebase Integration
- [x] RTDB streaming for `isLearning` (working, ~100ms latency)
- [x] RTDB streaming for `pendingCommand` (working, ~100ms latency)
- [x] Removed Firestore queue reads/writes from ESP32

### Web Integration (cross-cutting)
- [ ] Add `value` and `bits` fields to `IRCommand` type
- [ ] Persist `value` and `bits` when saving commands from `pendingSignal`
- [ ] Send raw `value` (not `address`/`command`) in RTDB `pendingCommand`
- [ ] Clear old Firestore command data (stale address/command-only records)

## Blockers

None.

## Next Steps

1. Implement native sender migration (ESP32 + web code changes)
2. Clear Firestore, re-learn Samsung Power with corrected data model
3. Validate full loop: web button → RTDB → ESP32 native sender → TV responds
4. Monitor WiFi stability in long-running sessions
