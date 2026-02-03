# Transmission - ESP32 Implementation

**Purpose:** Process transmission queue and emit IR signals.

## Architecture

```
include/transmission/
└── IIRTransmitter.h         # Hardware abstraction for IR transmitter

src/transmission/
├── ESP32IRTransmitter.cpp   # IRsend wrapper
├── ProtocolEncoders.cpp     # Encode protocols to IR timing
└── QueueProcessor.cpp       # Poll Firestore queue and transmit
```

## Components

### ESP32IRTransmitter (IIRTransmitter)
Wraps the IRremoteESP8266 library transmitter.

**Responsibilities:**
- Configure IR transmitter pin
- Set carrier frequency (38kHz standard)
- Transmit raw timing arrays
- Transmit encoded protocol signals

### ProtocolEncoders
Mirror of decoder functionality - converts command data to IR timing.

**Supported Protocols:**
- NEC encoder
- Samsung encoder  
- Sony encoder
- Raw playback (direct timing array)

**Responsibilities:**
- Convert (protocol, address, commandCode) → raw timing
- Generate correct carrier frequency pulses
- Validate encoding matches decoder output (symmetry tests)

### QueueProcessor
Main transmission loop.

**Responsibilities:**
- Poll Firestore `queue/` collection (~100ms interval)
- Fetch PENDING items in FIFO order
- Load command details via `commandId` reference
- Encode command using appropriate protocol encoder
- Transmit via `ESP32IRTransmitter`
- Update status to SENT or FAILED
- Set `processedAt` timestamp
- Handle retry logic for failures

## Testing Strategy

### Unit Tests (TDD)
```cpp
// test/test_protocol_encoder/
test_nec_encoder_produces_correct_timing()
test_samsung_encoder_produces_correct_timing()
test_sony_encoder_produces_correct_timing()
test_encoder_decoder_symmetry()  // encode → decode → same result
```

### Hardware Tests
Use `platformio.ini` environment `ir_transmitter_test`:
```bash
pio run -e ir_transmitter_test --target upload
```

Validation: Point at TV and verify it responds to transmitted signals.

## Dependencies

- `IRremoteESP8266` - IR signal transmission
- `Firebase-ESP-Client` - Firestore queue polling

## Performance Targets

- **Latency:** <500ms from queue enqueue to IR emission
- **Polling Frequency:** ~100ms
- **Range:** Sufficient for across-room control

## Integration

See `docs/contracts/transmission.md` for Firestore data model and integration flow.
