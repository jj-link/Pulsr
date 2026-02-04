# Transmitter - ESP32 Status

**Last Updated:** 2026-02-04  
**Phase:** Core Classes Implementation (TDD)

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
- [ ] **ESP32IRTransmitter**
  - [ ] IRsend wrapper implementation
  - [ ] Carrier frequency configuration (38kHz)
  - [ ] Hardware tested via `ir_transmitter_test`
- [ ] **QueueProcessor**
  - [ ] Firestore polling logic (~100ms)
  - [ ] FIFO queue ordering
  - [ ] Status updates (PENDING → SENT/FAILED)
  - [ ] Retry logic with backoff

### Firebase Integration
- [ ] Queue collection polling
- [ ] Command reference loading
- [ ] Status update writes
- [ ] Latency optimization (<500ms)

## Blockers

**Dependency:** Requires Decoder track to complete Firebase integration - needs learned commands in database to transmit.

## Next Steps

1. Wait for Decoder Firebase integration
2. Create `ir_transmitter_test` hardware validation script
3. Implement protocol encoders with TDD (mirror decoder tests)
4. Build queue processor with polling logic
