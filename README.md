# Pulsr

A smart universal remote system built with ESP32, Firebase, and React.

## Documentation

- [System Architecture](ARCHITECTURE.md) - System overview and design principles

### Embedded Software
- [Receiver](esp32/receiver/plan.md) - IR signal learning
- [Transmitter](esp32/transmitter/plan.md) - IR signal transmission

### Web App
- [Learning UI](web/features/learning/plan.md) - Learning mode interface
- [Remote UI](web/features/remote/plan.md) - Remote control buttons
- [Designer UI](web/features/designer/plan.md) - Layout editor
- [Chatbot UI](web/features/chatbot/plan.md) - AI assistant

## Getting Started

### Prerequisites

- [VS Code](https://code.visualstudio.com/) with [PlatformIO extension](https://platformio.org/install/ide?install=vscode)
- ESP32-S3-DevKitM-1 (or compatible ESP32-S3 board)
- IR receiver module (TSOP38238) on GPIO 5
- IR LED transmitter circuit on GPIO 4

### ESP32 Configuration

1. Copy the example config:
   ```bash
   cp esp32/include/config.example.h esp32/include/config.h
   ```
2. Edit `esp32/include/config.h` with your WiFi and Firebase credentials

### Hardware Tests

Hardware tests validate individual components before running the full production build. Run them from the `esp32/` directory.

> **Important:** Close any open Serial Monitor before uploading. The upload will fail if the COM port is busy.

#### IR Loopback Test (recommended first test)

Sends IR signals from the transmitter and verifies the receiver decodes them correctly. Tests the full encode → transmit → receive → decode pipeline.

**Setup:** Point IR LED (GPIO 4) at IR receiver (GPIO 5), a few cm apart.

```bash
cd esp32
pio run -e ir_loopback_test -t upload
pio device monitor
```

Press **any key** in the serial monitor to run all tests. Results print automatically. Press any key again to re-run.

#### IR Receiver Test

Captures and decodes IR signals from any remote control.

```bash
pio run -e ir_receiver_test -t upload
pio device monitor
```

Point any TV/AC remote at the IR receiver and press buttons. Decoded signals appear in the serial output.

#### IR Transmitter Test

Sends predefined IR commands to control a real device.

```bash
pio run -e ir_transmitter_test -t upload
pio device monitor
```

Point the IR LED at a TV/device. Press BOOT to cycle through NEC → Samsung → Sony test signals.

### Production Build

Builds and uploads the full embedded software with receiver, transmitter, and Firebase integration.

```bash
cd esp32
pio run -e production -t upload
pio device monitor
```

### Unit Tests

Run protocol encoder/decoder unit tests natively (no hardware needed):

```bash
cd esp32
pio test -e native_test
```
