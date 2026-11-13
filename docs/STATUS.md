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
| 6 | Offline update and the long-life kit (was BLE, dropped by D-030) | — |
| 7 | Polish (power, assembly guide) | — |

## Phase 2 revision (2026-11-13)
- **One way to edit.** Four gestures, four jobs: swipe goes, tap does, drag a dot changes, hold stops. No buttons (D-036). The dial lives on dots like clock hands (outer minutes, inner hours). Measured: a rim-band dial hijacked 60 % of sideways swipes; the dot design lost none in 1,249 random swipes.
- **Sideways always means the next neighbour:** Light | Clock | Sound, the days, the settings pages.
- **Settings redesigned (D-037):** eight pages, each a picture of what it sets: a watch face, a calendar page, a band of night on the clock ring for Sleep, a glowing orb, sound waves, a tiny sunrise, a battery ring.
- Fewer words (at most four lines per screen), the face that follows your day, the light plan ([LIGHT.md](LIGHT.md)), the offline and repairable plan ([LONGEVITY.md](LONGEVITY.md), 30 years is a yardstick not a promise).
- Concept page: https://claude.ai/artifact/UGtnZ6MqkrSfuM5BShyG5a (private link). Text check: 283 states, 0 issues.
- Board test app `/firmware/dialtest` (compiles, not yet flashed): feel the dots and check swipes on the real glass.

## Hardware state (bring-up test, 2026-11-12)
- `/firmware/bringup` runs on the real board: screen, touch, 7 I2C chips, ring (GRBW, smooth dithered sunrise), speaker.
- Ring ran with a direct 3.3 V data line, short wires and a 300 mA cap.
- Not yet tested: battery, full 0.9 A ring current with fuse + capacitor + resistor, USB voltage under load.

## Waiting for owner
1. **Approve Phase 2** (accepts D-024 as amended, D-025, D-028, D-030 to D-037).
2. **Flash `/firmware/dialtest`** and try it (swipe near the rim, next to a dot, in the middle; grab a dot with and without a pause). Tell me anything that feels wrong (UX-B2). I could not flash it because the board was not plugged in.
3. **Daylight saving:** US Eastern with automatic rules, a Manual option and a two-touch fix. OK? (not yet answered)
4. **Battery:** order a 503450-class cell by the specification in HARDWARE.md §1 (answered yes).

Answered: 12-hour AM/PM · 5-minute steps · battery spec · no lux meter · drop the phone link and wireless updates (Phase 6 becomes the offline update and the long-life kit) · 30 years is a quality yardstick, not a promise.

## Early enclosure draft (2026-11-13, at the owner's request)
- [ENCLOSURE.md](ENCLOSURE.md) draft and proposed D-029 (revised): pop-out back-panel stand leaning 15° back, made with a 3D printer and laser cutter, all electronics in the head, built for 30 years with replaceable wear parts. Phase 4 has not started. **Option D approved by the owner 2026-11-13.**
- Waiting for the owner: laser cutter materials, front halo or rear wall-wash, colours, board measurements E1–E6.

## Blockers
- None for Phase 3. Bench checks needed along the way: battery polarity (V2), RTC keeps time on battery (V6), PWR-key long-press can be disabled (V7), ring at 0.9 A for 30 min with the protection parts, night dimness banding (UX-B1), touch reliability near the edge (UX-B2).

## Next (Phase 3 plan)
Real firmware in `/firmware/clock`, in small testable steps: time and schedule logic with host tests, NVS A/B storage, alarm engine on core 1, light engine (sunrise, dithering, current cap), audio (noise, fades, escalation), the UI from UX.md with Ubuntu bitmap fonts and the layout check, hardware profile (D-025), watchdogs and safe mode. Also RELIABILITY.md and TEST_PLAN.md.
