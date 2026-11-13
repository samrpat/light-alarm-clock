# Bring-up test

A small test program for trying the hardware. It is **not** the alarm clock firmware (that starts in Phase 3).

## What it does
Tap the screen to step through:

| Mode | You should see / hear |
|------|-----------------------|
| Idle | A running timer on the screen. Ring dark. Silent. |
| Sunrise | The ring goes dark → red → amber → warm white over 20 s, then repeats. |
| White light | The white LEDs on, dimmed to stay under the current cap. |
| White noise | Hiss from the speaker. |
| Tone 440 Hz | A steady note from the speaker. |

The screen also shows the ring's estimated current and the USB voltage the power chip reports.
The ring is held to **300 mA** in this test because the quick wiring has no fuse.

## Wiring for the quick test (3 wires)

| Ring hole | Waveshare 8-pin header |
|-----------|------------------------|
| PWR / 5V | VBUS |
| GND | GND |
| IN / Data In | 17 |

- The ring's connections are plated holes. Solder a wire into each one. A wire only pushed through a hole makes poor contact and causes flicker.
- Keep the wires under about 10 cm. There is no level shifter in this quick test, so longer wires may cause flicker or wrong colours.
- Plug the speaker's lead into the board's speaker socket.
- Leave the battery out unless you have checked its plug polarity with a multimeter.

## Build and flash

One-time setup is already done on this Mac (ESP-IDF v5.5.6 in `~/esp/esp-idf`).

Plug the board into the Mac with a USB-C **data** cable, then in a terminal:

```bash
. ~/esp/esp-idf/export.sh
```

```bash
cd ~/Documents/Projects/Claude/alarm/firmware/bringup && idf.py flash monitor
```

If the board is not found, hold **BOOT**, tap **PWR** (or re-plug USB), release **BOOT**, and run the command again.
Press `Ctrl+]` to leave the monitor.

## What to send back
The first 30 lines of the monitor output. They list which chips answered on the I2C bus and which pins the board package uses for audio, which settles several of the bench checks in `docs/HARDWARE.md` §6.
