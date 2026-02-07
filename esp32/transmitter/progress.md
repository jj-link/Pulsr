# Transmitter - ESP32 Status

**Last Updated:** 2026-02-07  
**Phase:** Streaming Migration

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
- [x] **QueueProcessor**
  - [x] Firestore polling logic (legacy, being replaced by streaming)
  - [x] FIFO queue ordering
  - [x] Status updates (PENDING → PROCESSING → SENT/FAILED)
  - [x] Command loading from Firestore
  - [x] Encoding and transmission integration
  - [x] Production build verified (17.1% Flash, 14.8% RAM)
  - [x] Exponential backoff on errors (cap 60s, SSL reset after 5 failures)
  - [ ] Migrate to Firestore real-time streaming (replace polling)

### Firebase Integration
- [x] Queue collection polling (legacy, being replaced by streaming)
- [x] Command reference loading via commandId
- [x] Status update writes with timestamps
- [ ] Migrate to Firestore real-time streaming for queue
- [ ] Latency optimization testing (<500ms target)

## Blockers

None - core transmitter implementation complete.

## Next Steps

1. **Replace polling with Firestore streaming** for queue collection
   - Eliminates ~43,200 reads/day per device from 2s polling
   - Enables near-instant transmission (currently up to 2s delay)
2. Create `ir_transmitter_test` hardware validation script
3. Latency optimization testing (<500ms target)
