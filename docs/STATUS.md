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
- BOM, pin table, wiring diagram, speaker hookup, bench checks V1–V9 ([HARDWARE.md](HARDWARE.md))
- Halo driver PCB v1: fabrication files in `/pcb/halo-driver/` (script-checked, not yet CAD-checked or built)
- Owner answers recorded as D-010, D-013, D-015…D-018. Proposed: D-012, D-014.
- Project under git, pushed to GitHub

## Bring-up test (ahead of Phase 2, at the owner's request)
- ESP-IDF v5.5.6 installed on the Mac. `/firmware/bringup` compiles. 
- Quick wiring: ring straight to VBUS / GND / GPIO17, no level shifter or fuse, ring capped at 300 mA.
- **Flashed and running on the board (2026-11-12).** Startup log: display, touch (CST9217) and LVGL started; all 7 I2C chips answered; ring driver started on GPIO17; codec and speaker opened at 22 050 Hz.
- **Owner confirmed 2026-11-12: all modes work** (touch, ring sunrise and white, white noise, tone). Ring ran on a direct 3.3 V data line with short wires and a 300 mA cap.
- Not yet tested: battery, full 0.9 A ring current, level shifter, USB voltage under load.

## Waiting for owner
1. **Approve Phase 1** (accepts D-012, D-014, D-019).
2. PCB v2 direction (D-021): USB-C socket on the PCB at the rear, ring powered directly. Layout comes after the case design.
3. PCB layout accepted in principle; it is ordered only after the case design (D-020). First build is on a breadboard.

Answered 2026-11-12: battery yes (2000 mAh) · ring is #2861 warm white · speaker is Waveshare Speaker (B), 20 × 30 × 6.8 mm · charge to 4.1 V.

## Blockers
- None for Phase 2. Phase 3 needs bench checks V1–V8 on the real hardware.

## Next (Phase 2 plan)
Turn the approved concept (`/mockups/ux-concept.html`, D-011) into UX.md: screen map, gesture table, wireframes, night theme with pixel shift (D-016), first-run flow (no default alarm, D-015), warning and battery screens, settings (time zone, Wi-Fi, sunrise length).
