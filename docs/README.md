# Smart Portable Bedside Alarm Clock

A battery-backed bedside clock built on a **Waveshare ESP32-S3-Touch-AMOLED-1.75**, with three jobs:
1. **Never miss an alarm.** Per-weekday schedule plus one-off overrides, a fault-isolated alarm engine, watchdogs and missed-alarm catch-up.
2. **Wake gently.** A 24-LED NeoPixel halo plays a sunrise, then sound escalates softly → loud.
3. **Fall asleep.** White noise with a sleep timer that fades out before the sunrise starts.

It is controlled with gestures on a 1.75" round touchscreen. The phone is optional (BLE settings sync in Phase 6).
Reference product for feel and features: Dreamie by Hello Ambient (helloambient.com). Differences are in [REQUIREMENTS.md](REQUIREMENTS.md#reference-product-dreamie-helloambientcom).

## Current phase
**Phase 1: Power & wiring.** Done, waiting for owner approval. Phase 0 is approved. Details in [STATUS.md](STATUS.md).

## Hardware (fixed)
- Waveshare ESP32-S3-Touch-AMOLED-1.75: ESP32-S3R8, 466×466 CO5300 AMOLED, CST9217 touch, QMI8658 IMU, PCF85063 RTC, AXP2101 PMIC, audio codec + ES7210 mics, TF slot
- Adafruit NeoPixel Ring 24× **RGBW** (#2861/2862/2863)
- Owner's speaker, 8 Ω 2 W
- Power: USB-C 5 V 3 A, with a 2000 mAh Li-ion backup cell (recommended)

## Documents
| File | Contents | Since |
|------|----------|-------|
| [REQUIREMENTS.md](REQUIREMENTS.md) | Requirement checklist (R1…, N1…), assumptions, risks | Phase 0 |
| [STATUS.md](STATUS.md) | Phase tracker, blockers, what waits for approval | Phase 0 |
| [DECISIONS.md](DECISIONS.md) | Decision log (D-001…) | Phase 0 |
| [FIRMWARE.md](FIRMWARE.md) | Block diagram, RTOS tasks, alarm state machine (+ modules/data model in Phase 3) | Phase 0 |
| [CHANGELOG.md](CHANGELOG.md) | Dated change history | Phase 0 |
| [HARDWARE.md](HARDWARE.md) | BOM, pin table, wiring, speaker, bench checks | Phase 1 |
| [POWER.md](POWER.md) | Power design, current budget, runtime math | Phase 1 |
| UX.md | Screen map, gestures, wireframes | Phase 2 |
| RELIABILITY.md | FMEA, watchdog design | Phase 3 |
| TEST_PLAN.md | Test cases T-xx ↔ requirements | Phase 3 |
| ENCLOSURE.md | Enclosure design, print and assembly | Phase 4 |

Interface concept: `/mockups/ux-concept.html` (open in a browser). Code: `/firmware` (ESP-IDF project, from Phase 3) · CAD: `/enclosure` (from Phase 4).

## Build & flash
Not yet available (firmware starts in Phase 3). The planned toolchain is ESP-IDF v5.x:
```bash
cd firmware && idf.py set-target esp32s3 && idf.py build flash monitor
```

## Picking up this project (for a new person or AI chat)
Read this file, then [STATUS.md](STATUS.md), then [DECISIONS.md](DECISIONS.md). Stay consistent with all three.
Work in phases. Stop for owner approval at the end of each phase, and update every affected doc and [CHANGELOG.md](CHANGELOG.md).
