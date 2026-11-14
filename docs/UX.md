# UX specification

Phase 2 (revised 2026-11-13, round 3) · Decisions: [D-011](DECISIONS.md), [D-015](DECISIONS.md), [D-016](DECISIONS.md), [D-024](DECISIONS.md), [D-028](DECISIONS.md), [D-032](DECISIONS.md), [D-036](DECISIONS.md) to [D-039](DECISIONS.md)
Interactive concept (touch only, Ubuntu type, 401 checked screen states): `/mockups/ux-concept.html`, published at https://claude.ai/artifact/UGtnZ6MqkrSfuM5BShyG5a (private link).
Light and colour: [LIGHT.md](LIGHT.md). Offline and repairable: [LONGEVITY.md](LONGEVITY.md). A test app for the real touch screen: `/firmware/dialtest`.

## 1. Principles

1. **Four gestures, four jobs, no overlap.**

   | Gesture | Its one job |
   |---------|-------------|
   | **Swipe** | **Go** to a neighbour |
   | **Tap** | **Do** something: play, turn on, snooze, open, pick |
   | **Drag a dot** | **Change** a value. The only way to change one. |
   | **Hold** | **Switch an alarm**: a day on or off, skip tomorrow or bring it back, stop the ringing. Also opens settings. |

2. **No buttons.** No arrows, no plus and minus, no tap zones. A value changes only by dragging its dot.
3. **Sideways always means the next neighbour.** Light | Clock | Sound. Monday | Tuesday | … The settings pages. Up and down move between the waking screens (Week, Clock, Tomorrow).
4. **Whether an alarm rings is only changed on purpose.** Turning a day on or off, skipping tomorrow or bringing it back, and stopping the ringing all need a **hold**. One rule both ways, so a stray tap can neither silence tomorrow nor add an alarm you did not want. Snooze stays a tap. With no vibration motor, a hold shows itself: the rim fills from the first moment and the whole screen presses in slightly until it completes.
5. **Few words.** At most **four lines of text** on any screen (the week excepted). Icons and layout carry the meaning.
6. **Nothing to confirm, nothing to save.** Every change applies at once. Any sub-screen returns to the clock after 20 s of no touch.
7. **A touch is only ever one thing.** A dial turn starts only on a dot, after a brief hold, moving along the ring. Anything else is a swipe, a tap or ignored. This is measured, not assumed (§4.2).
8. **No text collides.** An automatic check measures every screen state in the real font (§11).

## 2. Physical facts

| Fact | Value |
|------|-------|
| Screen | 466 × 466 px, round, about 44.5 mm across, so **10.5 px per mm** |
| Fingertip | about 10 mm, about **105 px** |
| Dots | **Outer dot (minutes, and all single values): ring radius 188 px, dot 42 px across. Inner dot (hours): ring radius 148 px, dot 32 px across.** A dot is grabbed from up to 56 px away. |
| Safe text circle | radius **209 px**. Every text and icon box sits inside it. |
| Smallest text | **26 px** |
| Why not the very edge | the case may hide the last few millimetres, so the outer ring sits at 188 px, inside the visible area |

## 3. Gestures

| Gesture | Definition | Feedback |
|---------|-----------|----------|
| **Tap** | Under 0.4 s, moves under 16 px | Soft ripple |
| **Swipe** | Moves at least 56 px, one dominant axis, starting **anywhere**, including on the rim and next to a dot | Screens slide 0.24 s |
| **Drag a dot** | Touch within 56 px of a dot, **keep still for at least 90 ms**, then move so that the first 16 px follow the ring (tangent), not toward or away from the centre | The dot follows, the value updates live |
| **Hold** | Full time, moves under 16 px. The rim fills clockwise after 0.15 s. Letting go early cancels. | Rim fills |

- **What a dot does:**
  - **Minutes (outer dot, the "minute hand"):** 5 minutes per 30° of travel, 12 steps a lap. Passing the top moves the hour on by itself.
  - **Hours (inner dot, the "hour hand"):** 1 hour per 30°. It steps through all 24 hours, so AM and PM flip on their own and cannot be entered wrong.
  - **Volume and brightness (one dot on an open 270° arc):** the dot follows the finger, in steps of 5 %.
  - **Date and Region:** one step per 30°. On the Date page, tap a number to pick month, day or year first.
