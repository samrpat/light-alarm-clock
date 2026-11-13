# Built to last: offline and repairable

2026-11-13 · Decisions: [D-030](DECISIONS.md), [D-031](DECISIONS.md), [D-035](DECISIONS.md) · Requirements: R16, R28, R38–R42

**The yardstick:** like an old Nokia. "30 years" is a measure of quality the owner wants, not a promise or something to be proven. It does one job, needs nothing from the outside world, and can be fixed by whoever owns it. No warranty covers 30 years of electronics, so the goal is honest: nothing in the design is *guaranteed* to expire, every part that will wear out can be replaced, and a failure of one part never takes the alarm with it.

## 1. Standalone rules

1. **No network, ever.** No Wi-Fi, no Bluetooth, no time server, no cloud, no account, no certificate, no app. The radios are never switched on (D-030). Servers, certificates and phone apps are the first things to disappear over 30 years.
2. **Everything is settable on the device.** Time, date, region, sleep hours, volume, brightness.
3. **No clock needs a server to be right.** It is set by hand and keeps its own time (§2).
4. **Updates need no network.** A file on the SD card, or a cable to any computer (§5).
5. **Everything needed to rebuild is kept offline** (§5).

This removes from the plan: Wi-Fi time sync, over-the-air updates, and the phone link (old D-010, N1–N3).

## 2. Time

| Topic | Design |
|-------|--------|
| Source | The PCF85063 clock chip, kept on the battery. The firmware keeps UTC and converts to local time. |
| Drift | A 32 kHz crystal is typically about 20 parts per million: up to roughly 1.7 s a day, about 50 s a month, and it ages. Nothing outside corrects it, so the clock **learns its own drift**: when you correct the time after a week or more, it works out the error and trims the chip's offset register (one step is about 4 ppm; my reading of the datasheet, to verify). |
| Setting | **Time** and **Date** are the first two pages in settings. After a power loss with no battery, the clock says "Set time" and the alarm waits for it. |
| Year 2038 | ESP-IDF 5 uses a 64-bit time value, so 2038 is not an issue. Phase 3 adds a build check (`sizeof(time_t) == 8`) and a test. |
| Leap years | The firmware computes dates itself. Tests cover every leap day from 2028 to 2056 and a February 29 alarm. |
| Year range | The chip stores two digits of year, so it is correct to 2099. The date page stops at 2099. |
| Weekday | Computed by the firmware, never read from the chip. |
| Lost time | The chip's oscillator-stop flag marks the time as untrusted (D-003). |

## 3. Daylight saving, without a server

**The problem:** the rules are law and laws change. A clock that learned them from the internet would stop working when the server does. A clock with the rules fixed inside it is wrong the day the law changes.

**The answer has four layers, none needing a network:**

1. **Rules stored in the clock.** Each region is a rule, not a list of dates, so it works for any year to 2099: US since 2007 (second Sunday of March at 2 AM to first Sunday of November at 2 AM), the UK, and the rest of Europe (last Sunday of March to last Sunday of October), and regions without daylight saving. Written as standard time-zone strings such as `EST5EDT,M3.2.0,M11.1.0`.
2. **A Manual region.** If the law changes, choose **Manual** in Region: the clock never changes itself, and you move it an hour by hand. That is **hold the clock, tap the hour zone** (or flick the rim), two touches, because Time is the first settings page.
3. **A file on the SD card** named `region.txt` holding one time-zone string, for a rule the clock does not know. It is read at start-up when Region is set to Custom. Nothing else is needed.
4. **Alarms are stored as wall-clock times** ("6:45"), so they keep their meaning across a change. A time that does not exist (spring forward) rings at the next valid minute, and a time that happens twice (fall back) rings once (D-003).

**Options considered**

| Option | Verdict |
|--------|---------|
| Time from Wi-Fi (NTP) | Rejected: needs a server and a network for decades |
| GNSS satellite receiver | Not on this board (only the "-G" variant has one). Possible as an optional part later. |
| Radio time signal (like WWVB) | Rejected: depends on one broadcaster, and the board has no receiver |
| **Rules inside + Manual + SD file** | **Chosen** |

