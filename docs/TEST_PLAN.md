# Test plan

Every requirement maps to a test. **Host** tests run on a computer with no hardware (`cd firmware/clock/test && make test`), under AddressSanitizer and UBSan. **Bench** tests need the board. Result column: ✅ passes today · ⏳ not yet written or run.

Last run: 2026-11-14 · 518,574 checks, 0 failed (`firmware/clock/test/test_main.c`).

| Test | Requirement | Kind | What it proves | Result |
|------|-------------|------|----------------|--------|
| T-01 | R1 | host | each weekday has its own time and on/off; every one of 1,440 minutes can be set | ✅ |
| T-02 | R2 | bench | a weekday change takes at most 3 touches and 10 s on the real UI | ⏳ (UI, step 3c) |
| T-03 | R3 | host | skip next, tomorrow-only, skip an override, overrides expire | ✅ |
| T-04 | R4 | host | the next alarm is the one that really fires; 1,500 random schedules compared with a minute-by-minute scan over decades | ✅ logic; ⏳ face text |
| T-10 | R10 | host | sleep sound fades smoothly: no step over 1 % a second, silent before and after the window | ✅ curve; ⏳ audio |
| T-12 | R12 | host | dawn curve: 0.2 % to 100 %, 4.5 % at T−15, no step over 0.35 % a second, peak about 626 mA | ✅ curve; ⏳ banding on the ring (UX-B1) |
| T-14 | R14 | host + bench | current estimate below 0.9 A for every moment of the dawn | ✅ estimate; ⏳ measured |
| T-15 | R15 | host | PCF85063 registers decode and encode 2000-2099; oscillator-stop flag read; impossible dates rejected | ✅ decode; ⏳ chip |
| T-16 | R16 | host | drift learning: ignores under 7 days and implausible values | ✅ |
| T-17 | R17 | host | A/B records: any single bit flipped is caught; power cut at every byte of a write always leaves a good copy; random bytes never produce an out-of-range schedule | ✅ logic; ⏳ real NVS |
| T-18 | R18 | host + bench | the alarm path uses no UI, I2C or BLE; the engine runs on core 1 | ✅ structure (engine reads only the system clock); ⏳ kill-UI test |
| T-20 | R20 | host | off through the alarm: rings at once, softly, within 60 min; not after | ✅ |
| T-21 | R21 | host | reboot during dawn, ringing or snooze resumes at the right stage | ✅ |
| T-22 | R22 | host | dawn then four sound steps a minute apart; sound gives up at 30 min, light stays, closes after an hour | ✅ logic; ⏳ speaker |
| T-23 | R23 | host | snooze 9 min × 3, a fourth tap does nothing, dismiss, dismiss during the dawn | ✅ logic; ⏳ BOOT hold |
| T-28 | R28 | host | all 1,184 daylight-saving transitions 2026-2099 in 10 regions equal those of Python's zoneinfo; skipped hour rings at the next valid minute; repeated hour rings once | ✅ |
| T-39 | R39 | host | every day 1970-2100 round-trips; leap days 2028-2056; 2038-01-19 03:14:08; weekdays; 64-bit | ✅ |
| T-47 | R44 | host | 1,249 random swipes, 0 mistaken for a dial turn (concept page) | ✅ concept; ⏳ real glass (UX-B2) |
| T-50 | R47 | host | pre-dawn sound: N minutes ending at the dawn start, fade in then out; none for "All night" | ✅ curve |
| T-19, T-24-27, T-29-T-33, T-36-T-38, T-41-T-46, T-48, T-49, T-51 | see REQUIREMENTS.md | bench / UI | watchdogs, battery, enclosure, UI, Bluetooth | ⏳ |

## Mutation check
To be sure the tests can fail, seven deliberate bugs were put into the code one at a time (wrong DST week, repeated hour resolved the wrong way, catch-up window ignored, snooze limit ignored, sequence wrap, dawn curve changed, skip ignored). Every one made the suite fail. The original code was restored and passes.

## How to run
```bash
cd firmware/clock/test && make test
```
Regenerate the independent daylight-saving data (needs Python 3.9+ and the system time-zone database): `python3 gen_expected_tz.py`. The firmware itself builds with `cd firmware/clock && idf.py build` (ESP-IDF v5.5.6).