- **Hold times:** turn a day off or skip tomorrow 1.0 s · open settings 1.5 s · start a preview 1.5 s · dismiss the alarm 2.0 s. A hold that starts on a dot is never a hold.
- Movement of 16–56 px that is not a dial turn is ignored. A second finger is ignored. No sound on touch.
- **Which screens have dots:** Tomorrow, Day and Time (hour and minute dots), Bedtime (a moon dot), Sound, Light, Brightness and Alarm (one dot on an arc), Date and Region (one dot).

## 4. Options considered

### 4.1 Changing a value
| # | Method | Verdict |
|---|--------|---------|
| A | Drag around the rim, from anywhere in an outer band | **Rejected after measuring.** In a test, 108 of 179 sideways swipes that began in the outer band (60 %) were mistaken for dial turns. |
| B | Drag up/down anywhere | Rejected: the vertical axis already means "go". |
| C | Tap above and below the number (buttons, zones, arrows) | **Removed.** Having it beside the dial made three overlapping controls. |
| **D** | **Drag a dot on a ring, like the hands of a clock** | **Chosen.** One direct way. A dial turn can only start on a visible dot, so nothing else can be mistaken for it. Hours and minutes have their own dots, so a big change is one short drag, not many laps. |

The earlier weighted scores (A 50, B 55, C 84, D 88, E 58 out of 95) came before the owner chose the dial and are superseded.

### 4.2 Measured: swipes against dial turns
The concept page includes a test that fires thousands of simulated touches. On 2026-11-13, with the final rules:

| Test | Result |
|------|--------|
| Random swipes in all four directions, starting anywhere (651 of them within 56 px of a dot), on 10 screens that have dots | **1,249 swipes, 0 mistaken for a dial turn** (re-run after the colour redesign: 983 swipes, 0 mistaken) |
| Deliberate grab (150 ms), drag outer dot 60° | time +10 min, as designed |
| Deliberate grab, drag inner dot 60° | time +2 h |
| Swipe left or right along the top rim | moves to the next neighbour |
| Instant flick that starts on a dot | a swipe, time unchanged |
| Hold in the centre, tap in the centre | stop and bring back, as designed |

Simulated touches are not a real finger. **Test UX-B2 repeats this on the real panel** with `/firmware/dialtest`.

### 4.3 How screens connect
**Cross:** clock in the centre. Up and down for waking, left and right for sleeping, and sideways always means the next neighbour (principle 3). A row of screens or an icon home screen were rejected.

### 4.4 Day and night
Not a switch: the face changes continuously from wake to bedtime and winds down (§7). If the time is untrusted, the night level is used.

## 5. Screen map

```
                      [ Day ]             tap a day on the Week screen
                         |
                     [ Week ]             swipe DOWN from the clock    (waking)
                         |
 [ Light ] ---------- [ CLOCK ] ---------- [ Sound ]
  swipe RIGHT             |              swipe LEFT                     (sleeping)
                     [ Tomorrow ]         swipe UP from the clock       (waking)

 Settings: hold the clock for 1.5 s. Seven pages side by side.
```

| Screen | Reached by | Sideways | Back by | Auto-return |
|--------|-----------|----------|---------|-------------|
| Clock | home | Light / Sound | | |
| Tomorrow | swipe up | none | swipe down | 20 s |
| Week | swipe down | none | swipe up | 20 s |
| Day | tap a day on Week | previous / next day | swipe up | 20 s |
| Sound | swipe left | back to the clock | swipe right | 20 s (the sound carries on) |
| Light | swipe right | back to the clock | swipe left | 20 s (the light carries on) |
| Settings | hold the clock 1.5 s | previous / next page | swipe down | 30 s |
| Ringing | automatic | none | tap = snooze, hold 2 s = stop | |

## 6. Screens (text budget in brackets)

### 6.1 Clock (home) [2–3 lines]
- **The time is the hero:** Ubuntu Light 124 px. Above it the date (day only). Below it a small bell and the next alarm at 26 px.
- **The alarm line shows only the time** ("6:45 AM") when the alarm is within the next 24 hours. The day appears ("Fri 6:45 AM") only when the next alarm is further away, because then it matters.
- A status line appears only when something needs saying.
- **Tapped (5 s):** five small icons: calendar above (Week), bell below (Tomorrow), **lamp** lower left, speaker lower right, and sliders for settings. Nothing pulses behind the text.
- **Dawn:** the face does not jump to bright. From 30 minutes before the alarm the sunrise colour fades in behind the time on the same curve as the ring, and at the alarm it has become the ringing screen.

