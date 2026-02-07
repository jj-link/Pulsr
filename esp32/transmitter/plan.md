# Transmitter - ESP32 Implementation

**Purpose:** Process transmission queue and emit IR signals.

## Architecture

```
include/transmitter/
└── IIRTransmitter.h         # Hardware abstraction for IR transmitter

src/transmitter/
├── ESP32IRTransmitter.cpp   # IRsend wrapper
└── ProtocolEncoders.cpp     # Encode protocols to IR timing
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

### Command Dispatch (via RTDB stream)
The ESP32 receives commands directly via the RTDB stream on `/devices/{deviceId}/pendingCommand`.

**Flow:**
1. Web UI writes `pendingCommand` object to RTDB: `{protocol, address, command, bits, timestamp}`
2. RTDB stream callback fires in `FirebaseManager`
3. `FirebaseManager` extracts command fields and invokes the transmit callback
4. `main.cpp` encodes and transmits IR via `ProtocolEncoders` + `ESP32IRTransmitter`
5. ESP32 clears `pendingCommand` from RTDB after transmission

**Advantages over previous Firestore queue approach:**
- ~100ms latency (same as `isLearning` toggle) vs up to 5-30s with polling
- No Firestore reads from ESP32 for command dispatch
- No queue document accumulation or page size issues
- Dramatically simpler code path

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
- `Firebase-ESP-Client` - RTDB streaming

## Performance Targets

- **Latency:** ~100ms from button press to IR emission (RTDB streaming)
- **Communication:** RTDB real-time streaming (SSE) — no polling, no Firestore reads
- **Range:** Sufficient for across-room control

## NeoPixel LED Feedback

The built-in NeoPixel RGB LED provides visual feedback during transmission:

| Color | Meaning |
|-------|---------|
| Purple | Processing queue item (received from web app) |
| Cyan-green | Transmit success (flashes 500ms) |
| Red-orange | Transmit failed (flashes 1s) |
| Dim green | Returns to ready state after flash |

Feedback is delivered via callback from `FirebaseManager` command handler to `main.cpp`.

## Current Status

- **Command dispatch:** Migrating from Firestore queue polling to direct RTDB `pendingCommand` streaming
- **RTDB streaming:** ESP32 streams `/devices/{deviceId}` for `isLearning` (working) and `pendingCommand` (in progress)
- **Web writes:** Button press writes `pendingCommand` object directly to RTDB (in progress)
- **NeoPixel feedback:** Wired via callback to main.cpp LED handler
- **Protocol encoding:** NEC, Samsung, Sony, RAW all working
- **`isLearning` pattern:** Proven ~100ms latency via RTDB streaming — `pendingCommand` replicates this pattern

## Integration

See `docs/contracts/transmitter.md` for Firestore data model and integration flow.
