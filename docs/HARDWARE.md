# Hardware

Phase 1 · 2026-11-12 · Power reasoning is in [POWER.md](POWER.md).

Sources for board facts: Waveshare's `HARDWARE_REFERENCE.md` and `pin_config.h` in
`github.com/waveshareteam/ESP32-S3-Touch-AMOLED-1.75`, and the Waveshare wiki.
**The schematic PDF itself has not been read yet.** Items marked ⚠ must be checked on the real board (§6).

## 1. Bill of materials

Prices are approximate (USD, late 2026) and not checked against a live listing.

| # | Part | Part number / source | Qty | ~Cost | Status |
|---|------|----------------------|-----|-------|--------|
| 1 | Waveshare ESP32-S3-Touch-AMOLED-1.75 | Waveshare SKU 31261 | 1 | $30 | Have |
| 2 | NeoPixel Ring 24× RGBW | Adafruit #2861 (3000 K), #2862 (4500 K) or #2863 (6000 K) | 1 | $20 | Have (which one?) |
| 3 | Speaker, 8 Ω 2 W | owner's | 1 | — | Have (size unknown) |
| 4 | Level shifter 74AHCT125, DIP-14 | Adafruit #1787, or TI SN74AHCT125N | 1 | $1.50 | Buy |
| 5 | Resettable fuse, 1.1 A hold | Bourns MF-R110 | 1 | $0.50 | Buy |
| 6 | Electrolytic capacitor 1000 µF, 10 V or higher | e.g. Panasonic EEU-FR1A102 | 1 | $0.60 | Buy |
| 7 | Resistor 470 Ω, ¼ W | any | 1 | $0.05 | Buy |
| 8 | Resistor 10 kΩ, ¼ W | any | 1 | $0.05 | Buy |
| 9 | Ceramic capacitor 100 nF | any | 1 | $0.05 | Buy |
| 10 | Li-polymer cell 3.7 V 2000 mAh, 103450, with protection, MX1.25 2-pin | generic; **check polarity** | 1 | $9 | Buy (recommended) |
| 11 | USB-C power adapter 5 V 3 A | e.g. Raspberry Pi 15 W USB-C supply | 1 | $8 | Buy if you have none |
| 12 | MX1.25 2-pin pigtail (for the speaker) | generic 1.25 mm "PicoBlade-style" lead | 1 | $1 | Check the board's box first |
| 13 | Small perfboard, ~30 × 40 mm | any | 1 | $1 | Buy |
| 14 | Wire: 22 AWG (ring power), 26–28 AWG (signals); 1×8 female header housing or Dupont leads | any | — | $3 | Buy |

Baseline total to buy: about **$25**.

## 2. Pin table

### Used by this project

| Function | Pin | Notes |
|----------|-----|-------|
| **NeoPixel data** | **GPIO17** (header pin 6) | Through 74AHCT125. RMT output. |
| Spare (reserved: boost enable in the optional upgrade) | GPIO18 (header pin 7) | Unused in baseline. |
| Spare | GPIO16 (header pin 8) | ⚠ Left unused until the MCLK question is settled (§6). |
| Ring 5 V | VBUS (header pin 1) | Through PTC fuse. |
| Ring / logic ground | GND (header pin 2) | |
| Debug UART | GPIO43 TX, GPIO44 RX (header pins 5, 4) | Optional. USB serial also works. |
| Dismiss fallback button | GPIO0 (BOOT) | Low when pressed. Direct GPIO interrupt. |

### On-board (fixed by Waveshare)

| Function | Pins |
|----------|------|
| Display CO5300, QSPI | CS 12 · SCLK 38 · D0–D3 4, 5, 6, 7 · RESET 39 · TE 13 |
| Touch CST9217 (I2C 0x5A) | INT 11 · RST 40 |
| I2C bus (shared by everything) | SCL 14 · SDA 15 |
| I2S audio | BCLK 9 · LRCK 45 · DOUT→ES8311 8 · DIN←ES7210 10 · MCLK 42 ⚠ |
| Amplifier enable (NS4150B) | GPIO46, direct |
| SD card (1-bit SDMMC) | CMD 1 · CLK 2 · D0 3 · (D3 41 unused) |
| IMU QMI8658 (I2C 0x6B) | INT2 → GPIO21 · INT1 → expander P6 |
| USB | D− 19 · D+ 20 |

### I2C addresses

ES8311 0x18 · TCA9554 0x20 · AXP2101 0x34 · ES7210 0x40 · PCF85063 0x51 · CST9217 0x5A · QMI8658 0x6B

### TCA9554 expander

| Pin | Function | Direction |
|-----|----------|-----------|
| P0–P2 | Free (test points) | — |
| P3 | RTC interrupt | in |
| P4 | PWR button state (high while pressed) | in |
| P5 | AXP2101 interrupt | in |
| P6 | IMU INT1 | in |
| P7 | GNSS reset (‑G variant only) | out |

What this means for the alarm path (D-002): the amplifier enable is a **direct GPIO**, so sound needs no I2C. The RTC and PMIC interrupts arrive **only through the expander**, so firmware polls them once a second and never depends on them to fire an alarm.