### 6.2 Tomorrow (swipe up) [title, time, one state word]
```
        ·  ·  ·  ·  ·
      ·   ( )  outer dot = minutes
     ·       Tomorrow     ·
    ·    6  :  00         ·     inner dot = hours
    ·        AM           ·
     ·     Once only     ·
      ·  ·  ·  ·  ·  ·
```
Two rings of twelve small dots, like a clock face, with two dots on them.

| State | Under the time | Gestures |
|-------|----------------|----------|
| Usual | nothing | drag a dot to change tomorrow only · **hold 1 s = skip** |
| Changed | "Once only" (teal) | dragging back to the usual time clears it |
| Skipped | "Skipped" (red), time struck through | **hold 1 s = bring it back** |
| None | "Drag a dot to set one" (dots dim, at 7:00) | drag a dot |

### 6.3 Week (swipe down)
Seven wedges (day, time or "off"), the next alarm in the middle. Tap a wedge to open that day (about 14 mm wide at the finger).

### 6.4 Day (tap a day on Week) [title, time, "Off" when off]
Title is the day's name. Same two dots. **Hold 1 s = on or off** (dragging a dot also turns the day on). Swipe sideways for the next or previous day: a day that is off starts at the time you just set (carry-over).

### 6.5 Sound (swipe left) [title, one pill]
The title is the sound's name, and the screen takes its colour. Ten bars show the volume and dance while it plays. Drag the dot for volume. Tap the icon = play or pause. Tap the pill = sleep timer. **Swipe up or down = next sound**: seven noise colours (pink first) and four ambient sounds ([SOUND.md](SOUND.md)). A column of small marks at the left shows which one.

### 6.6 Lamp (swipe right) [title, one number]
A big percentage or "Off" and a two-icon switch (moon = night light, sun = reading light). Drag the dot for brightness. Tap the number = on/off. Tap the switch = change mode. Caps and colours follow the time of day (LIGHT.md §4).

### 6.7 Dawn and ringing [3 lines]
- **Dawn:** the clock with "Sunrise · alarm 7:00 AM". The ring follows LIGHT.md §3. **Hold 1.5 s** anywhere stops it for today.
- **Ringing:** the time, "Tap to snooze", "Hold to stop", three dots for snoozes left. **Tap anywhere = snooze 9 min** (3 times). **Hold 2 s = stop.** BOOT held 3 s stops the alarm even if the screen is dead.
- After 30 minutes the sound stops and the face says "Alarm 7:00 AM timed out" until touched (D-015).

### 6.8 Settings (hold the clock 1.5 s): seven pages, each a picture of what it sets
Swipe sideways between pages, drag a dot to change, swipe down to leave. A row of small pills shows the page. Each page has its own colour.

| # | Page | Colour | What you see | What you do |
|---|------|--------|--------------|-------------|
| 1 | **Time** | blue | A watch face: two rings, 60 fine ticks, the time | Drag the outer dot (minutes) and the inner dot (hours). Opens by itself after a power loss. |
| 2 | **Date** | red | A calendar page: month, a big day number, year | Tap the line you want, then drag the dot |
| 3 | **Region** | green | The region, its UTC offset, daylight saving on or off. **The ring has exactly one mark per region**, so the dot's position is the choice. (Date works the same: 12 marks for months, 31 for days.) | Drag the dot: US Eastern (default), Central, Mountain, Pacific, Alaska, Hawaii; UK; Central Europe; **Manual**; Custom (SD card) |
| 4 | **Bedtime** | violet | A violet band of night on the clock ring, starting at a moon dot, with the 3-hour wind-down fading in before it. The band ends at an **orange bell: your alarm**. Text: bedtime, "Wind down 7:30 PM", "Alarm 6:45 AM". | Drag the moon (15-minute steps, default 10:30 PM). The bell cannot be moved here. |
| 5 | **Day brightness** | gold | An orb that grows and glows, and the words "Night stays dim" | Drag the dot (default 60 %). It sets the daytime face only. At night the face is always at the lowest level at which the clock is still readable (set once on the real panel, UX-B1). |
| 6 | **Alarm** | orange | A bell with sound waves, the volume, "Tap to test" | Drag the dot for volume (30–100 %, default 70 %). **Tap the bell to test:** the ring lights and the alarm sound plays at that volume for 6 s. Tap again to stop. |
| 7 | **Battery** | mint | A ring that shows the charge, **the percentage large, the voltage small**, and the version | Nothing |

