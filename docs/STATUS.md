# Status

Last updated: 2026-11-14

| Phase | Name | State |
|-------|------|-------|
| 0 | Requirements & architecture | ✅ Done, approved 2026-11-12 |
| 1 | Power & wiring | ✅ Done, approved 2026-11-13 |
| 2 | UX & gesture design | ✅ Done, approved 2026-11-14 |
| 3 | Core firmware (MVP) | 🔧 In progress: 3a done, 3b-1 builds |
| 4 | 3D-printed enclosure | — |
| 5 | Sleep sounds library | — |
| 6 | Phone time sync over Bluetooth (optional) and the offline update kit | — |
| 7 | Polish (power, assembly guide) | — |

## Phase 3 progress (2026-11-14)
- **3a, done:** the core logic in `firmware/clock/components/clockcore` with 518,574 host checks passing under sanitizers ([TEST_PLAN.md](TEST_PLAN.md)). 1,184 daylight-saving changes (2026-2099, 10 regions) match an independent library; 1,500 random schedules match a brute-force scan; settings survive a power cut at every byte; reboot during dawn or ringing resumes right; 7 injected bugs all caught.
- **3b-1, builds (not flashed):** `firmware/clock` ESP-IDF project: alarm engine on core 1, NVS A/B storage, task watchdog, 64-bit time assert. 0x34310 bytes, 86 % of the app partition free.
- **To do:** 3b-2 RTC task, ring and speaker tasks from the bring-up code; 3b-3 supervisor, boot-loop guard, safe mode, BOOT dismiss, battery rules; 3c the LVGL UI from UX.md with Inter fonts and the layout check on real strings. Then bench tests.
- Reliability notes: [RELIABILITY.md](RELIABILITY.md).

## Phase 2 revision (2026-11-13)
- **One way to edit.** Four gestures, four jobs: swipe goes, tap does, drag a dot changes, hold stops. No buttons (D-036). The dial lives on dots like clock hands (outer minutes, inner hours). Measured: a rim-band dial hijacked 60 % of sideways swipes; the dot design lost none in 1,249 random swipes.
- **Sideways always means the next neighbour:** Light | Clock | Sound, the days, the settings pages.
- **Settings redesigned (D-037):** eight pages, each a picture of what it sets: a watch face, a calendar page, a band of night on the clock ring for Sleep, a glowing orb, sound waves, a tiny sunrise, a battery ring.
- Fewer words (at most four lines per screen), the face that follows your day, the light plan ([LIGHT.md](LIGHT.md)), the offline and repairable plan ([LONGEVITY.md](LONGEVITY.md), 30 years is a yardstick not a promise).
- Concept page: https://claude.ai/artifact/UGtnZ6MqkrSfuM5BShyG5a (private link). Text check: 283 states, 0 issues.
- Board test app `/firmware/dialtest` (compiles, not yet flashed): feel the dots and check swipes on the real glass.

- **Round 4:** a vivid colour language (D-038), one Alarm page with tap-to-test, and bedtime separated from alarm time with no wake-time setting (D-039). Text check: 386 states, 2,299 boxes including the dots, 0 issues. Swipe test re-run: 983 swipes, 0 mistaken.

- **Round 5 (D-040):** bigger clock, small alarm line, gradual dawn on the screen, hold to turn alarms on and off with visible feedback, eleven synthesised sounds ([SOUND.md](SOUND.md)), Lamp rename, Day brightness, Battery page, real mark counts on Region and Date. Checks: 401 states, 0 layout issues; 812 swipes, 0 mistaken.

- **Round 6 (D-041):** sound pickers (bubbles), 16 sounds incl. nature, smooth tap hints, delayed hold line. Checks: 427 states, 0 layout issues; 582 swipes, 0 mistaken; all 16 sounds build.

- **Round 9 (D-043):** seconds can be set (pause, drag, tap to start). A Phone settings page for optional Bluetooth time sync, off unless tapped ([BLUETOOTH.md](BLUETOOTH.md)). Checks: 446 states, 0 layout issues; 468 swipes, 0 mistaken.

## Hardware state (bring-up test, 2026-11-12)
- `/firmware/bringup` runs on the real board: screen, touch, 7 I2C chips, ring (GRBW, smooth dithered sunrise), speaker.
- Ring ran with a direct 3.3 V data line, short wires and a 300 mA cap.
- Not yet tested: battery, full 0.9 A ring current with fuse + capacitor + resistor, USB voltage under load.

## Waiting for owner
1. ~~Approve Phase 2~~ Approved 2026-11-14 (accepts D-024 as amended, D-025, D-028, D-031 to D-047; D-044 and D-047 are the latest).
2. **Flash `/firmware/dialtest`** and try it (swipe near the rim, next to a dot, in the middle; grab a dot with and without a pause). Tell me anything that feels wrong (UX-B2). I could not flash it because the board was not plugged in.
3. **Daylight saving:** US Eastern with automatic rules, a Manual option and a two-touch fix. OK? (not yet answered)
4. **Battery:** order a 503450-class cell by the specification in HARDWARE.md §1 (answered yes).

Answered: 12-hour AM/PM · 5-minute steps · battery spec · no lux meter · drop the phone link and wireless updates (Phase 6 becomes the offline update and the long-life kit) · 30 years is a quality yardstick, not a promise.

## Early enclosure draft (2026-11-13, at the owner's request)
- [ENCLOSURE.md](ENCLOSURE.md) draft and proposed D-029 (revised): pop-out back-panel stand leaning 15° back, made with a 3D printer and laser cutter, all electronics in the head, built for 30 years with replaceable wear parts. Phase 4 has not started. **Option D approved by the owner 2026-11-13.** Final case will be metal, so the design follows metal-ready rules (ENCLOSURE.md §12, proposed D-043).
- Waiting for the owner: laser cutter materials, front halo or rear wall-wash, colours, board measurements E1–E6.

## Blockers
- None for Phase 3. Bench checks needed along the way: battery polarity (V2), RTC keeps time on battery (V6), PWR-key long-press can be disabled (V7), ring at 0.9 A for 30 min with the protection parts, night dimness banding (UX-B1), touch reliability near the edge (UX-B2).

## Next (Phase 3 plan)
Real firmware in `/firmware/clock`, in small testable steps: time and schedule logic with host tests, NVS A/B storage, alarm engine on core 1, light engine (sunrise, dithering, current cap), audio (noise, fades, escalation), the UI from UX.md with Ubuntu bitmap fonts and the layout check, hardware profile (D-025), watchdogs and safe mode. Also RELIABILITY.md and TEST_PLAN.md.
