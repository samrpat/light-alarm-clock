# Requirements

Status key: `[ ]` not met · `[~]` partly met · `[x]` met and its test passes.
Priority: **M** = must-have, **N** = nice-to-have.
Every requirement maps to at least one test. Test IDs (`T-xx`) are placeholders until
[TEST_PLAN.md](TEST_PLAN.md) is written in Phase 3.

Last reviewed: 2026-11-13 (Phase 2)

## 1. Schedule

| ID | Pri | Requirement | Phase | Test |
|----|-----|-------------|-------|------|
| R1 | M | `[ ]` Weekly schedule: for each of the 7 weekdays, a wake time (HH:MM, 1-minute resolution) and an on/off flag, set independently. | 3 | T-01 |
| R2 | M | `[~]` Changing one weekday's time takes **≤ 3 touches** (swipe down, tap the day, drag the inner dot; a hold counts as one touch) and **≤ 10 s** from the clock face. Designed in UX.md §8; not yet built. | 2, 3 | T-02 |
| R3 | M | `[ ]` One-off overrides: **Skip next alarm** and **Tomorrow only at HH:MM**. An override is used once, then cleared automatically. It can be cancelled before it fires. | 3 | T-03 |
| R4 | M | `[ ]` The clock face always shows the next alarm that will actually fire, overrides included (for example "Skipping Tue → next Wed 06:45"), or "No alarm set". | 2, 3 | T-04 |

## 2. Interaction

| ID | Pri | Requirement | Phase | Test |
|----|-----|-------------|-------|------|
| R5 | M | `[ ]` Four gestures, four jobs (swipe goes, tap does, drag a dot changes, hold stops). No buttons. Values change only by dragging a dot. Text ≥ 26 px, at most four text lines per screen, and a swipe can start anywhere (UX.md §1–3, D-036). | 2 | T-05 |
| R6 | M | `[~]` The face follows the day (UX.md §7, D-032): colour and brightness change from wake, wind down over the 3 hours before bedtime, and end as the dim amber night face (blue under 15 %, dimmed through the panel, shifts every minute, no white until the alarm rings). Wake and bedtime are settable (default 7:00 AM and 10:30 PM). | 2, 3 | T-06 |
| R7 | — | ~~IMU gestures (pick-up to wake, flip to snooze).~~ **Dropped** by D-011: the clock is stationary. | — | — |
| R8 | N | `[ ]` Sound and Light remember their last volume, timer, brightness and colour mode, so starting either is 2 gestures. This replaces the scenes idea taken from Dreamie (D-024, D-023). | 2, 3 | T-08 |
| R34 | M | `[ ]` Anything that can stop an alarm (turn a day off, skip tomorrow, dismiss, cancel a snooze or sunrise) needs a **hold**. Anything that only makes it ring (turn on, snooze, bring back) is a tap (D-024). | 2, 3 | T-34 |
| R35 | M | `[ ]` Every sub-screen returns to the clock after 20 s without a touch (settings 30 s). Every change is saved the moment it is made (D-024). | 2, 3 | T-35 |
| R36 | M | `[ ]` The firmware boots, keeps time and rings with no battery and with no ring fitted. Optional parts (battery, ring, protection parts, level shifter, light sensor) each have a defined behaviour when missing (D-025). | 3 | T-36 |
| R37 | M | `[~]` No text overlaps, none under 26 px, all inside the safe circle, in Ubuntu. `layoutCheck()` reports zero issues for every screen state (D-028). Passes on the concept page; repeats on the firmware strings in Phase 3. | 2, 3 | T-37 |
| R38 | M | `[ ]` The device never needs a network, server, account or phone at any time in its life (D-030, LONGEVITY.md §1). The radios are never switched on. | all | T-38 |
| R39 | M | `[ ]` Dates are correct to 2099: 64-bit time, every leap day 2028–2056, the year 2038 (LONGEVITY.md §2). | 3 | T-39 |
| R40 | M | `[ ]` The alarm still works if the screen, touch, speaker, ring or battery fails, or one copy of the program or settings is corrupt (LONGEVITY.md §6). | 3 | T-41, T-44 |
| R41 | M | `[ ]` The firmware can be rebuilt and updated with no network: pinned toolchain, every library committed to the repository, a "long-life kit", and an SD-card update (LONGEVITY.md §5). | 7 | T-42 |
| R42 | M | `[ ]` The battery is a replaceable consumable (a door, nothing glued), and the clock keeps working without it (LONGEVITY.md §4). | 4 | T-45 |
| R43 | M | `[ ]` Nothing lights the sleeper: the ring is off from bedtime to the dawn, the screen is under 1 lux at the pillow, and the evening light is warm and capped (LIGHT.md §2, §4). | 3 | T-46 |
| R44 | M | `[~]` A swipe starting anywhere, including right next to a dot, is never mistaken for a dial turn, and a deliberate grab always turns it (D-036). Host test: 1,249 random swipes, 0 mistaken. Still to do on the real panel with `/firmware/dialtest` (UX-B2). | 2, 3 | T-47 |