- **Bedtime and alarm are different things and look different.** The **alarm** is when the clock wakes you: orange, a bell, set on the Tomorrow and Week screens. **Bedtime** is when the face goes dark: violet, a moon, set here. There is **no "wake time" setting**. The face treats your alarm as the start of the day (if no alarm is set, the last one, or 7:00 AM).
- The old Preview page and Alarm volume page are one page: the dot changes, the tap does.
- **Time is first,** so a daylight saving fix is two touches: hold the clock, drag the inner dot one step.

### 6.9 First run
1. Power on. Time and Date open in turn if the time is untrusted. Otherwise the clock shows "No alarm set" and, for 5 minutes, "Swipe down to add one".
2. Swipe down, tap a day, drag the dots to the time. The day turns on.
3. Swipe sideways to the next day, tap the centre (it takes the same time).

## 7. The face and the colour language

**Every place has its own colour, and colour belongs to the day.**

| Place | Colour | Why |
|-------|--------|-----|
| Clock | A living sky, full screen: peach at dawn, blue through the day, gold and rose toward evening | The time of day at a glance |
| Tomorrow, Week, Day, Alarm | Sunrise orange to coral | Everything about waking |
| Sound | One colour per sound: pink, white, brown, green, grey, blue, violet, and four for rain, ocean, wind, fan | You can tell the sound by its colour |
| Lamp | Gold (reading) or ember red (night light) | The colour of the light itself |
| Bedtime | Violet | Night |
| Ringing | A bright sunrise, full screen, dark text | Impossible to miss |

| Phase | When | What happens |
|-------|------|--------------|
| **Day** | from your alarm until 3 hours before bedtime | Full colour. The clock's sky drifts slowly. The date shows. |
| **Wind-down** | the last 3 hours | Colour, blue and brightness drain away on a smooth curve on every screen. The date fades. |
| **Night** | bedtime to the alarm | Every screen is dim amber on black. The clock shows only the time and the next alarm, and shifts one step a minute. A tap lifts it for 5 s. |

- **Depth and motion:** each screen has a soft glow behind its content; dots are small glossy spheres with a halo that breathes; sound bars dance while playing; the bell wiggles during a test; screens slide with a slight spring. Motion stops under "reduce motion".
- The next alarm on the clock always carries a small bell.
- Dimming is done through the panel's brightness setting, not by darkening colours (bench check UX-B1).

**Can this hardware draw it?** Mostly, and it must be measured. Gradients and glows will be pre-rendered images held in the 8 MB of PSRAM, not computed live, and animations redraw small areas. My estimate is 20–30 frames a second for full-screen slides on this panel; that is unverified (bench check UX-B3). If it is slower, slides become fades. A bright full-screen face all day wears an AMOLED more than a black one, so the day face also shifts a few pixels each minute.

## 8. Edit flows

| Task | Touches | Steps |
|------|---------|-------|
| Wake an hour earlier tomorrow | **2** | swipe up · drag the inner dot back one step |
| Wake 15 minutes earlier tomorrow | **2** | swipe up · drag the outer dot back three steps |
| Skip tomorrow | **2** | swipe up · hold |
| Undo a skip | **2** | swipe up · hold |
| Move every Wednesday an hour | **3** | swipe down · tap Wed · drag the inner dot |
| Turn a day off | **3** | swipe down · tap the day · hold |
| Fix the clock after a daylight saving change | **2** | hold the clock · drag the inner dot one step |
| Start the sleep sound | **2** | swipe left · tap |
| Change the sound | **2** | swipe left · swipe up |
| Night light on | **2** | swipe right · tap |
| Snooze | **1** | tap anywhere |
| Stop the alarm | **1** | hold 2 s |

A hold or a drag counts as one touch. The weekday edit has a budget of 10 s (R2).

## 9. Alarm lifecycle and status line

| Engine state | Clock face | Ring |
|--------------|-----------|------|
| Idle | face of the day | off |
| Dawn | "Sunrise · alarm 7:00 AM" | LIGHT.md §3 |
| Ringing | ringing screen, full brightness | daylight stage |
| Snoozed | "Snoozed · rings 7:09 AM" | off |
| Timed out | "Alarm 7:00 AM timed out" until touched | on |

