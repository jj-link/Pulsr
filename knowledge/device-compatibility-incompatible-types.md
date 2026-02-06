title: Device Compatibility - Incompatible Remote Types
category: hardware
tags: ["ir", "remote", "compatibility", "limitations", "bluetooth", "rf", "wifi"]

---

## Hardware Capability

The Pulsr ESP32 device contains **only**:
- IR (infrared) LED transmitter
- IR receiver module

This means it can only interact with devices that use **infrared remote control**.

## Incompatible Device Types

The following remote types **will NOT work** with Pulsr:

| Technology | Examples | Why It Won't Work |
|------------|----------|-------------------|
| **Bluetooth** | Apple TV (newer), Fire TV Stick, game controllers | No Bluetooth radio |
| **RF (Radio Frequency)** | RF outlets, ceiling fans, garage doors (433MHz/315MHz) | No RF transceiver |
| **WiFi Direct** | Some smart remotes, WiFi HID devices | No WiFi peripheral mode |
| **Zigbee/Z-Wave** | Smart home hubs, Zigbee remotes | No mesh network radio |
| **Proprietary 2.4GHz** | Logitech Unifying receivers, some gaming peripherals | Custom protocols, no compatible radio |

## Compatible Device Examples

Devices that typically use IR and **will work**:
- TVs (Samsung, LG, Sony, Vizio, etc.)
- Audio receivers and soundbars
- Air conditioners
- DVD/Blu-ray players
- Streaming boxes with IR (older Roku, some cable boxes)

## How to Check Your Device

1. **Look at the remote**: Does it have an LED bulb on the front? IR remotes have a visible LED.
2. **Phone camera test**: Point remote at phone camera, press a button. IR appears as purple/white flashing light on camera.
3. **Check specifications**: Look for "Infrared" or "IR" in device manual/website.

## What This Means for Users

- Pulsr cannot control Bluetooth-only streaming sticks (newer Fire TV, Chromecast without IR)
- Cannot learn or transmit RF-based smart plugs/fans
- Cannot integrate with Zigbee/Z-Wave smart home ecosystems directly
- IR has line-of-sight requirement (must be in same room, no walls blocking)

## Future Expansion

To add support for other protocols would require additional hardware modules:
- Bluetooth LE module for BLE remotes
- RF transceiver (CC1101, etc.) for 433MHz/315MHz devices
- Zigbee coordinator for Zigbee devices

These are not included in the current hardware design.