## 3. Sound

| ID | Pri | Requirement | Phase | Test |
|----|-----|-------------|-------|------|
| R9 | M | `[ ]` White noise for falling asleep, with sleep timer (Off / 15 / 30 / 60 / 90 min / all night) and volume control. | 3 | T-09 |
| R10 | M | `[ ]` White noise fades out smoothly (default 120 s, no clicks or steps you can hear) when the sunrise or the alarm begins. The timer end also fades (default 30 s). | 3 | T-10 |
| R11 | N | `[ ]` More sleep sounds (rain, fan, pink/brown noise, ocean), stored on flash or TF card, loop with no gap. | 5 | T-11 |

## 4. Light

| ID | Pri | Requirement | Phase | Test |
|----|-----|-------------|-------|------|
| R12 | M | `[ ]` Dawn: 30 minutes, fixed, exponential, deep red → amber → warm white at the alarm time → bright daylight hold for 10 minutes, then fade (LIGHT.md §3, D-033). Needs USB power; on battery the screen ramps instead. | 3 | T-12 |
| R13 | M | `[ ]` Reading light and night light with caps by time of day, and both switching off with the sleep timer or after 20 minutes at night (LIGHT.md §4). | 3 | T-13 |
| R14 | M | `[ ]` Firmware enforces a hard LED current cap (value set in [POWER.md](POWER.md)), whatever the colour or mode. | 1, 3 | T-14 |

## 5. Alarm reliability (the alarm must never be missed)

| ID | Pri | Requirement | Phase | Test |
|----|-----|-------------|-------|------|
| R15 | M | `[ ]` Time survives power loss: the PCF85063 RTC is the time source, and system time is restored from it at boot. Lost RTC integrity (oscillator-stop flag) is detected and shown as "Set time"; it never silently gives a wrong time. | 3 | T-15 |
| R16 | M | `[ ]` No time source outside the device. Time is set by hand, and the clock learns its own drift and trims the RTC when you correct it (LONGEVITY.md §2). An optional GNSS receiver may be added later. | 3 | T-16 |
| R17 | M | `[ ]` Schedule is stored in NVS as an A/B pair of records, each with schema version, sequence number and CRC32. A bad record falls back to the other copy. If both are bad, the schedule is empty and the clock shows a persistent warning and chimes (D-015). Never a silent "no alarm". A new clock has no alarms until the user adds one. | 3 | T-17 |
| R18 | M | `[ ]` The alarm path does not depend on UI, touch, BLE or the I2C bus at the moment it fires. A UI crash or hang, or a BLE fault, cannot block or delay an alarm. | 3 | T-18 |
| R19 | M | `[ ]` Watchdogs: task watchdog on every alarm-critical task, interrupt watchdog, a supervisor heartbeat for UI/BLE, and a boot-loop guard that enters an "alarm-only" safe mode. | 3 | T-19 |
| R20 | M | `[ ]` Missed-alarm catch-up: if the device boots within the catch-up window (default 60 min) after an alarm that was never handled, the alarm fires at once. | 3 | T-20 |
| R21 | M | `[ ]` Defined, tested behaviour for brownout, low battery, USB unplug/replug, and reboot during the sunrise or ringing window (the alarm resumes at the right escalation stage). | 1, 3 | T-21 |
| R22 | M | `[ ]` Escalating alarm: light (sunrise) → soft sound → louder sound in steps, up to a configured maximum. The alarm volume has a floor that cannot be set to silent. | 3 | T-22 |
| R23 | M | `[ ]` Snooze (default 9 min, max 3 snoozes, both configurable). Snooze is a tap anywhere. Dismiss is a 2 s hold with a filling rim, hard to trigger by accident. A BOOT-button hold (3 s) dismisses even if the touch/UI is dead. | 2, 3 | T-23 |
| R24 | M | `[ ]` Low-battery warning the night before: at bedtime, or when USB is unplugged after 20:00, warn if the predicted charge can't cover the night plus alarm. | 1, 3 | T-24 |
| R25 | M | `[ ]` Battery reserve: as the battery nears the energy needed for the next alarm, shed load (white noise, then display, then sunrise brightness) so the alarm sound still fires. | 1, 3 | T-25 |
| R26 | M | `[ ]` Self-test / **Preview alarm**: a compressed 60 s run of sunrise → sound escalation → snooze/dismiss, plus an LED, speaker and RTC check. | 3 | T-26 |
| R27 | M | `[ ]` The PWR button cannot switch the clock off by accident (PMIC long-press power-off set to the longest delay or disabled; power-off only through a confirmed UI action). | 1, 3 | T-27 |
| R28 | M | `[ ]` Correct local time across regions and daylight saving with no network: rules stored in the clock for any year to 2099, a Manual region, and a `region.txt` SD-card file for other rules (LONGEVITY.md §3, D-031). A skipped hour rings at the next valid minute, a repeated hour rings once. Fixing the clock after a rule change takes 2 touches. | 3 | T-28, T-40 |