On battery the ring is dark, so the screen ramps to warm white during the dawn and the alarm instead.
One status line, highest priority first: **Set time** · Alarm timed out · Alarms reset · **Plug me in · 31%** (battery only) · Battery 22% · Wed skipped. With no battery fitted, battery items never show.

## 10. Hardware that may not be fitted (D-025)

| Optional part | If missing |
|---------------|-----------|
| Battery | Detected through the battery chip. No battery text. About says "Battery: none". A power cut stops the clock until the time is set. |
| Ring | The Light screen says "No ring fitted"; the dawn uses the screen. A build flag. |
| Protection parts (fuse, capacitor, resistor) | Ring cap stays at 0.3 A until a build flag says they are fitted (then 0.9 A). |
| Level shifter, PCB | No firmware difference. |

## 11. Typography and layout rules

**Ubuntu** (Light for numerals, Regular for text, Medium for emphasis), Ubuntu Font Licence 1.0 in `docs/licenses/`. Firmware uses bitmap fonts made from the TTFs with `lv_font_conv`.

| Role | Size |
|------|------|
| Clock time | 104 px Light |
| Picker digits | 80 px Light |
| Settings value | 72 / 56 / 46 px Light |
| Titles, status, pill text | 28–34 px |
| Hints | 26 px |

**Rules.** Every text and icon box inside the safe circle (radius 209 px). No two boxes overlap (3 px padding). No text under 26 px. At most four text lines per screen (the week excepted). A string that does not fit is shortened, never shrunk.

**The automatic check.** `layoutCheck()` in the concept page draws **401 screen states**, now including the dots, (every screen, every day name, the widest times, every status line, every region name, every settings page), measures every box in the loaded Ubuntu font and reports anything outside the circle, overlapping, under 26 px, or over four lines. **Result on 2026-11-13: 2,401 boxes, 0 issues.**

## 12. Acceptance checks

| ID | Check | Requirement |
|----|-------|-------------|
| UX-1 | Move Wednesday's time by an hour in 3 touches and under 10 s | R2 |
| UX-2 | Tomorrow-only change in 2 touches; the override clears after it fires | R3 |
| UX-3 | A tap alone cannot skip tomorrow, turn a day off or stop an alarm | R34, R23 |
| UX-4 | A palm or an accidental press during an alarm snoozes at most; only a still 2 s hold stops it | R23 |
| UX-5 | Eyes closed in the dark: find the clock and snooze within 2 s | R5, R23 |
| UX-6 | Night face reads from 1 m in a dark room and lights nothing | R6 |
| UX-7 | Every sub-screen returns to the clock after 20 s (settings 30 s) with changes kept | R35 |
| UX-8 | The night face moves one step a minute | R6 |
| UX-9 | The second line always matches the engine's real next alarm | R4 |
| UX-10 | First alarm on a fresh clock in 3 touches | R1, R5 |
| UX-11 | Mon–Fri entry with carry-over | R1 |
| UX-12 | A swipe starting anywhere, including next to a dot, never changes a value (host test of 1,000+ swipes) | R5 |
| UX-13 | `layoutCheck()` reports zero issues | R37 |
| UX-14 | The face follows day, wind-down and night at the set hours, and uses night when the time is untrusted | R6 |
| UX-15 | Boots, keeps time and rings with no battery and with no ring | R36 |
| UX-16 | Daylight saving fix in 2 touches | R28 |
| UX-B1 | Bench: the dimmest panel level shows no banding or flicker | R6 |
| UX-B3 | Bench: full-screen slides and glows reach a smooth frame rate on the real panel (target 20+ a second), or fall back to fades | R45 |
| UX-B2 | Bench, with `/firmware/dialtest`: on the real panel, swipes (rim, next to a dot, middle) never change the time, and a deliberate grab always turns it | R5 |

## 13. What the concept page shows

The whole gesture model (swipe, tap, drag a dot, hold), every screen, the seven settings pages, the dynamic face on a time-of-day slider, the 30-minute dawn on the ring, and the two stress tests in §4.2. It does not simulate the night pixel shift, the 15-minute morning brighten, first-run tips or the status-line priorities beyond static examples.
