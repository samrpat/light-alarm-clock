# Status

Last updated: 2026-11-12

| Phase | Name | State |
|-------|------|-------|
| 0 | Requirements & architecture | ✅ Done, approved 2026-11-12 |
| 1 | Power & wiring | ✅ Done, **awaiting approval** |
| 2 | UX & gesture design | ⏳ Next (concept already explored, see below) |
| 3 | Core firmware (MVP) | — |
| 4 | 3D-printed enclosure | — |
| 5 | Sleep sounds library | — |
| 6 | BLE settings sync | — |
| 7 | Polish (OTA, power, assembly guide) | — |

## Done in Phase 0
- Requirements R1–R33, N1–N5, with planned test IDs
- Assumptions A1–A8 still to verify, risks K1–K10
- Proposed decisions D-001…D-010
- Block diagram, RTOS task layout, alarm state machine ([FIRMWARE.md](FIRMWARE.md))
- Dreamie reference reviewed: adopted ideas recorded

## UX exploration ahead of Phase 2
- Owner reviewed mockups and set the direction: no motion gestures, swipe between screens, fewest possible touches (D-011).
- Interactive concept: `/mockups/ux-concept.html`. Awaiting owner feedback. UX.md is still written in Phase 2.

## Done in Phase 1
- Board pins, chips and header verified from Waveshare's hardware reference (schematic PDF not yet read)
- Power design: USB-C main supply, ring on VBUS with fuse, backup battery, PMIC settings ([POWER.md](POWER.md))
- BOM, pin table, wiring diagram, speaker hookup, bench checks V1–V8 ([HARDWARE.md](HARDWARE.md))
- Owner answers recorded as D-010, D-013, D-015…D-018. Proposed: D-012, D-014.
- Project under git, pushed to GitHub

## Waiting for owner
1. **Approve Phase 1** (accepts D-012 and D-014).
2. **Battery:** OK to add the 2000 mAh backup cell? Without it the clock stops and loses the time on any unplug.
3. **Which RGBW ring:** warm 3000 K (#2861), natural 4500 K (#2862) or cool 6000 K (#2863)?
4. **Speaker size:** diameter and depth (needed for Phase 4).
5. **Charge voltage:** 4.2 V (full capacity) or 4.1 V (longer battery life, if it stays plugged in)?

## Blockers
- None for Phase 2. Phase 3 needs bench checks V1–V8 on the real hardware.

## Next (Phase 2 plan)
Turn the approved concept (`/mockups/ux-concept.html`, D-011) into UX.md: screen map, gesture table, wireframes, night theme with pixel shift (D-016), first-run flow (no default alarm, D-015), warning and battery screens, settings (time zone, Wi-Fi, sunrise length).
