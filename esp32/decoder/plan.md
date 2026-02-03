# Decoder - ESP32 Implementation

**Purpose:** Capture and decode IR signals from physical remotes.

## Architecture

```
include/decoder/
├── ISignalCapture.h        # Hardware abstraction for IR receiver
└── IProtocolDecoder.h      # Protocol decoding abstraction

src/decoder/
├── ESP32SignalCapture.cpp  # IRrecv wrapper
├── IRLibProtocolDecoder.cpp # Protocol decoders (NEC, Samsung, Sony)
└── LearningStateMachine.cpp # Learning mode logic
```

## Components

### ESP32SignalCapture (ISignalCapture)
Wraps the IRremoteESP8266 library receiver.

**Responsibilities:**
- Configure IR receiver pin
- Enable/disable receiver
- Check for signal availability
- Decode raw IR pulses

### IRLibProtocolDecoder (IProtocolDecoder)
Identifies and decodes known protocols.

**Supported Protocols:**
- NEC (32-bit)
- Samsung (32-bit)
- Sony (12/15/20-bit)
- Raw (fallback)

**Responsibilities:**
- Parse `decode_results` from IRrecv
- Extract protocol, address, command code
- Fall back to raw timing if unknown
- Return structured `DecodedSignal`

### LearningStateMachine
Manages learning mode state and timeouts.

**States:**
- IDLE: Normal operation (listening for queue)
- LEARNING: Waiting for IR signal
- CAPTURED: Signal received, uploading to Firestore
- TIMEOUT: No signal after 30 seconds

**Responsibilities:**
- Listen for Firestore `device.isLearning` changes
- Transition to LEARNING state
- Capture one signal
- Upload to Firestore via Firebase ESP32 SDK
- Return to IDLE

## Testing Strategy

### Unit Tests (TDD)
```cpp
// test/test_protocol_decoder/
test_nec_protocol_decodes_correctly()
test_samsung_protocol_decodes_correctly()
test_sony_protocol_decodes_correctly()
test_unknown_protocol_returns_raw()
test_decoder_round_trip_symmetry()
```

### Hardware Tests
Use `platformio.ini` environment `ir_receiver_test`:
```bash
pio run -e ir_receiver_test --target upload
```

See `src/hardware_tests/ir_decoder_test.cpp` for validation script.

## Dependencies

- `IRremoteESP8266` - IR signal capture and decoding
- `Firebase-ESP-Client` - Firestore integration
- `Adafruit_NeoPixel` - Status LED

## Integration

See `docs/contracts/decoder.md` for Firestore data model and integration flow.
