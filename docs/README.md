# Smart Portable Bedside Alarm Clock

A battery-backed bedside clock built on a **Waveshare ESP32-S3-Touch-AMOLED-1.75**, with three jobs:
1. **Never miss an alarm.** Per-weekday schedule plus one-off overrides, a fault-isolated alarm engine, watchdogs and missed-alarm catch-up.
2. **Wake gently.** A 24-LED NeoPixel halo plays a sunrise, then sound escalates softly → loud.
3. **Fall asleep.** White noise with a sleep timer that fades out before the sunrise starts.

It is controlled by touch on a 1.75" round screen, needs no network or phone, and is built to run for decades (LONGEVITY.md).
Reference product for feel and features: Dreamie by Hello Ambient (helloambient.com). Differences are in [REQUIREMENTS.md](REQUIREMENTS.md#reference-product-dreamie-helloambientcom).

## Current phase
**Phase 3: Core firmware.** In progress: the tested core logic is done; hardware integration and the UI follow. Phases 0, 1 and 2 are approved. Details in [STATUS.md](STATUS.md).

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
| [UX.md](UX.md) | Gestures, options compared, screen map, day/night look, type and layout rules, acceptance checks | Phase 2 |
| [LIGHT.md](LIGHT.md) | The evidence and the ring and screen light plan: dawn, evening, night | Phase 2 |
| [SOUND.md](SOUND.md) | The eleven sleep sounds and how they are made | Phase 2 |
| [BLUETOOTH.md](BLUETOOTH.md) | Optional phone time sync, how it works and the phone-side options | Phase 2 |
| [LONGEVITY.md](LONGEVITY.md) | Long-life, offline plan: time, daylight saving, wear, updates | Phase 2 |
| `licenses/` | Third-party licence texts (Ubuntu Font Licence) | Phase 2 |
| [RELIABILITY.md](RELIABILITY.md) | What happens when a part fails, and the watchdog design | Phase 3 |
| [TEST_PLAN.md](TEST_PLAN.md) | Test cases T-xx ↔ requirements, with results | Phase 3 |
| ENCLOSURE.md | Enclosure design, print and assembly | Phase 4 |

Interface concept: `/mockups/ux-concept.html` (open in a browser; also published as a private page, link in UX.md). PCB: `/pcb/halo-driver`. Code: `/firmware` (`bringup` and `dialtest` hardware test apps now, the product firmware from Phase 3) · CAD: `/enclosure` (from Phase 4).

## Build & flash
The product firmware is in `/firmware/clock` (Phase 3). Host tests: `cd firmware/clock/test && make test`. Build: `cd firmware/clock && idf.py build`. A hardware **bring-up test** exists now in `/firmware/bringup` (see its README).
Toolchain: ESP-IDF **v5.5.6**, installed at `~/esp/esp-idf`.
```bash
. ~/esp/esp-idf/export.sh
```
```bash
cd firmware/bringup && idf.py flash monitor
```

## Picking up this project (for a new person or AI chat)
Read this file, then [STATUS.md](STATUS.md), then [DECISIONS.md](DECISIONS.md). Stay consistent with all three.
Work in phases. Stop for owner approval at the end of each phase, and update every affected doc and [CHANGELOG.md](CHANGELOG.md).
