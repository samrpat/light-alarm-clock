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
| 6 | Offline update and the 30-year kit (was BLE, dropped by D-030) | — |
| 7 | Polish (power, assembly guide) | — |

## Phase 2 revision (2026-11-13)
- Fewer words: at most four text lines per screen. Eight settings pages: Time, Date, Region, Sleep hours, Brightness, Alarm volume, Preview alarm, About (D-032).
- The face follows the day (gold, cool white, warm, a 3-hour wind-down, dim amber night). No sun arc.
- The touch rim dial is back, with tap zones as the backup (D-034).
- Light plan from the evidence: 30-minute dawn, nothing lights the sleeper ([LIGHT.md](LIGHT.md), D-033).
- 30-year offline plan: no network ever, daylight saving with rules plus Manual plus an SD file, wear parts replaceable ([LONGEVITY.md](LONGEVITY.md), D-030, D-031).
- Battery: a thin 503450-class cell (D-035). Wi-Fi, BLE and over-the-air updates dropped (D-030).
- Concept page: https://claude.ai/artifact/UGtnZ6MqkrSfuM5BShyG5a (private link). Text check: 257 states, 0 issues.

## Hardware state (bring-up test, 2026-11-12)
- `/firmware/bringup` runs on the real board: screen, touch, 7 I2C chips, ring (GRBW, smooth dithered sunrise), speaker.
- Ring ran with a direct 3.3 V data line, short wires and a 300 mA cap.
- Not yet tested: battery, full 0.9 A ring current with fuse + capacitor + resistor, USB voltage under load.

## Waiting for owner
1. **Approve Phase 2** (accepts D-024 as amended, D-025, D-028, D-030 to D-035).
2. **Battery:** order a 503450-class cell by the specification in HARDWARE.md §1, then check the plug and polarity on arrival. I could not confirm a single store listing.
3. **Daylight saving:** US Eastern with automatic rules, a Manual option, and a two-touch fix. OK?
4. **A lux meter** (about $20–30) would let me set the dawn and evening light levels by measurement (LIGHT.md L1–L3). Optional.
5. **Phase plan:** Phase 6 (phone link) is dropped. Phase 6 becomes "offline update and the 30-year kit". OK?

Answered: 12-hour AM/PM yes · 5-minute steps · day vibe = the dynamic face · no picker feel test in the bring-up app (the dial may change that: UX-B2).

## Early enclosure draft (2026-11-13, at the owner's request)
- [ENCLOSURE.md](ENCLOSURE.md) draft and proposed D-029: fold-out weighted foot, all electronics in the head, built for 30 years with replaceable wear parts. Phase 4 has not started.
- Waiting for the owner: approve option C, front halo or rear wall-wash, colours, board measurements E1–E6.

## Blockers
- None for Phase 3. Bench checks needed along the way: battery polarity (V2), RTC keeps time on battery (V6), PWR-key long-press can be disabled (V7), ring at 0.9 A for 30 min with the protection parts, night dimness banding (UX-B1), touch reliability near the edge (UX-B2).

## Next (Phase 3 plan)
Real firmware in `/firmware/clock`, in small testable steps: time and schedule logic with host tests, NVS A/B storage, alarm engine on core 1, light engine (sunrise, dithering, current cap), audio (noise, fades, escalation), the UI from UX.md with Ubuntu bitmap fonts and the layout check, hardware profile (D-025), watchdogs and safe mode. Also RELIABILITY.md and TEST_PLAN.md.
