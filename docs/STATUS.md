# Status

Last updated: 2026-11-13

| Phase | Name | State |
|-------|------|-------|
| 0 | Requirements & architecture | ✅ Done, approved 2026-11-12 |
| 1 | Power & wiring | ✅ Done, approved 2026-11-13 |
| 2 | UX & gesture design | ✅ Done, **awaiting approval** |
| 3 | Core firmware (MVP) | ⏳ Next |
| 4 | 3D-printed enclosure | — |
| 5 | Sleep sounds library | — |
| 6 | BLE settings sync | — |
| 7 | Polish (OTA, power, assembly guide) | — |

## Done in Phase 2
- [UX.md](UX.md): principles, touch maths for the 44.5 mm screen, four-gesture vocabulary, screen map, wireframes for every screen and state, settings (8 pages), first-run flow, gesture counts, night theme, status-line priorities, acceptance checks.
- Key rule (D-024): anything that can stop an alarm needs a hold; anything that only makes it ring is a tap.
- Concept page updated: https://claude.ai/artifact/UGtnZ6MqkrSfuM5BShyG5a (private link). It shows the clock, Tomorrow, Week, Day, Sound, Light and Ringing. Settings, first run, warnings and the sunrise screen are wireframes only.
- Phase 1 accepted: D-012, D-014 (amended), D-019 (in principle).
- Fewest-parts rule proposed (D-023): level shifter, PCB and PCB v2 left out of the baseline.

## Hardware state (from the bring-up test, 2026-11-12)
- `/firmware/bringup` runs on the real board: screen, touch, all 7 I2C chips, ring (GRBW, 24 LEDs, smooth dithered sunrise), speaker (noise and tone).
- Ring ran with a direct 3.3 V data line, short wires and a 300 mA cap.
- Not yet tested: battery, full 0.9 A ring current with the fuse, capacitor and resistor, USB voltage under load.

## Waiting for owner
1. **Approve Phase 2** (accepts D-024).
2. **D-023 fewest parts:** accept leaving out the level shifter, PCB and PCB v2 for now? (Reversible: add the $1.50 chip if the ring flickers at full current.)
3. **Daytime brightness:** the board has no light sensor, so the face is always very dim and a tap lifts it for 5 s. Fine, or add a Day/Night schedule?
4. **Battery:** ordered? Check the plug polarity with a multimeter before connecting it.

## Blockers
- None for Phase 3 to start. These bench checks are needed along the way: battery polarity (V2), RTC keeps time on battery (V6), PWR-key long-press can be disabled (V7), ring at 0.9 A for 30 min (D-023), night dimness banding (UX-B1).

## Next (Phase 3 plan)
Real firmware in `/firmware/clock`, built in small testable steps: time and schedule logic with host tests (`next_alarm`, DST, overrides), NVS A/B storage, alarm engine on core 1, light engine (sunrise, dithering, current cap), audio (noise, fades, escalation), the UI from UX.md, watchdogs and boot-loop safe mode. Also RELIABILITY.md and TEST_PLAN.md.