## 6. Power

| ID | Pri | Requirement | Phase | Test |
|----|-----|-------------|-------|------|
| R29 | M | `[~]` USB-C is the main supply. A backup Li-ion cell charges while in use and takes over on unplug without a reset (designed in POWER.md; not yet tested). | 1 | T-29 |
| R30 | M | `[~]` The backup battery (about 1000 mAh, 5 mm thick, D-035) keeps the clock and the alarm sound running through a power cut: clock only about 15 h; a typical night with a 90-minute sleep timer uses about 75 % of its usable energy. A full 8 h of white noise on battery is not covered. The ring is dark on battery. By calculation only, not yet measured. | 1 | T-30 |
| R31 | M | `[ ]` Safe shutdown at critical battery: state saved, LEDs off, clean power-down, and it recovers on next power-up. | 1, 3 | T-31 |
| R32 | M | `[~]` 5 V NeoPixel supply and 3.3 V→5 V data path per [POWER.md](POWER.md) (bulk capacitor, series data resistor, level shifting). | 1 | T-32 |

## 7. Enclosure

| ID | Pri | Requirement | Phase | Test |
|----|-----|-------------|-------|------|
| R33 | M | `[ ]` 3D-printed enclosure: round display, NeoPixel ring as a diffused halo, tuned speaker chamber and grille, battery bay, USB-C access, button access, ventilation, weighted non-slip base. | 4 | T-33 |

## 8. Connectivity and maintenance (nice-to-have)

| ID | Pri | Requirement | Phase | Test |
|----|-----|-------------|-------|------|
| N1 | N | `[–]` Dropped: the product has no radio (D-030). | 6 | T-40 |
| N2 | N | `[–]` Dropped: the product has no radio (D-030). | 6 | T-41 |
| N3 | N | `[–]` Dropped: the product has no radio (D-030). | 6 | T-42 |
| N4 | N | `[ ]` Offline firmware update from an SD-card file or a USB cable, with automatic rollback if the new program fails its self-test (LONGEVITY.md §5). | 7 | T-42 |
| N5 | N | `[ ]` The device works fully with no phone. Every setting can be changed on the device itself. *(Also a Dreamie principle.)* | 2, 3 | T-44 |

---

## Constraints, assumptions and risks

### Hardware assumptions (checked in Phase 1 against Waveshare's hardware reference; V-numbers are bench checks in HARDWARE.md §6)

