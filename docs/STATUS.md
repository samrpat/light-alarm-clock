# Status

Last updated: 2026-11-13

| Phase | Name | State |
|-------|------|-------|
| 0 | Requirements & architecture | ✅ Done, approved 2026-11-12 |
| 1 | Power & wiring | ✅ Done, approved 2026-11-13 |
| 2 | UX & gesture design | ✅ Revised after owner feedback, **awaiting approval** |
| 3 | Core firmware (MVP) | ⏳ Next |
| 4 | 3D-printed enclosure | — |
| 5 | Sleep sounds library | — |
| 6 | BLE settings sync | — |
| 7 | Polish (OTA, power, assembly guide) | — |

## Phase 2 revision (2026-11-13)
- No rim or dial gesture. Three gestures: tap, swipe, hold. Values change by tap zones (D-026).
- Ubuntu type; automatic text-overlap check passes on 188 screen states (D-028).
- Day and night look on a schedule (D-027). 12-hour AM/PM.
- Hardware extras optional; firmware must run without them (D-025).
- Concept page rebuilt: https://claude.ai/artifact/UGtnZ6MqkrSfuM5BShyG5a (private link).

## Hardware state (bring-up test, 2026-11-12)
- `/firmware/bringup` runs on the real board: screen, touch, 7 I2C chips, ring (GRBW, smooth dithered sunrise), speaker.
- Ring ran with a direct 3.3 V data line, short wires and a 300 mA cap.
- Not yet tested: battery, full 0.9 A ring current with fuse + capacitor + resistor, USB voltage under load.

## Waiting for owner
1. **Approve Phase 2** (accepts D-024 as amended, D-025, D-026, D-027, D-028).
2. **12-hour AM/PM** with 5-minute steps: OK? (Exact minutes can come from the phone in Phase 6.)
3. **Day "vibe":** beyond colour, brightness and the date, what should the day look have?
4. **Feel test:** want the tap-zone picker (and the rim idea) added to the bring-up app so you can try them on the real screen?
5. **Battery:** ordered? Check the plug polarity with a multimeter before connecting it.

## Blockers
- None for Phase 3. Bench checks needed along the way: battery polarity (V2), RTC keeps time on battery (V6), PWR-key long-press can be disabled (V7), ring at 0.9 A for 30 min with the protection parts, night dimness banding (UX-B1), touch reliability near the edge (UX-B2).

## Next (Phase 3 plan)
Real firmware in `/firmware/clock`, in small testable steps: time and schedule logic with host tests, NVS A/B storage, alarm engine on core 1, light engine (sunrise, dithering, current cap), audio (noise, fades, escalation), the UI from UX.md with Ubuntu bitmap fonts and the layout check, hardware profile (D-025), watchdogs and safe mode. Also RELIABILITY.md and TEST_PLAN.md.
