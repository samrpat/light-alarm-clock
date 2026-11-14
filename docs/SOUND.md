# Sound plan

2026-11-13 · Decision: [D-040](DECISIONS.md) · Requirements: R9, R10, R11

All sleep sounds are **made by the clock itself from maths**, not played from recordings. That means no files to lose, no licences, no loop point to hear, and nothing that can go missing over the years.

## The sounds

Tap the sound pill on the Sound screen and a ring of bubbles opens; pick one and it closes. Each sound has its own colour and picture, on two pages: Noise and Nature.

| # | Sound | What it is | How it is made |
|---|-------|-----------|----------------|
| 1 | **Pink noise** (default) | Equal energy per octave: a soft, even "shhh". The usual sleep choice. | White noise through a pink filter (−3 dB per octave) |
| 2 | White noise | Equal energy at every pitch: brighter, hissier | Random numbers |
| 3 | Brown noise | Deep rumble, like a distant waterfall | White noise, integrated (−6 dB per octave) |
| 4 | Green noise | Mid-range, like wind in leaves | White noise, band-pass around 500 Hz |
| 5 | Grey noise | Sounds equally loud at every pitch to the ear | White noise shaped against the ear's sensitivity |
| 6 | Blue noise | Bright and airy | Pink noise, differentiated (+3 dB per octave) |
| 7 | Violet noise | Very bright, a fine hiss | White noise, differentiated (+6 dB per octave) |
| 8 | Rain | Steady rain | White noise, 900 Hz to 9 kHz |
| 9 | Ocean | Slow waves | Brown noise swelling every 9 seconds |
| 10 | Wind | Gusting wind | Pink noise through a filter that wanders |
| 11 | Fan | A room fan | Brown noise, low-passed, with a quiet 110 Hz hum |

- **About "like TMSOFT's pink noise":** pink noise is a defined spectrum, so this is the same kind of sound. I could not compare it with their recording, and a small 2 W speaker has little bass, so it will be tuned by ear on the real speaker in Phase 3 (a tilt control per sound).
- Blue and violet are bright and many people find them harsh for sleep. They are included because you asked for the full set.
- The ambient sounds are simple syntheses. They will sound like "noise shaped like rain", not a field recording. If that is not good enough, recordings on the SD card are the fallback (Phase 5).
- The concept page plays all eleven for real (tap play on the Sound screen).

## Nature sounds added in round 6

| Sound | What it is | How it is made |
|-------|-----------|----------------|
| Storm | Rain with distant thunder that rolls in at random, every 12 to 26 s | Rain texture plus low-passed noise swelling and rippling |
| Stream | A babbling brook | Soft noise plus about 55 tiny rising "bubble" tones a second at random pitches |
| Waterfall | A broad, steady roar | Pink and brown noise together |
| Forest | Leaves with a hint of night insects | Quiet high noise plus two fluttering tones near 4 kHz |
| Fire | A fireplace: a low rumble and random crackles and pops | A quiet rumble plus about 6 random clicks a second, some louder, some in clusters |

Sixteen sounds in all. Like the others they are shaped noise, not recordings: good for masking and sleep, not a replica of a real forest. Recordings on the SD card stay the fallback (Phase 5).

## Rules

- Volume, sleep timer and the chosen sound are remembered.
- Every start, stop and change fades (0.4 s in, 30 s out at the timer, 2 minutes out at the dawn). No clicks.
- A 150 Hz high-pass protects the small speaker (HARDWARE.md §4). Sleep sounds are capped at about 25 % of full output.
- Sound runs on the alarm-safe core and never blocks the alarm (D-002).

## Firmware notes

A 32-bit generator makes white noise at 22 050 Hz. Pink uses a 7-stage filter (Paul Kellet's coefficients, as in the concept page). Brown and violet are one-line filters. Green, grey, rain, wind and fan add one to three biquad filters. Ocean and wind add a slow oscillator. All of it costs well under 1 % of one core (my estimate, to measure).

## Round 7: rebuilt rain, storm, stream and fire

The first versions of fire, storm and stream were shaped noise with a wandering filter, and sounded wrong ("horrible"). They are now built from **random events**, like the real thing: crackles, raindrops, bubbles and thunder claps placed at random times. Each is generated once into a loop 10 s long (storm 40 s) with no repeat you can hear.

**Honest limit:** I cannot listen. I checked that each builds without errors, loops, has the right event rate and level balance, but whether it *sounds* good is for your ears. Please try Fire, Storm, Stream and Rain in the concept and tell me what is wrong ("too crackly", "thunder too loud", "stream too hissy") and I will adjust the numbers. These are likely to need tuning on the real 2 W speaker too.
