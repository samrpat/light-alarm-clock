# Changelog

## 2026-11-12 — Phase 0: Requirements & architecture
- Created `/docs`: README, REQUIREMENTS, STATUS, DECISIONS, FIRMWARE, CHANGELOG.
- Defined requirements R1–R33 (must) and N1–N5 (nice), each with a planned test ID.
- Listed hardware assumptions A1–A8 to verify in Phase 1, and risks K1–K10.
- Proposed D-001 (ESP-IDF + LVGL 9), D-002 (alarm path isolated on core 1, no I2C/UI/BLE dependency), D-003 (UTC RTC + POSIX TZ, DST rules), D-004 (A/B CRC'd NVS records), D-005 (OTA-ready partition table), D-006 (watchdogs, supervisor, safe mode), D-007 (dismiss safety + BOOT fallback), D-008 (software volume), D-009 (repo layout), D-010 (Wi-Fi, open).
- Added the Dreamie (helloambient.com) reference. Adopted scenes (R8), ultra-dark/hidden clock (R6), phone-free operation (N5).

## 2026-11-12 — UX concept (pre-Phase 2)
- Added `/mockups/ux-concept.html`: interactive concept of the cross-shaped screen map and four-gesture vocabulary.
- D-011: dropped IMU gestures (R7) because the clock is stationary; dismiss is now hold 2 s; no double-tap.

## 2026-11-12 — Phase 1: Power & wiring
- Phase 0 approved; D-001…D-010 accepted.
- Verified board hardware from Waveshare's reference: pin map, single I2C bus, ES8311 + NS4150B (enable GPIO46), header GPIO16/17/18 + VBUS.
- Added POWER.md and HARDWARE.md.
- D-012 (USB main supply, ring on VBUS, battery as backup, no boost), D-013 (RGBW ring), D-014 (GPIO17 + 74AHCT125), D-015 (no default alarm; 30 min auto-stop; snooze 9 min × 3), D-016 (always-on dim shifting night face), D-017 (US Eastern default), D-018 (git).
- Requirements R12, R17, R29, R30 reworded; assumptions A1–A6 confirmed; risk K1 resolved.
- Project put under git and pushed to github.com/samrpat/light-alarm-clock.
