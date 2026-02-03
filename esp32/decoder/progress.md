# Decoder - ESP32 Status

**Last Updated:** 2026-02-02  
**Phase:** Foundation Complete

## Progress

### Foundation
- [x] Interface abstractions defined
- [x] Hardware test script working (`ir_receiver_test`)
- [x] Production firmware scaffold

### Testing Infrastructure
- [ ] Unity testing framework configured
- [ ] Test fixtures with known IR signal data
- [ ] Mock ISignalCapture for tests

### Core Classes (TDD)
- [ ] **IRLibProtocolDecoder**
  - [ ] NEC protocol decoder + tests
  - [ ] Samsung protocol decoder + tests
  - [ ] Sony protocol decoder + tests
  - [ ] Raw fallback + tests
  - [ ] Round-trip symmetry tests
- [ ] **ESP32SignalCapture**
  - [ ] IRrecv wrapper implementation
  - [ ] Hardware tested via `ir_receiver_test`
- [ ] **LearningStateMachine**
  - [ ] State transition logic
  - [ ] 30-second timeout handler
  - [ ] Firestore state listener

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
