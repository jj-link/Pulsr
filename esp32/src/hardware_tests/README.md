# Hardware Test Scripts

This directory contains standalone test programs for validating hardware functionality without the full production architecture.

## Purpose

These scripts are **not part of the production firmware**. They serve as:

- **Hardware validation tools** - Verify IR receiver/transmitter, LEDs, and other components work correctly
- **Library examples** - Simple demonstrations of how to use the IRremoteESP8266 and NeoPixel libraries
- **Debugging aids** - Quick scripts to test specific hardware issues
- **Documentation** - Reference implementations showing basic hardware interaction

## Available Tests

### `ir_decoder_test.cpp`

**What it does:**
- Listens for IR signals from any remote control
- Decodes known protocols (NEC, Samsung, Sony, etc.)
- Captures raw timing data for unknown protocols
- Provides visual feedback via NeoPixel LED

**How to use:**
1. Build and upload: `pio run -e ir_receiver_test --target upload`
2. Open serial monitor: `pio device monitor`
3. Type `L` to enter learning mode
4. Point a remote at the IR receiver and press a button
5. View the decoded signal data in the serial output

**When to use:**
- Verifying IR receiver is wired correctly
- Testing if specific remotes are compatible
- Capturing timing data for protocol development
- Debugging signal reception issues

### `ir_transmitter_test.cpp` (TODO)

**What it will do:**
- Test IR LED transmission
- Verify signal strength and range
- Test different protocols and carrier frequencies
- Validate timing accuracy

**Status:** Not yet implemented. Will be created when transmission features are added.

## Building Test Scripts

Test scripts use separate PlatformIO environments to avoid conflicts with production firmware.

```bash
# Build IR receiver test
pio run -e ir_receiver_test

# Upload and monitor
pio run -e ir_receiver_test --target upload
pio device monitor

# Future: IR transmitter test (when implemented)
# pio run -e ir_transmitter_test --target upload
```

## Note

These scripts have **tight coupling** to hardware and are **not unit testable**. This is intentional - they prioritize simplicity and directness for testing purposes. The production firmware (in `src/main.cpp`) uses proper abstractions and loose coupling.
