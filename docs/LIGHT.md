# Light plan

2026-11-13 · Decisions: [D-033](DECISIONS.md) · Requirements: R12, R13, R6, R43

This is the plan for every light the clock makes: the ring, and the screen's colour and brightness. It is built from published studies where I could read them, and it says where I could not.

## 1. What the evidence says (and how strong it is)

| Source | What I read | Finding |
|--------|-------------|---------|
| [Brown et al. 2022, PLoS Biology](https://journals.plos.org/plosbiology/article?id=10.1371/journal.pbio.3001571) (expert consensus) | Figure 2 caption in full; the rest through a summary I did not fully verify | Measured as melanopic EDI (the light that matters to the body clock): **daytime at least 250 lux** at the eye · **evening under 10 lux, starting 3 hours before bedtime** · **sleep period under 1 lux**. Evening light should be warm white with little short-wavelength (blue) content. The summary says warm white 2,700–3,000 K has a melanopic ratio under 0.35. |
| [Zhu, Luo and Zhou 2020](https://jps.ecnu.edu.cn/CN/Y2020/V43/I3/526), 21 people, abstract only | | A **30-minute exponential ramp from 0 to 210 lux** at the eye, ending at wake-up, improved alertness and mood after a short night compared with no light and with a constant 40 lux. Reaction times were faster. |
| [Campanella et al. 2024](https://pmc.ncbi.nlm.nih.gov/articles/PMC10969141), 29 people, summary | | A bedroom alarm with a sigmoid rise to 300 lux at about 4000 K, plus sound and temperature, showed **no overall difference** in reaction time, sleepiness or mood. Sleep efficiency was slightly lower (84.0 % against 86.7 %). The authors note all earlier studies used 30 minutes and that much shorter ramps may not help. |
| [Figueiro et al. 2019](https://pmc.ncbi.nlm.nih.gov/articles/PMC6506010), 30 people | | **Red light (628 nm) through closed eyelids** during sleep gave faster responses and fewer false starts on waking, with cortisol and melatonin unaffected. Eyelids pass red much better than other colours (reported as about 14.5 % at 700 nm against 3 % or less at 580 nm and below). |
| [Home study, 2020](https://mdpi.com/1422-0067/21/20/7736) (abstract) and [a 100 lux lab study, 2022](https://npr.org/sections/health-shots/2022/04/01/1089997121/light-disrupts-sleep) (press report) | | Light **during sleep** matters: about 5 lux cut melatonin and 1 lux increased sleep fragmentation in the home study; 100 lux raised overnight heart rate and next-morning insulin resistance. |

**How strong is this?** Modest. The dawn studies are small, and the one bedroom trial found no overall effect. What is solid is the other side: light in the evening and during sleep can do harm, and a gradual wake-up with no light in sleep costs nothing. So the plan makes sure the clock **never lights the sleeper**, and uses the best-supported wake-up shape. I did not find any study comparing warm against cool light on waking, so the daylight step in §3 is my judgement, not a finding.

## 2. Rules

1. **Dark in sleep.** From bedtime to wake the ring is off (0 lux). The screen is at its lowest level, amber only. Target: under 1 lux at the pillow.
2. **Thirty minutes of dawn.** The only length every study used. Fixed, not a setting.
3. **Exponential rise,** as in the 2020 study, because the eye responds to brightness on a log scale. It starts at a tiny fraction of the maximum.
4. **Start with red,** which gets through closed eyelids and barely touches the body clock. Then amber, then warm white.
5. **End bright and slightly cooler after the alarm,** while you are awake.
6. **Evening: warm and dim.** From 3 hours before bedtime the screen loses its blue and fades, and the ring's reading light is capped and warmer.
7. **Never leave a light on in sleep.** The night light switches off with the sleep timer or after 20 minutes.

## 3. The dawn sequence

T is the alarm time. The ring's output fraction F is a share of the maximum the cap allows (D-012, 0.9 A).

| Stage | When | Ring | F | Why |
|-------|------|------|---|-----|
| 1. Deep red | T−30 to T−15 | red only | 0.2 % to 4.5 % | Passes the eyelids, minimal effect on the body clock |
| 2. Amber | T−15 to T−5 | red, then green and a little white are added | 4.5 % to 35 % | Colour warms toward candlelight |
| 3. Warm white | T−5 to T | the white LED takes over, red and green fade to a tint | 35 % to 100 % | Reaches full warm white at the alarm |
| 4. Daylight | T to T+10 | white stays full, red goes, a little blue is added (aim for about 4000 K) | 100 % | Awake now: brighter and cooler. The 4000 K figure is from the 2024 trial. |
| Fade | after dismiss, T+10 min | fades over 60 s | | No light left on |

**The curve.** For T−30 to T: F = 0.002 raised to the power (1 − x), where x runs from 0 to 1 over the 30 minutes. That is a 500-fold smooth rise. It crosses 4.5 % at T−15, 12.6 % at T−10 and 35 % at T−5.

**Channel mix** (each channel is F times its share, then dithered so there are no visible steps):

| Time | Red | Green | White | Blue |
|------|-----|-------|-------|------|
| T−30 to T−15 | 100 % | 0 | 0 | 0 |
| T−15 to T−5 (k = 0 to 1) | 100 % | 40 % × k | 15 % × k | 0 |
| T−5 to T (k = 0 to 1) | 100 − 70 × k % | 40 − 25 × k % | 15 + 85 × k % | 0 |
| T to T+10 (k = 0 to 1) | 30 × (1 − k) % | 15 − 5 × k % | 100 % | 25 × k % |

Peak current is about 0.6 A (24 LEDs at 18 mA per channel), under the 0.9 A cap.

**Brightness.** Studies used 210–300 lux at the eye. This ring will not reach that at a bedside distance. My rough estimate, not a measurement, is 40–150 lux at 0.5 m. Bench check L1 measures it; the goal is as bright as the ring and the heat allow.

**Sound.** The alarm sound starts at T (D-015), after the light has peaked. The sleep sound (which came back before the dawn, D-044) fades out over about 3 minutes as the dawn begins.

**What I deliberately did not do:** a shorter ramp (the authors warn it may not help), cool white while asleep, a ramp triggered by sleep stage (needs a sensor this board does not have), or a setting for length (30 minutes is the evidence).

## 4. The evening and night

| Time | Screen | Ring (Light screen) |
|------|--------|---------------------|
| Day | cool-to-warm white by the hour, brightness setting | reading light up to 100 % |
| Wind-down: bedtime − 3 h to bedtime | blue drains out, brightness falls on an exponential curve, extras fade | reading light capped from 100 % down to 20 % and warmer (red and amber added to the white) |
| Night: bedtime to wake | amber only, lowest level, shifts a step a minute | reading light capped at 8 %, red and amber only. **Night light: red only, about 3 %.** Both switch off with the sleep timer or after 20 minutes |

**Evening target.** The consensus limit is 10 melanopic lux. With a warm-white ratio under about 0.35, that is roughly **25 lux of warm light at the eye**. Bench check L2 sets the evening cap by measurement.

## 5. Calibration (needs a lux meter; a phone is not reliable enough)

| ID | Measure | Sets |
|----|---------|------|
| L1 | Lux at the pillow (about 0.5 m) at T and at T+10 | Whether the dawn is bright enough, and the cap |
| L2 | Lux at the eye with the evening cap | The wind-down cap (about 25 lux) |
| L3 | Lux of the screen at its lowest level, and of the red night light | Night level (under 1 lux) |
| L4 | Smoothness: watch the first 5 minutes of the dawn in a dark room | Dithering is enough |
| L5 | Ring and case temperature after 40 minutes at T+10 | Heat limit (bench check M6 in POWER.md) |

## 6. Firmware notes

- `light_fx` holds the curve as a 64-point table, interpolates, and applies the 0.9 A cap before dithering.
- The face (screen) uses the same wind-down fraction as the ring's evening cap, so both fall together.
- The dawn is computed from the next alarm time and runs even if the UI is dead (D-002).
- Tests: T-12 (curve and cap, host test), T-13 (night and reading caps), T-46 (no ring output between bedtime and wake except the dawn).
