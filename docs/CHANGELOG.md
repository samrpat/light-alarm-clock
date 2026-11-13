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

## 2026-11-12 — Phase 1 additions
- Owner answers: backup battery yes, ring #2861 warm white, speaker Waveshare Speaker (B) 20 × 30 × 6.8 mm, charge to 4.1 V.
- D-019: halo driver PCB v1 (44 × 28 mm, through-hole). Added `/pcb/halo-driver/` with generator script, Gerbers, drill files, zip and preview. HARDWARE.md §7 describes it.
- D-020: first build on a breadboard (HARDWARE.md §3a); PCB deferred until after the case design.
- D-021: PCB v2 will take USB-C power at the rear, switch the ring directly and feed the board through VBUS (POWER.md §10). New bench check V10.

## 2026-11-12 — Bring-up test
- Installed ESP-IDF v5.5.6 (`~/esp/esp-idf`), cmake and ninja on the development Mac.
- Added `/firmware/bringup`: screen + touch, ring sunrise on GPIO17 (300 mA cap), white noise and tone. Compiles; not yet run on hardware.
- D-022: toolchain pinned, Waveshare board package adopted (uses esp_lvgl_adapter). Bench check V1 settled from the package's pin definitions (MCLK = GPIO42).
- Bring-up test flashed and booted on the real board: display, touch, all 7 I2C chips, ring driver and speaker codec initialised without errors. Visual and audio results still to be confirmed by the owner.
- Owner confirmed the bring-up test works on the hardware: touch, ring (GRBW order, direct 3.3 V data at short range), white noise and tone.
