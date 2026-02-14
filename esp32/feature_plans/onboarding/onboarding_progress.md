# Onboarding - AP Provisioning Progress

**Last Updated:** 2026-02-12
**Current Phase:** Phase 1 Complete (ESP32 Firmware - NVS + AP Portal)

## Status Summary
- AP onboarding + claim code approach is approved.
- Implementation started: Phase 1 - ESP32 Firmware

## Phase Checklist

### Phase 0 - Plan and Docs
- [x] Create onboarding implementation plan
- [x] Create onboarding setup guide
- [x] Define phased rollout and acceptance criteria

### Phase 1 - Firmware (NVS + AP Portal)
- [x] Add NVS storage for provisioning values
- [x] Add boot mode selection (`UNPROVISIONED`, `PROVISIONED_UNCLAIMED`, `RUNNING`, `RECOVERY`)
- [x] Add AP setup mode (`Pulsr-Setup-XXXX`)
- [x] Add setup endpoints: `GET /`, `POST /api/provision`, `GET /api/status`, `POST /api/reset`
- [x] Add factory reset path

### Phase 2 - Backend (Claim API)
- [ ] Add `POST /claims/create`
- [ ] Add `POST /claims/redeem`
- [ ] Persist one-time claims with TTL and consumed state
- [ ] Bind `deviceId` to `ownerId` transactionally

### Phase 3 - Web Onboarding UX
- [ ] Add `Add Device` claim-code flow
- [ ] Show onboarding steps and claim status (`waiting`, `claimed`, `expired`, `failed`)
- [ ] Refresh owned device list after successful claim

### Phase 4 - Security Hardening
- [ ] Replace permissive Firestore rules
- [ ] Restrict user access to owned devices only
- [ ] Restrict device access to its own RTDB/Firestore paths

## Verification Checklist
- [ ] Fresh ESP32 with empty NVS starts AP mode
- [ ] Valid AP form submission stores config and reboots
- [ ] Valid claim code binds device to correct owner
- [ ] Used/expired claim code is rejected with clear error
- [ ] Factory reset returns device to AP mode
- [ ] Claimed device appears in owner list and remote commands work

## Blockers
- None

## Next Action
1. Start Phase 1 by implementing NVS storage + boot mode selection in firmware.