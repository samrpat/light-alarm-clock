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
- Bring-up test: sunrise made smooth with dithering across the 24 LEDs and across time (250 frames a second), plus eased colour curves. To be carried into the product light engine (R12).

## 2026-11-13 — Phase 1 approved; fewest-parts rule
- Phase 1 approved by the owner. D-012 and D-014 accepted; D-019 accepted in principle.
- D-023 (proposed): parts stay only if they add function or protection. Level shifter, PCB and PCB v2 electronics left out of the baseline; HARDWARE.md §3b describes the three-part inline build.

## 2026-11-13 — Phase 2: UX & gesture design
- Added UX.md: principles, touch maths, gesture vocabulary, screen map, wireframes for every screen and state, 8 settings pages, first-run flow, edit-flow gesture counts, night theme, status-line priorities, acceptance checks UX-1…UX-12 and UX-B1.
- D-024: off needs a hold, on needs a tap; auto-return; carry-over; velocity steps; panel dimming; scenes replaced by memory.
- Requirements R2, R5, R6, R8, R23 reworded; R34 and R35 added.
- Concept page updated for hold-to-turn-off and carry-over.

## 2026-11-13 — Phase 2 revision after owner feedback
- "No physical dial": the rim-turn gesture is gone. Values change by tap zones (top more, bottom less; left hours, right minutes; hold repeats). Five methods compared in UX.md §4.1 (D-026). Three gestures only.
- Ubuntu font everywhere; Ubuntu Font Licence saved in `docs/licenses/` (D-028).
- Automatic text check `layoutCheck()` added to the concept page: 188 screen states, 1,667 boxes, 0 issues after fixing 12 edge cases found on the first run.
- Day and night look on a schedule (D-027). Time shows in 12-hour AM/PM.
- Optional hardware (D-025) replaces the fewest-parts verdicts of D-023. Firmware must run with no battery and no ring (R36).
- UX.md rewritten. R2, R5, R6 reworded; R36, R37 added. Concept page rebuilt and republished.

## 2026-11-13 — Early enclosure draft
- Added ENCLOSURE.md (draft, ahead of Phase 4): three stand options compared, fold-out weighted foot recommended, layout of every part, 30-year rules (brass inserts, no glue, metal hinge, ASA or PETG, replaceable wear parts, board carrier), ports and buttons, materials, shopping list, measurements E1–E6, checks C1–C7.
- D-029 proposed. HARDWARE.md §5 gets the board size from a third-party listing (to be measured).

## 2026-11-13 — Phase 2 revision, round 2
- Fewer words and a short settings list (eight pages). The face now follows the day with a 3-hour wind-down (D-032). Sun arc removed at the owner's request.
- The touch rim dial restored, tap zones kept as the backup (D-034). Concept page passes a simulated drag test and the layout check (257 states, 1,796 boxes, 0 issues, max four text lines).
- Added LIGHT.md (evidence-based dawn, evening and night light, D-033) and LONGEVITY.md (30-year, offline, daylight saving without a server, D-030, D-031).
- Battery: a thin 503450-class cell by specification (D-035). Wi-Fi, BLE and OTA dropped; Phase 6 repurposed.
- Renumbered: the enclosure draft kept D-029; this round is D-030 to D-035. R5, R6, R12, R13, R16, R28, R30 reworded; N1–N3 dropped; R38–R43 and K11–K12 added.
- Revised the same day after owner input (3D printer and laser cutter only, pop-out stand, lean back): option D, a pop-out back panel with a stop block and 15° lean; laser-cut opal acrylic diffuser and silicone gaskets; stability figures added (calculated). D-029 rewritten.
- Owner approved stand option D (pop-out back panel).

## 2026-11-13 — Phase 2 revision, round 3
- **Measured a flaw:** a dial that could start in an outer band mistook 108 of 179 sideways swipes (60 %) for turns. Replaced by dots on rings, like clock hands: only a touch on a dot, after a short pause, moving along the ring, can turn it (D-036). Removed all buttons, tap zones and hold-to-repeat. Sideways always means the next neighbour.
- Stress test: 1,249 random swipes (651 within 56 px of a dot), 0 mistaken for a dial turn.
- Settings redesigned as eight pictures (D-037): Time, Date, Region, Sleep (a band of night on the clock ring), Brightness, Alarm volume, Preview alarm, About. Bedtime and Wake merged into Sleep.
- Added `/firmware/dialtest`, a test app for the real touch screen (compiles, not yet flashed).
- Owner answers: battery spec yes (D-035 accepted), phone link and OTA dropped (D-030 accepted), 30 years is a yardstick not a promise (wording softened in LONGEVITY.md and elsewhere).