**If the law changes and you do nothing,** the alarm rings an hour off until you notice the clock is wrong and fix it, which takes two touches. The face always shows the time, so it is visible.

Tests: every transition from 2026 to 2056 for every region, on a computer, and the two nights around each.

## 4. Hardware that wears out

| Part | Risk over the long run | What we do |
|------|--------------------|-----------|
| **Li-Po battery** | Lasts roughly 3–5 years and can swell with age | A **consumable**: the bay has a door, the cell is never glued, and the clock works without it (R36). Replace every 3–5 years. Write the date on the cell. |
| **AMOLED screen** | Pixels dim and can burn in; lifetime is not specified | Mostly black UI, the face shifts every minute, night level is very dim, the day arc moves. If the screen ever fails, the light, the sound and a BOOT-button dismiss still work. |
| **Flash memory** | Stored data can fade over decades; I did not read this chip's retention figure | Two copies of the program, a check at every start that repairs a bad copy from the good one, two copies of the settings with checksums. Few writes. |
| **Ring supply capacitor** | Electrolytic capacitors dry out | Choose a 105 °C long-life or polymer type, rated well above 5 V, kept away from heat. |
| **Clock crystal** | Drifts | Learns its own drift (§2). |
| **USB-C socket** | Wear | Strain relief in the case. |
| **LEDs** | Dim slowly | They run rarely and at low power. |
| **Speaker** | Cones last decades | Nothing to do. |
| **Plastic case** | Yellowing, cracking | PETG, no glued parts, screws into heat-set inserts. |

## 5. Rebuilding and updating with no network

**The build environment is a risk too:** the tools and libraries are downloaded from servers when you build, and those servers may be gone.

- **Pin everything** (done: ESP-IDF v5.5.6).
- **Keep every library in the repository.** The board support package, LVGL, the LED driver and the rest are copied into `managed_components` and committed (today that folder is not committed; Phase 3 changes this).
- **A "long-life kit"** (Phase 7): one folder to copy onto two USB sticks or SD cards, with the ESP-IDF source and tool archives, the Python packages, the flashing tool, the fonts, the licences, this documentation printed to PDF, the built firmware files, and a list of checksums. A rebuild test every five years confirms it still works.
- **Update from the SD card:** the clock looks for `clock-update.bin` at start-up, checks it, writes it to the spare program slot, and rolls back if the new program does not pass its self-test. No tool, no network. A USB cable and the flashing tool is the second way.
- **Settings export:** the settings are copied to the SD card on request, so a replacement board can be brought up in a minute.
- **A spare:** keep one spare board, ring and battery in a drawer, flashed.

## 6. Nokia-style failure behaviour

| If this fails | The alarm still... |
|---------------|--------------------|
| Screen | rings, with the light, and stops with the BOOT button |
| Touch | rings, and stops with the BOOT button |
| Speaker | lights the ring and the screen at full |
| Ring | rings with sound and the screen at full white |
| Battery | works on USB; a power cut stops it until the time is set |
| Settings memory | uses the second copy, then an empty schedule with a warning |
| The program | the second copy starts, then an alarm-only safe mode (D-006) |
| The SD card | nothing; it is only used for updates |

## 7. Tests to add (Phase 3)

| ID | Test |
|----|------|
| T-38 | Runs 30 days with no network and keeps time within the drift limit |
| T-39 | Date maths: every leap day 2028–2056, year 2038, year 2099 |
| T-40 | Daylight saving: every transition 2026–2056 for each region, alarms around each |
| T-41 | Corrupt either program copy or either settings copy: the clock still boots and rings |
| T-42 | Update from SD with a good file and a bad file (bad file rejected, rollback works) |
| T-44 | Pull the display, touch and speaker one at a time: the alarm still works |
| T-45 | 1,000 power cuts with the battery fitted and with it removed |

## 8. What I cannot promise

The screen, the battery and the flash are parts whose real long-term life I could not confirm from a datasheet. The design is built so that none of them is a single point of failure for the alarm, and so that each can be replaced.