## 3. Wiring

```
 BOARD 8-PIN HEADER                                   NEOPIXEL RING (RGBW)
 ──────────────────                                   ────────────────────
 pin 1  VBUS ──┬──[ PTC 1.1A ]──┬────────────────────► 5V / PWR
               │                │(+)
               │            [1000µF]
               │                │(−)
 pin 2  GND ───┼────────────────┴────────────────────► GND
               │
               │        74AHCT125 (DIP-14)
               ├───────► pin 14  VCC          [100nF between pin 14 and pin 7]
 GND ──────────┼───────► pin 7   GND
 GND ──────────┼───────► pin 1   /OE1 (low = enabled)
               │
 pin 6 GPIO17 ─┼──┬────► pin 2   A1
               │ [10k]
               │  │
 GND ──────────┼──┘      pin 3   Y1 ───[ 470Ω ]──────► DIN (Data In)
               │
               └───────► pins 4, 10, 13 (/OE2–4) to VCC = unused gates off
 GND ──────────────────► pins 5, 9, 12 (A2–A4)

 SPEAKER:  board MX1.25 "SPK" connector ── 2 wires ── speaker terminals
 BATTERY:  board MX1.25 "BAT" connector ── LiPo 2000 mAh (check polarity first)
 USB-C:    5 V 3 A adapter into the board's USB-C port
```

Build notes:
- Put the 1000 µF capacitor and the 470 Ω resistor **close to the ring**, not at the board end.
- Mind the capacitor's polarity: the striped side goes to GND.
- Use 22 AWG for the two ring power wires. Keep them and the data wire under about 15 cm.
- Wire to the ring's **Data In** pad. Data Out stays unconnected.
- The ring's ground and the board's ground must be joined (they are, through header pin 2).

## 4. Speaker (8 Ω, 2 W)

- **Amplifier:** NS4150B, a 3 W-class mono class-D chip with a bridged output. Into 8 Ω it can deliver about 1.4 W from a 5 V rail, or about 0.8 W from a battery-level rail. ⚠ Which rail feeds it is not stated by Waveshare (bench check M5).
- **Safety:** either way the amplifier cannot reach the speaker's 2 W rating, so it cannot burn out the speaker.
- **Wiring:** two wires from the board's MX1.25 speaker connector to the speaker. **Neither wire is ground.** Do not connect either speaker terminal to GND or to anything else. Polarity does not matter with a single speaker.
- **Volume limits in firmware (D-008):**
  - Codec gain fixed at boot at the highest setting that does not clip at full-scale input (set by ear and with a scope or phone spectrum app during bring-up).
  - Alarm: up to 100 % of that. Floor at 30 % so it can never be set silent.
  - White noise and sleep sounds: capped at about 25 % (−12 dB).
  - A 150 Hz high-pass filter on everything. A small speaker cannot reproduce lower notes; sending them only wastes power and makes it rattle.
- **Still needed for the enclosure (Phase 4):** the speaker's diameter and depth. A ruler measurement is enough.

## 5. Enclosure-relevant dimensions known so far

| Item | Size |
|------|------|
| RGBW ring | 65.5 mm outer, 52.2 mm inner, 3.25 mm thick |
| Display active area | 1.75" ≈ 44.5 mm across |
| Battery 103450 | 10 × 34 × 50 mm |
| Board outline and mounting holes | ⚠ Not given in text by Waveshare. Measure the board or use their dimension drawing in Phase 4. |

Note for Phase 4: the ring's 52.2 mm opening is wider than the 44.5 mm screen, so the ring can sit around the display as a halo, if the board's outer body fits inside 52 mm. That has to be measured.

## 6. Checks to do on the real board before Phase 3

| # | Check | How | Why |
|---|-------|-----|-----|
| V1 | I2S MCLK is GPIO42, and GPIO16 is free | Run Waveshare's audio demo; scope or logic-probe GPIO16 and 42 | Their two sources disagree. |
| V2 | Battery connector polarity | Multimeter on the plug vs the "+" silk mark | Reverse polarity kills the PMIC. |
| V3 | Header VBUS pin really carries USB 5 V and holds ≥ 4.6 V at 0.9 A | POWER.md M2 | Trace and any series part are unknown. |
| V4 | Amplifier supply rail | POWER.md M5 | Sets real maximum loudness. |
| V5 | Board runs from USB with no battery, and from battery with no USB | Try both | Assumed, not stated by Waveshare. |
| V6 | RTC keeps time across a USB unplug with the battery fitted | Unplug for 10 min, compare | R15. |
| V7 | PWR-key long-press power-off can be disabled | Write the AXP2101 setting, hold PWR 15 s | R27. |
| V8 | Which ring you have (#2861 / #2862 / #2863) and its colour order | Light W only; light R only | Sunrise colour curves depend on it. |

If V3 fails (VBUS sags or the pin gets warm), the fallback is a separate USB-C breakout in the case that feeds the ring directly, with the board plugged into the same supply.
