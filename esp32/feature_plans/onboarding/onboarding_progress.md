# Onboarding - AP Provisioning Progress

**Last Updated:** 2026-02-14
**Current Phase:** Phase 4 Complete (Security Hardening)

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
- [x] Add `POST /claims/create`
- [x] Add `POST /claims/redeem`
- [x] Persist one-time claims with TTL and consumed state
- [x] Bind `deviceId` to `ownerId` transactionally

### Phase 3 - Web Onboarding UX
- [x] Add `Add Device` claim-code flow
- [x] Show onboarding steps and claim status (`waiting`, `claimed`, `expired`, `failed`)
- [x] Refresh owned device list after successful claim

### Phase 4 - Security Hardening
- [x] Replace permissive Firestore rules
- [x] Restrict user access to owned devices only
- [x] Restrict device access to its own RTDB/Firestore paths

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
Onboarding feature is complete. All phases finished:
1. ESP32 firmware (NVS + AP portal)
2. Backend claim API
3. Web onboarding UI
4. Security hardening