| # | Assumption | Why it matters |
|---|------------|----------------|
| A1 | ✅ Confirmed: **ES8311** + **ES7210**, **NS4150B** amp, MX1.25 speaker header. Amp supply rail still unknown (V4). | Driver choice; speaker power limit. |
| A2 | ✅ Confirmed: one I2C bus (SCL 14, SDA 15) for all seven chips. | A hung touch controller could block the bus. R18 forbids the alarm needing I2C at fire time. |
| A3 | ✅ Resolved: amp enable is **GPIO46, direct**. No I2C needed for sound. | If so, the amp is enabled at boot and kept on, or re-asserted with bus recovery. |
| A4 | ✅ Confirmed by Waveshare's text: RTC is fed by the AXP2101/battery, no coin cell. Without the battery, time is lost on unplug. | If the Li-ion is empty or unplugged, time is lost. The OS flag catches this (R15). |
| A5 | ✅ Resolved: RTC INT reaches only the expander (P3), not a GPIO. Polled. | Only needed for deep-sleep wake. The baseline design stays awake all night. |
| A6 | ✅ Confirmed: GPIO16, 17, 18 (+ UART 43/44, VBUS, 3V3, GND). GPIO17 = ring data. GPIO16 on hold (V1). | Pin budget for Phase 1. |
| A7 | ⚠ Still to verify on the board (V7). | R27. |
| A8 | ⚠ Polarity is not documented in text. Check with a multimeter (V2). **Many hobby batteries are reversed.** | Reversed polarity can destroy the PMIC. |

### Risks

| ID | Risk | Impact | Mitigation |
|----|------|--------|------------|
| K1 | ✅ **Resolved by D-013 (owner has the RGBW ring).** Original risk: RGB ring, not RGBW. "Warm white" and "daylight" are mixed from R+G+B, so colour quality is limited and white tends to look cool and harsh. | Sunrise looks less natural than commercial lamps such as Dreamie, which uses a 120-LED array and a 2700 K warm channel. | Careful colour curves plus a deep diffuser. **Option:** swap for the same-size 24-LED RGBW ring with a warm-white chip (part number confirmed in Phase 1). |
| K2 | **Limited brightness.** 24 LEDs at full white draw about 1.4 A at 5 V (~7 W) in theory, which is more than battery + boost should supply. | Sunrise is a strong bedside glow, not a light-therapy lamp (~10 000 lux). | Firmware current cap (R14). Brightest stage allowed only on USB power (decided in Phase 1). |
| K3 | **No battery fitted yet.** Without it, a power cut or unplug stops the clock and loses the time. With it, an empty battery and no USB still means no alarm. | Missed alarm. | Bedtime warning (R24), load shedding to keep an alarm reserve (R25), "plug in" nag. Recommended use: on USB overnight. |
| K4 | **AMOLED burn-in** from a clock face shown all night. | Permanent ghosting. | Night face uses few pixels, pixel shift every minute, auto-off when idle, dim colours (Phase 2). |
| K5 | Shared I2C bus contention or lock-up (A2). | UI or settings freeze. | Mutex with timeouts, bus recovery (9 clock pulses), alarm path needs no I2C (D-002, D-008). |
| K6 | Accidental power-off or dismiss half-asleep. | Missed alarm. | R23, R27, IMU never dismisses. |
| K7 | Speaker is 8 Ω 2 W (safe with the on-board amp). Its size is still unknown. | Loudness and enclosure design. | Your answer is needed before Phase 1. |
| K8 | Flash wear from frequent NVS writes. | NVS failure years later. | Write only on settings change and alarm state transitions. Use RTC slow memory for per-second state. |
| K9 | RTC drift (crystal ±20 ppm ≈ ±1 min/month). | Alarm slowly drifts. | PCF85063 offset register calibration + phone/Wi-Fi sync (R16). |
| K10 | DST and time zone mistakes. | Alarm fires an hour off twice a year. | RTC holds UTC, POSIX TZ string, host-tested `next_alarm()` (D-003). |
| K11 | **Long-term ageing (a 30-year yardstick, not a promise):** the battery, the screen's pixels, flash memory and electrolytic capacitors all wear. | A part fails and takes the alarm with it. | Replaceable battery, a screen-independent alarm path, two program copies, long-life capacitors (LONGEVITY.md §4, §6). |
| K12 | **A daylight saving law changes.** | The alarm rings an hour off until corrected. | Manual region, `region.txt` on the SD card, a 2-touch fix (LONGEVITY.md §3). |

### Reference product: Dreamie (helloambient.com)

Ideas adopted: one-touch scenes with per-scene volume (R8), ultra-dark display that can hide the clock (R6), works with no phone (N5), noise colours (R11).
Not possible on this hardware: 120-LED directed light with joystick aiming, dedicated 2700 K warm channel (see K1), environment and motion sensors, Bluetooth headphone output.
