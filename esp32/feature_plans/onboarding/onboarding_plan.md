# Onboarding - AP Provisioning Plan

## Objective
Move setup from compile-time `config.h` values to runtime provisioning so multiple ESP32 devices can be onboarded without reflashing.

## End-to-End Flow
1. User signs into web app and clicks `Add Device`.
2. Web app requests claim code from backend.
3. Backend returns a short-lived one-time claim code (format: `PULSR-XXXX`, 4 alphanumeric uppercase).
4. Unprovisioned ESP32 starts AP: `Pulsr-Setup-XXXX` (XXXX = last 4 chars of MAC address) and hosts setup page at `http://192.168.4.1`.
5. User connects to AP and submits home WiFi SSID/password + claim code.
6. ESP32 saves provisioning payload to NVS and reboots.
7. ESP32 joins home WiFi.
8. ESP32 calls backend redeem endpoint with claim code + hardware ID.
9. Backend validates claim, binds `deviceId` to `ownerId`, marks code consumed.
10. ESP32 stores returned identity in NVS and starts normal Firebase streaming on `/devices/{deviceId}`.

## Firmware Work (ESP32)
### Boot Modes
- `UNPROVISIONED`: missing required NVS keys; start AP setup mode.
- `PROVISIONED_UNCLAIMED`: WiFi creds exist; attempt claim redemption.
- `RUNNING`: claim complete; run existing Firebase manager loop.
- `RECOVERY`: repeated failures; return to AP mode.

### NVS Keys (`pulsr`)
- `wifi_ssid`
- `wifi_pass`
- `claim_code`
- `device_id`
- `owner_uid`
- `provisioned`
- `claimed`

### AP Setup Endpoints
- `GET /`: setup form
- `POST /api/provision`: store SSID/password/claim code
- `GET /api/status`: current provisioning state
- `POST /api/reset`: clear NVS and reboot

### Claim Redemption
- Request body: `claimCode`, `hardwareId`, optional `deviceName`.
- Response: `deviceId`, `ownerId`, claim status.
- On success: clear `claim_code`, set `claimed=true`.

### Factory Reset
- Provide one trigger path (button hold or serial command).
- Clear all onboarding keys and reboot to AP mode.

## Backend Work
### Claim Endpoints
- `POST /claims/create` (authenticated web user): returns claim code + expiration.
- `POST /claims/redeem` (ESP32): validates and consumes claim.

### Firestore Data
`deviceClaims/{claimId}`
- `code`, `ownerId`, `status`, `expiresAt`, `consumedAt`, `hardwareId`, `deviceId`

`devices/{deviceId}`
- `ownerId`, `name`, `isLearning`, `lastSeenAt`

## Web Work
- Add `Add Device` flow to generate claim code.
- Show onboarding instructions for AP connection + local portal.
- Show claim status (`waiting`, `claimed`, `expired`, `failed`).
- Refresh owned device list after claim success.

## Security Work
- Remove permissive rules (`allow read, write: if true`).
- Restrict user access to owned devices only.
- Restrict device access to its own node/document only.
- Enforce claim TTL + one-time consumption + basic rate limiting.

## Phases
1. Firmware NVS + AP portal.
2. Backend claim create/redeem endpoints.
3. Web claim-code onboarding UI.
4. Security rules hardening.

## Acceptance Criteria
1. Fresh ESP32 with empty NVS always starts AP setup mode.
2. Valid setup submission stores config and reboots.
3. Valid claim code binds device to correct user account.
4. Used/expired claim codes are rejected with clear error.
5. Claimed device appears in owner's device list and accepts remote commands.
6. Factory reset reliably returns device to AP setup mode.