# Decision log (ADR style)

Rules: never delete an entry. To change a decision, add a new one and mark the old one
`Superseded by D-xxx`. Status values: **Proposed** (awaiting owner approval) · **Accepted** · **Superseded**.

---

## D-001 — Firmware framework: ESP-IDF + LVGL 9 + FreeRTOS
- **Date:** 2026-11-12 · **Status:** Accepted 2026-11-12 · **Reqs:** R18, R19, N4
- **Decision:** Use **ESP-IDF v5.x** (exact version pinned in Phase 3, latest stable 5.x) with `idf.py`.
  Components: `esp_lvgl_port` + **LVGL 9**, `esp_lcd` (CO5300 QSPI panel driver from Waveshare's IDF demo or the component registry), `esp_codec_dev` (codec), `led_strip` (RMT, NeoPixel), NimBLE (Phase 6), `app_update` (OTA, Phase 7).
- **Options considered:**
  1. *Arduino-ESP32 + LVGL.* Fastest start and Waveshare ships Arduino demos. But it hides `sdkconfig` (watchdog, brownout, partition, PSRAM, core pinning), adds a loop task we don't need, and timing is less predictable.
  2. *PlatformIO (either framework).* Convenient, but its IDF version lags and it is one more layer to debug.
  3. **ESP-IDF (chosen).** Full control of FreeRTOS priorities and core pinning, task/interrupt watchdogs, brownout level, NVS, partition table, bootloader rollback. Supports host-side unit tests.
- **Why:** The top requirement is that the alarm is never missed, and that is a scheduling and fault-isolation problem. IDF gives direct control over it.
- **Consequences:** Steeper start. Waveshare's IDF demo is the reference BSP for the display, touch and codec. Arduino libraries can still be pulled in as an IDF component if ever needed.

## D-002 — Alarm-critical path isolation
- **Date:** 2026-11-12 · **Status:** Accepted 2026-11-12 · **Reqs:** R18, R19, R22
- **Decision:** Alarm-critical tasks (`supervisor`, `alarm_engine`, `audio_out`, `light_fx`, `power_mon`) are **pinned to core 1** at the highest application priorities. UI (LVGL), touch/IMU input and BLE run on **core 0** at lower priorities.
  At the moment an alarm fires, the critical path needs **no** LVGL, touch, BLE, mutex owned by the UI, or I2C transaction:
  - time comes from the ESP32 system clock (loaded from the RTC at boot, re-checked periodically)
  - LEDs go through RMT
  - audio goes through I2S with software volume (D-008)
  - the fallback dismiss (BOOT button, GPIO0) is a GPIO interrupt straight into `alarm_engine`.

  UI → alarm communication goes only through a bounded command queue with validated messages. The alarm engine publishes a read-only state snapshot. It never waits on the UI.
- **Options considered:** single superloop (simple, but one hang blocks everything) · separate co-processor or ULP for the alarm (strongest isolation, but the ULP can't drive I2S or RMT) · **task isolation on core 1 (chosen)**.
- **Consequences:** Cross-core messages must be lock-free or use short-timeout primitives. A UI hang during an alarm does **not** reset the chip (see D-006).

## D-003 — Time base, time zone and DST
- **Date:** 2026-11-12 · **Status:** Accepted 2026-11-12 · **Reqs:** R15, R28
- **Decision:** The RTC stores **UTC**. The time zone is a POSIX TZ string in NVS, applied with `setenv("TZ")`/`tzset()`.
  Alarm times are **local wall-clock** times. `next_alarm(now_utc, schedule, overrides, tz)` is a pure function with host unit tests.
  DST: a nonexistent local time (spring forward) fires at the next valid minute. An ambiguous time (fall back) fires at its **first** occurrence only.
  The PCF85063 oscillator-stop flag is checked at boot. If it is set, time is "untrusted": the UI asks for the time, and the alarm runs from the last sync plus a warning.
- **Consequences:** The user picks a time zone once in setup. The phone sync (Phase 6) sends UTC + TZ.

## D-004 — Schedule persistence format
- **Date:** 2026-11-12 · **Status:** Accepted 2026-11-12 · **Reqs:** R1, R3, R17, K8
- **Decision:** NVS namespace `alarm`, two blob keys `sched_a` and `sched_b`. Each record holds `{magic, schema_ver, seq, payload, crc32}`.
  A write goes to the older slot, then is read back and checked. Load picks the valid record with the highest `seq`.
  If both are invalid: ~~load defaults (Mon–Fri 07:00)~~ *(superseded by D-015: empty schedule plus a warning)*.
  Fast-changing alarm runtime state (stage, snooze count) lives in **RTC slow memory** (survives a soft reset) and is written to NVS only on state transitions (for power-loss recovery).
- **Consequences:** Settings survive a power cut mid-write. Write count stays low. (The defaults are an open question for the owner.)

## D-005 — Flash partition layout fixed now (OTA-ready)
- **Date:** 2026-11-12 · **Status:** Accepted 2026-11-12 · **Reqs:** N4, R11
- **Decision (16 MB flash):** `nvs` 64 KB · `otadata` 8 KB · `phy_init` 4 KB · `ota_0` 4 MB · `ota_1` 4 MB · `nvs_log` 64 KB (fault log) · `sounds` (LittleFS) ~7.8 MB. Enable bootloader app rollback.
- **Why:** Changing partitions later means reflashing over USB and losing settings. Fixing them now makes Phase 7 OTA a pure software change.
- **Consequences:** Firmware must stay < 4 MB (comfortable with LVGL + NimBLE). Large sound libraries go on the TF card.

## D-006 — Watchdogs, supervisor and boot-loop safe mode
- **Date:** 2026-11-12 · **Status:** Accepted 2026-11-12 · **Reqs:** R19, R20, R21
- **Decision:**
  - The **task watchdog** (5 s, panic → reset) watches the core-1 critical tasks.
  - The **interrupt watchdog** is on. **Brownout detector** is on (level set in Phase 1).
  - The `supervisor` checks **heartbeats** from UI, input and BLE. If one is stale for more than 10 s *outside* the alarm window (no alarm within 20 min and none active), it does a logged `esp_restart()`. *Inside* the window it does nothing. The alarm keeps running and the BOOT-button dismiss still works.
  - **Boot-loop guard:** a reset counter in RTC memory. After 3 abnormal resets within 10 min, boot into **alarm-only safe mode**: no BLE and a minimal UI (time + next alarm + dismiss), with alarm, light and sound fully working.
  - **Catch-up:** at boot, an unhandled occurrence ≤ 60 min old fires at once (R20).
- **Consequences:** Every reset reason is logged to `nvs_log` for diagnosis.

## D-007 — Dismiss / snooze safety model
- **Date:** 2026-11-12 · **Status:** Accepted 2026-11-12 · gesture details superseded by D-011 · **Reqs:** R7, R23, K6
- **Decision:**
  - **Snooze** is easy: tap anywhere, or flip face-down if IMU gestures are on.
  - **Dismiss** is deliberate: a hold-and-drag gesture (exact form set in Phase 2), which a stray palm or a bump cannot trigger.
  - **IMU never dismisses.**
  - **Fallback dismiss:** hold the BOOT button for 3 s. This works when touch or UI is dead.
  - When the snooze limit is reached, snooze is replaced by a "last call": it keeps ringing.
- **Consequences:** The enclosure must give access to the BOOT button (Phase 4).

## D-008 — Alarm volume in software, codec gain fixed
- **Date:** 2026-11-12 · **Status:** Accepted 2026-11-12 · **Reqs:** R10, R18, R22
- **Decision:** Codec analog/DAC gain is set once at boot to a calibrated maximum (Phase 1, matched to your speaker). All volume, fades and escalation are done by **scaling samples in `audio_out`** (32-bit math, per-sample ramps so there are no zipper sounds).
- **Why:** No I2C on the alarm path. Fades are smooth and sample-accurate. Volume limits can't be bypassed.
- **Consequences:** At very low volumes there is slightly less dynamic range. This is not audible for noise or alarm tones.

## D-009 — Repository layout; docs are the project memory
- **Date:** 2026-11-12 · **Status:** Accepted 2026-11-12
- **Decision:** `/docs` (this set), `/firmware` (ESP-IDF project, `components/` per module, `test/` host tests), `/enclosure` (parametric CAD). Docs are updated at the end of every phase. Recommend putting the folder under git (owner's choice).

## D-010 — Wi-Fi (SNTP + OTA)
- **Date:** 2026-11-12 · **Status:** Accepted 2026-11-12, option (b). Wi-Fi is opportunistic: several networks can be saved; away from home the clock runs from the RTC and phone sync. See D-012 note on time.
- **Options:** (a) **BLE only.** Time sync from the phone; OTA over USB or BLE (slow, ~1–2 MB at BLE speed takes minutes). (b) **Wi-Fi for SNTP + OTA only**, credentials entered via BLE. Wi-Fi is never needed for the alarm.
- **Recommendation:** (b). Automatic accurate time and easy OTA. Wi-Fi stays off except a short daily sync (power and RF quietness at night).

## D-011 — No motion gestures; four-gesture touch vocabulary
- **Date:** 2026-11-12 · **Status:** Accepted (owner decision); the tap-to-turn-off in the first concept is replaced by hold (D-024), and "turn the edge" by tap zones (D-026) · **Reqs:** R2, R5, R7, R23
- **Decision:** The clock stays in one place on the nightstand, so **no IMU gestures** (no flip-to-snooze, no pick-up to wake). The IMU is unused for now.
  The touch UI uses four gestures only: **swipe** (go to a screen), **turn the edge** (change a value), **tap** (on/off), **hold** (guarded actions). No double-tap, so taps respond at once.
  Screens form a cross around the clock, each one swipe away: up = Tomorrow's alarm (one-off change or skip), down = Week (tap a day, then turn), left = Sound, right = Light.
  Ringing: tap anywhere = snooze, hold 2 s = dismiss. BOOT hold 3 s stays as the fallback.
- **Supersedes:** the flip-to-snooze and hold-and-drag dismiss parts of D-007. The rest of D-007 stands.
- **Consequences:** Editing a weekday is 3 gestures (R2 met). Tomorrow-only changes are 2. Concept prototype: `/mockups/ux-concept.html`. Detail goes into UX.md in Phase 2.

## D-012 — Power architecture: USB-C primary, LED ring on VBUS, battery as backup
- **Date:** 2026-11-12 · **Status:** Accepted 2026-11-13 (Phase 1 approved) · **Reqs:** R14, R21, R24, R25, R29–R32
- **Context:** The owner runs the clock from USB-C and has no battery yet. The board's 8-pin header exposes **VBUS** (USB 5 V) but no battery or VSYS pin.
- **Decision:**
  - The NeoPixel ring is powered **straight from the header VBUS pin** (USB 5 V) through a 1.1 A resettable fuse. No boost converter.
  - A single-cell Li-ion on the board's MX1.25 connector is a **backup**, strongly recommended. It keeps the RTC, screen and alarm **sound** running through a power cut or an unplug. The ring is dark on battery.
  - On battery, the alarm still fires: sound escalation as normal, and the screen ramps to full white as a small substitute light.
  - Firmware caps ring current at **0.9 A** and sets the AXP2101 input limit to **1.0 A** and charge current to **400 mA**, so the total stays under 2 A. Required supply: **5 V, ≥ 3 A USB-C adapter**.
- **Options considered:**
  1. **Ring on VBUS, battery backup (chosen).** Fewest parts, no boost losses or noise, nothing that can fail between battery and alarm.
  2. *Boost from the battery (e.g. Adafruit PowerBoost 1000 Basic #2030) with a diode-OR to VBUS.* Sunrise works unplugged. Needs a splice into the battery leads (the header has no battery pin), 4 more parts, and a bigger cell (≥ 3000 mAh). Kept as a documented upgrade in POWER.md §8.
  3. *Run the ring from the 3.3 V rail.* Rejected: rail current is far too low and the LEDs are out of spec.
- **Consequences:** Without a battery, a power cut stops the clock and loses the time (the RTC has no cell of its own), so the "never miss an alarm" goal is **not met without the battery**. R30 is reworded to a backup-runtime target.

## D-013 — LED ring is the RGBW version
- **Date:** 2026-11-12 · **Status:** Accepted (owner has it) · **Reqs:** R12, R13 · resolves risk K1
- **Decision:** Use the Adafruit 24-LED **RGBW** ring (#2861 warm 3000 K, #2862 natural 4500 K or #2863 cool 6000 K; the owner still has to confirm which). 32 bits per pixel, GRBW order, SK6812 timing.
- **Consequences:** White light comes from the dedicated white LED, which looks better and uses about a third of the power of mixed RGB white. Full-on worst case rises to 24 × 4 × ~18 mA ≈ 1.7 A, so the firmware cap (R14) matters more.

## D-014 — NeoPixel data on GPIO17 through a 74AHCT125
- **Date:** 2026-11-12 · **Status:** Accepted 2026-11-13, then amended by D-023 (level shifter left out of the baseline unless the ring flickers) · **Reqs:** R32
- **Decision:** Data pin **GPIO17** (header pin 6). One gate of a **74AHCT125** powered from VBUS shifts 3.3 V → 5 V. 470 Ω in series at the ring's DIN, 10 kΩ pull-down on the gate input, 1000 µF across the ring supply.
- **Why GPIO17, not GPIO16:** Waveshare's `pin_config.h` names GPIO16 as an I2S MCLK alias while their hardware reference says MCLK is GPIO42. Until that is checked on the real board, GPIO16 stays unused.
- **Options considered:** direct 3.3 V drive (out of spec at 5 V: needs ≥ 3.5 V high) · diode trick to lower the first pixel's supply (dims and tints it) · BSS138 shifter modules (too slow for 800 kHz) · **74AHCT125 (chosen)**.
- **Consequences:** AHCT inputs tolerate a signal with their supply off, so nothing back-feeds VBUS when unplugged. GPIO18 and GPIO16 stay spare.

## D-015 — Alarm policies
- **Date:** 2026-11-12 · **Status:** Accepted (owner) · **Reqs:** R17, R22, R23 · supersedes the defaults in D-004
- **Decision:** A new or reset clock has **no alarms**; the user adds them. If stored settings are lost, the schedule is empty and the clock shows a persistent warning and chimes at the next interaction. Sound auto-stops after 30 min of ringing (light stays on). Snooze is 9 min, up to 3 times.

## D-016 — Night display: always on, very dim, shifting
- **Date:** 2026-11-12 · **Status:** Accepted (owner) · **Reqs:** R6 · addresses K4
- **Decision:** The night face stays on at very low brightness. The whole face shifts by a few pixels every minute along a slow orbit to spread wear.

## D-017 — Default time zone: US Eastern
- **Date:** 2026-11-12 · **Status:** Accepted (owner) · **Reqs:** R28
- **Decision:** Default POSIX TZ `EST5EDT,M3.2.0,M11.1.0`. Changeable in settings and by phone sync.

## D-018 — Version control
- **Date:** 2026-11-12 · **Status:** Accepted (owner)
- **Decision:** Git, remote `github.com/samrpat/light-alarm-clock` (public), branch `main`. One commit per phase at minimum.

## D-019 — Support circuit on a small custom PCB
- **Date:** 2026-11-12 · **Status:** Accepted in principle 2026-11-13; on hold (D-023) · **Reqs:** R32
- **Decision:** The ring driver circuit (D-014 plus fuse and bulk capacitor) goes on a 44 × 28 mm, 2-layer, all-through-hole board, "halo driver v1". Fabrication files are generated by `/pcb/halo-driver/gen_pcb.py`, which also checks netlist, connectivity and clearances.
- **Options considered:** perfboard (no wait, but messy and error-prone) · surface-mount board with factory assembly (smaller, but higher cost and minimum order for a 6-part circuit) · a full custom board replacing the Waveshare module (rejected: the display module is the fixed hardware) · **through-hole PCB (chosen)**.
- **Why a script, not KiCad:** KiCad is not usable on the development machine. The board is simple enough (6 nets) to generate and check directly.
- **Consequences:** No CAD design-rule check yet; view the Gerbers in the manufacturer's viewer before ordering. Board shape may change in Phase 4 to fit the case.

**Owner answers recorded 2026-11-12:** battery yes, 2000 mAh (D-012) · ring is Adafruit #2861, warm white 3000 K (D-013) · charge voltage 4.1 V (POWER.md §5) · speaker is Waveshare 8 Ω 2 W Speaker (B).

## D-020 — First build on a breadboard; PCB after the case design
- **Date:** 2026-11-12 · **Status:** Accepted (owner) · amends D-019
- **Decision:** The first working build uses a solderless breadboard for the level shifter, with the ring's power path wired directly (HARDWARE.md §3a). The halo driver PCB is revised and ordered only after Phase 4, so it fits the case.
- **Consequences:** The PCB files stay in `/pcb/halo-driver/` as v1. Its outline, holes and connector positions are expected to change.

## D-021 — PCB v2 takes USB-C power itself and feeds the ring and the board
- **Date:** 2026-11-12 · **Status:** Proposed, on hold (D-023) · **Reqs:** R14, R29, R32, R33 · amends D-012 and D-019 for the PCB build only
- **Decision:** The PCB carries its own power-only USB-C socket at the rear of the case. From there: a current-limited switch feeds the ring directly, and an ideal diode feeds the Waveshare board through its header VBUS pin. The PCB reports the adapter's advertised current to the ESP32. Details in POWER.md §10.
- **Why:** The cable leaves from the back instead of the side, and ring brightness is no longer held to 0.9 A by the unknown capacity of the Waveshare VBUS pin.
- **Options considered:** keep the ring on the board's VBUS pin (simplest, but side cable and 0.9 A cap) · panel-mount USB-C extension cable to the board's port (rear cable, but cap stays) · USB-C PD at 9–12 V with a buck converter (more power than a 24-LED ring can use) · **USB-C 5 V on the PCB (chosen)**.
- **Consequences:** Surface-mount parts, so factory assembly or fine soldering. Bench check V10 must pass. Firmware keeps a current ceiling, now set by the adapter and by heat. The first (breadboard) build is unchanged.

## D-022 — Toolchain pinned; Waveshare board package used; early bring-up test
- **Date:** 2026-11-12 · **Status:** Accepted (owner asked to try the hardware today) · refines D-001
- **Decision:** ESP-IDF is pinned to **v5.5.6**. The firmware uses Waveshare's official board package `waveshare/esp32_s3_touch_amoled_1_75` (v3.0.1, needs IDF ≥ 5.5) for display, touch, I2C, audio and SD card. That package drives LVGL 9 through `esp_lvgl_adapter`, not `esp_lvgl_port` as D-001 assumed. The ring uses `espressif/led_strip` v3 (RMT, SK6812 GRBW).
- A throwaway **bring-up test** (`/firmware/bringup`) is built before Phase 2 to prove the screen, touch, ring and speaker and to answer bench checks early. It is not the product firmware and does not follow the D-002 task layout.
- **Consequences:** Phase 3 must check that the board package's LVGL task and I2C handling can live on core 0 without touching the alarm path. If not, the package is used only as a pin and driver reference.

## D-023 — Fewest parts: a part stays only if it adds function or protection
- **Date:** 2026-11-13 · **Status:** Superseded by D-025 (the owner is fine with these parts and wants them optional) · was: amends D-014, D-019, D-020, D-021
- **Decision:** Every part must either add a function or protect something. Applied to the Phase 1 design:

| Part | Verdict | Reason |
|------|---------|--------|
| Backup battery | Keep | Keeps time and alarm sound through a cut (reliability). |
| 1000 µF capacitor across the ring | Keep | Protects the first LED from the plug-in surge. |
| 470 Ω resistor in the data line | Keep | Protects the first LED from spikes. |
| 1.1 A resettable fuse in the ring's 5 V | Keep | Safety against a wiring short. |
| 74AHCT125 level shifter, 10 kΩ, 100 nF | **Leave out of the baseline** | The bring-up test drove the ring directly from 3.3 V with short wires and worked. Re-test at the final wire length and the full current cap. Add the shifter only if it flickers. |
| Halo driver PCB v1 | **Not needed** | With only three small parts, they solder inline on the wires. No board, no breadboard. |
| PCB v2 with USB-C socket, ring switch, ideal diode, adapter sensing | **On hold** | Adds mainly a rear cable exit, which a panel-mount USB-C extension cable also gives with zero electronics. Revisit in Phase 4 only if the 0.9 A cap or heat proves limiting. |

- **Risk accepted:** Driving the ring's data input at 3.3 V is outside its datasheet (it wants ≥ 3.5 V). It works today, but may not on another ring, in the warm case or at long wires. The fallback is a $1.50 chip added inline. Test M-ring: run the final wiring at the 0.9 A cap for 30 min and watch for flicker or wrong colours.
- **Consequences:** The parts list drops to three small parts plus wires. The PCB files stay in `/pcb/halo-driver/` for later. Applies to firmware too: no feature is added without a stated need.

## D-024 — Interface rules from Phase 2
- **Date:** 2026-11-13 · **Status:** Proposed (awaiting Phase 2 approval); items 5 and 6 superseded by D-026, item 7 by D-027 · **Reqs:** R2–R6, R8, R23, R34, R35 · amends D-011
- **Decision** (full detail in [UX.md](UX.md)):
  1. **Off needs a hold, on needs a tap.** Day: tap turns on, hold 1 s turns off. Tomorrow: hold 1 s skips, tap brings it back. Replaces the tap-to-toggle in the first concept.
  2. **Always back:** the opposite swipe returns. Any sub-screen returns to the clock after 20 s idle (settings 30 s). Every change saves at once.
  3. **Settings:** hold the clock 1.5 s. Eight pages side by side (time zone, set time, sunrise length, alarm loudness, night dimness, preview alarm, Wi-Fi status, about).
  4. **Carry-over:** swiping sideways on Day gives the next day (if off) the time just set. Mon–Fri is 11 gestures.
  5. **Dial speed sets the step:** 1, 5, 15, 60 minutes by turn speed. Volume and brightness are linear.
  6. **Rim or swipe:** decided by whether the first movement follows the rim (turn) or heads inward (swipe). A swipe can start anywhere.
  7. **Night dimming through the panel's brightness setting, not darker colours**, to avoid banding (bench check UX-B1). No light sensor, so the dim level is always on and a tap lifts it for 5 s.
  8. **Scenes are replaced by memory:** Sound and Light remember their settings. No scene feature is built (fewest parts, D-023).
  9. **One status line** at a time, in a fixed priority order (UX.md §8).
- **Options considered:** tap-to-toggle everything (a sleepy tap can silence tomorrow) · confirm dialogs (more touches) · double-tap (needs a wait before every tap; ruled out in D-011) · scene presets (an extra screen for little gain).
- **Consequences:** Phase 3 builds a large-digit font (150 px, digits and colon), a touch classifier with host tests, and the auto-return timer. The concept page is updated to the hold rule. Settings, first run, warnings and the sunrise screen are wireframes only.

## D-025 — Hardware extras are optional; firmware runs without them
- **Date:** 2026-11-13 · **Status:** Proposed (owner: "I am okay with those components, I just want them optional so I can develop while they come") · supersedes D-023
- **Decision:** The level shifter, fuse, capacitor, resistor, custom PCB and backup battery all stay in the design, but none is needed to build and run the firmware. Each has a defined behaviour when it is missing:
  - **Battery:** detected through the battery chip. When absent, battery text and "Plug me in" never show, and About reads "Battery: none".
  - **Ring:** a build flag. When absent the Light screen says "No ring fitted" and the sunrise uses the screen.
  - **Protection parts:** a build flag `RING_CAP_MA`, **0.3 A** until the fuse, capacitor and resistor are fitted, then **0.9 A**.
  - **Level shifter, PCB, future light sensor:** no firmware difference.
  - A missing part must never block the alarm path (D-002).
- **Parts are labelled** Required, Recommended or Optional in HARDWARE.md. The fuse, capacitor and resistor are "needed before the ring goes above 0.3 A".
- **Consequences:** Phase 3 firmware has a small `hw_profile` header and a boot-time check. Test UX-15 covers it. D-019 and D-021 stay on hold (PCB later); D-014's level shifter is optional.

## D-026 — Values are changed by tap zones; no rim or dial gestures
- **Date:** 2026-11-13 · **Status:** Proposed (owner: "there is no physical dial", "consider multiple ways and figure out the best one") · supersedes the "turn the edge" gesture of D-011 and D-024 items 5 and 6 · **Reqs:** R2, R5, R34
- **Decision:** Three gestures: tap, swipe, hold. The top of the screen raises a value, the bottom lowers it. On time pickers the left half moves hours (±1 h) and the right half moves minutes (±5 min, carrying into the hour). Holding a tap zone repeats and speeds up. Holding the centre is the guarded action.
- **Options considered:** rim drag, drag-anywhere drum, one-value tap zones, arc slider (full table in UX.md §4.1; weighted scores A 50 · B 55 · C 84 · **D 88** · E 58, my judgement). Tap zones win on reliability, no new gesture type, no clash with swipes, and no dependence on the rim or the case.
- **Consequences:** A 30-minute change is a hold of about 1.5 s. R2 is counted in touches with a hold as one. The two methods can be compared on the real panel in the bring-up app (UX-B2). The classifier is simpler: tap, swipe, hold only.

## D-027 — Day and night look on a schedule
- **Date:** 2026-11-13 · **Status:** Proposed (owner: "want day and night schedule, some vibe for day and dim for night") · supersedes D-024 item 7 · **Reqs:** R6
- **Decision:** Night hours are settable, default 9 PM to 7 AM. Night: amber only, blue under 15 %, dimness 1–5 through the panel, the face shifts a step a minute (D-016). Day: brighter (10–100 %, default 50 %), an accent colour (Amber, Sky, Mint, Rose) and the date. If the time is untrusted, the night look is used. A tap lifts the night face for 5 s. The ring is not part of the look.
- **Options considered:** schedule (chosen) · light sensor (not on the board; a possible optional part) · manual switch (a bright screen at 3 a.m. is the worst failure).
- **Open:** what extra "vibe" the day look should have.

## D-028 — Ubuntu type and an automatic layout check
- **Date:** 2026-11-13 · **Status:** Proposed (owner: "use Ubuntu font style", "no awkward overlaps between text") · **Reqs:** R5, R37
- **Decision:** All device text is Ubuntu (Light, Regular, Medium), under the Ubuntu Font Licence 1.0 (text in `docs/licenses/`). Firmware uses bitmap fonts made from the TTFs with `lv_font_conv`. Text sits inside a safe circle of radius 209 px, never under 26 px, never overlapping. `layoutCheck()` in the concept page measures all 188 screen states in the real font. Result: 1,667 boxes, 0 issues.
- **Consequences:** Phase 3 builds the fonts and repeats the check on the real strings, and the check is a gate for UI changes. The Ubuntu Font Licence should be re-read before any firmware binary is distributed.
