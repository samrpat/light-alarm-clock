# Reliability (Phase 3, living document)

How the alarm is kept from failing, and what happens when a part does. Grows as the firmware does.

## The rule
The alarm path is **only**: system clock → `next_alarm()` → `sm_step()` → a snapshot read by the light and sound tasks. It has no screen, touch, I2C or Bluetooth in it (D-002, R18). In the code: `main/alarm_engine.c` calls only the pure core and NVS, runs on core 1 at priority 20, and reads `time()`.

## Failure table (FMEA, short)
| Failure | What the alarm does | Where it is tested |
|---------|--------------------|--------------------|
| Power cut or reset during the dawn or the ringing | Resumes at the right dawn point or sound step; a snooze survives | T-21 |
| Off through the alarm time | Rings at once, softly, if within 60 min; otherwise not, and the next day is armed | T-20 |
| One settings copy corrupt | Uses the other; a power cut at any byte of a write leaves a good copy | T-17 |
| Both copies corrupt | Empty schedule plus a persistent warning, never a silent "no alarm" | T-17, store.c |
| Clock chip lost its time | Detected by the oscillator-stop flag; the UI asks for the time | T-15 (decode) |
| Clock set by hand | No false "missed alarm" from the jump | T-21 |
| Daylight-saving night | Skipped hour rings at the next valid minute; repeated hour rings once | T-28, T-21b |
| Nobody answers | Sound stops after 30 min, the light stays, the screen says missed, closed after an hour | T-22 |
| Task hangs | Task watchdog on the engine (10 s), panic reboots, state restored | bench T-19 |
| Screen, touch, speaker, ring, battery fail | Remaining outputs still fire; BOOT hold dismisses | bench T-41 |
| Bad `region.txt` or time-zone string | Parser rejects it (20,000 junk strings tested) and keeps the default | T-28b |

## Not yet done (honest list)
- Supervisor task, boot-loop guard and safe mode (R19).
- The BOOT-button dismiss (R23).
- Light and sound tasks that act on the snapshot; the RTC task that keeps the system clock right.
- Brown-out and low-battery behaviour (R21, R24, R25, R31).
