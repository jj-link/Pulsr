# Transmitter - ESP32 Implementation

**Purpose:** Process transmission queue and emit IR signals.

## Architecture

```
include/transmitter/
└── IIRTransmitter.h         # Hardware abstraction for IR transmitter

src/transmitter/
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
- Stream Firestore `queue/` collection for new items (replaces polling)
- Process PENDING items in FIFO order
- Load command details via `commandId` reference
- Encode command using appropriate protocol encoder
- Transmit via `ESP32IRTransmitter`
- Update status to SENT or FAILED
- Set `processedAt` timestamp
- Handle retry logic for failures
- Exponential backoff on connection errors (cap 60s, SSL reset after 5 failures)

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

- **Latency:** <500ms from queue enqueue to IR emission (streaming enables near-instant)
- **Communication:** Firestore real-time streaming (SSE) — no polling overhead
- **Range:** Sufficient for across-room control

## NeoPixel LED Feedback

The built-in NeoPixel RGB LED provides visual feedback during transmission:

| Color | Meaning |
|-------|---------|
| Purple | Processing queue item (received from web app) |
| Cyan-green | Transmit success (flashes 500ms) |
| Red-orange | Transmit failed (flashes 1s) |
| Dim green | Returns to ready state after flash |

Feedback is delivered via `TransmissionEventCallback` from `QueueProcessor` to `main.cpp`.

## Current Status

- **QueueProcessor:** Implemented with `listDocuments` polling (migrating to streaming)
- **Status alignment:** Uses lowercase status values (`pending`, `processing`, `completed`, `failed`) matching web app
- **Timestamps:** ISO 8601 format for Firestore REST API compatibility
- **Command loading:** Handles both `stringValue` and `integerValue` for address/command fields
- **NeoPixel feedback:** Wired via callback from QueueProcessor to main.cpp LED handler
- **Error recovery:** Exponential backoff (cap 60s) + SSL reset after 5 consecutive failures
- **Migration planned:** Replace polling with Firestore streaming to reduce reads and improve latency

## Integration

See `docs/contracts/transmitter.md` for Firestore data model and integration flow.
