title: WiFi SSID with Special Characters in ESP32 config.h
category: troubleshooting
keywords: ["wifi", "ssid", "special characters", "apostrophe", "escape", "config.h", "connection error"]

---

## Problem

When setting up WiFi credentials in `esp32/include/config.h`, special characters in the SSID (like apostrophes `'` in "Joseph's wifi") can cause compilation or connection errors.

## Solution

Escape special characters with a backslash `\`:

```cpp
// Before (causes connection error):
#define WIFI_SSID "John Doe's wifi"

// After (works correctly):
#define WIFI_SSID "John Doe\'s wifi"
```

## Common Special Characters That Need Escaping

- **Apostrophe (`'`)**: `\'`
- **Quote (`"`)**: `\"`
- **Backslash (`\`)**: `\\`

## Verification

After making changes:
1. Save `config.h`
2. Upload code to ESP32
3. Check serial monitor for "[WiFi] Connected!" message

## Related

See also: ESP32 Configuration Setup, WiFi Connection Troubleshooting
