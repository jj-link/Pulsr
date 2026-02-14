# AP Onboarding Setup Guide

**Status:** Draft for planned AP onboarding workflow.  
**Last Updated:** 2026-02-12

This guide describes the target onboarding flow for adding multiple ESP32 units without editing `config.h` per device.

## Prerequisites

1. A Pulsr web account with sign-in enabled.
2. ESP32 flashed with onboarding-capable firmware.
3. Phone or laptop with WiFi.
4. Target home WiFi credentials.

## Setup Flow

### Step 1: Generate a Claim Code

1. Open the Pulsr web app.
2. Go to `Add Device`.
3. Generate a claim code (example: `PULSR-84KQ`).
4. Keep this code available for the setup portal.

### Step 2: Boot the Device in Setup Mode

1. Power the ESP32.
2. If not provisioned, it starts AP mode automatically.
3. Look for a setup network like `Pulsr-Setup-AB12`.

### Step 3: Connect to ESP32 Setup AP

1. Connect phone/laptop to `Pulsr-Setup-AB12`.
2. Open `http://192.168.4.1`.
3. Complete the form:
- Home WiFi SSID
- Home WiFi password
- Claim code
- Optional room/device display name

### Step 4: Provision and Reboot

1. Submit the form.
2. ESP32 stores values in NVS.
3. Device reboots and joins your home WiFi.

### Step 5: Claim and Verify

1. Device redeems claim code with backend.
2. Backend binds device ownership to your account.
3. Device appears in the Pulsr device list.
4. Open Remote view and test a command.

## Recovery and Reset

### If WiFi Credentials Are Wrong

1. Device fails to connect after retry threshold.
2. Device returns to setup AP mode.
3. Reconnect and submit corrected credentials.

### Factory Reset

1. Trigger reset action (button-hold or serial command).
2. Device clears provisioning keys in NVS.
3. Device reboots into AP setup mode.

## Troubleshooting

### Setup Portal Does Not Load

1. Confirm your device is connected to `Pulsr-Setup-*`.
2. Open `http://192.168.4.1` directly.
3. Disable mobile data briefly (some phones avoid captive AP traffic).

### Claim Code Rejected

1. Regenerate a new code in web app.
2. Ensure code has not expired.
3. Ensure code has not already been used.

### Device Does Not Appear in Web App

1. Confirm home WiFi credentials are valid.
2. Check if the claim was consumed.
3. Re-run onboarding with a fresh claim code.

## Current Development Note

Until onboarding firmware ships, setup still uses `esp32/include/config.h`.  
This guide is the target flow that implementation will follow.

