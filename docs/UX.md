# UX specification

Phase 2 (revised 2026-11-13) · Decisions: [D-011](DECISIONS.md), [D-015](DECISIONS.md), [D-016](DECISIONS.md), [D-024](DECISIONS.md), [D-026](DECISIONS.md), [D-027](DECISIONS.md), [D-028](DECISIONS.md)
Interactive concept (touch-only, real Ubuntu type): `/mockups/ux-concept.html`, published at https://claude.ai/artifact/UGtnZ6MqkrSfuM5BShyG5a (private link).

## 1. Principles

1. **Touch only.** Nothing on the device turns, clicks or slides. The only physical controls are the BOOT and PWR buttons, and no daily task needs them.
2. **Three gestures:** swipe (go), tap (start, or nudge a value), hold (anything that could stop an alarm).
3. **The clock is home.** Every other screen is one swipe away, and the opposite swipe comes back.
4. **Up means more, down means less.** On every screen with a value, tapping the top raises it and tapping the bottom lowers it.
5. **An alarm can only be stopped on purpose.** Turning a day off, skipping tomorrow, cancelling a snooze or dismissing needs a **hold**. Turning on, snoozing and bringing back are **taps**.
6. **Nothing to confirm, nothing to save.** Every change applies and saves at once. Any sub-screen returns to the clock after 20 s without a touch.
7. **Nothing depends on the rim.** The case and the halo ring may cover the outer millimetres, and the rim has no tactile cue in the dark.
8. **No text collides.** Every line sits in a safe zone, and an automatic check measures every screen state in the real font (§11).

## 2. Physical facts

| Fact | Value |
|------|-------|
| Screen | 466 × 466 px, round, about 44.5 mm across, so **10.5 px per mm** |
| Fingertip | about 10 mm, about **105 px** |
| Tap zones | at least **100 px** tall and wide |
| Safe text circle | radius **209 px** from the centre (24 px margin from the edge). Every text and icon box must sit inside it. |
| Smallest text | **26 px** |

## 3. Gestures

| Gesture | Definition | Feedback |
|---------|-----------|----------|
| **Tap** | Touch under 0.4 s, moves under 16 px | Soft ripple |
| **Swipe** | Moves at least 56 px (5 mm), one dominant axis, starting anywhere | Screens slide 0.24 s |
| **Hold** | Touch for the full time, moving under 16 px. The rim fills clockwise from 0.15 s. Letting go early cancels. | Rim fills |

Anything between 16 and 56 px of movement is ignored, so a wobbly tap does nothing. A second finger is ignored.

**Tap zones** (value screens; the centre band keeps its own meaning):
```
        |  hours  |  minutes |
 y 0    +---------+----------+
        |   ▲ +1h |  ▲ +5min |   top zone     0–120 px   later / more
 y 120  +---------+----------+
        |    centre band      |   120–346 px   tap: turn on · hold: turn off
 y 346  +---------+----------+
        |   ▼ -1h |  ▼ -5min |   bottom zone  346–466 px earlier / less
        +---------+----------+
```
- Left half = hours (±1 h). Right half = minutes (±5 min, carrying into the hour, so 7:00 minus 5 is 6:55).
- On screens with one value (volume, brightness, settings) the top and bottom zones are full width.
- **Hold in a top or bottom zone repeats.** After 0.45 s it steps every 0.23 s; after about 1.4 s minutes jump by 15. Holding in the **centre** is the guarded action (turn off, skip, dismiss).
- The hold times: turn a day off or skip tomorrow 1.0 s · open settings 1.5 s · dismiss the alarm 2.0 s · start a preview 1.5 s.

## 4. Options considered (and why)

### 4.1 Changing a value by touch (time, volume, brightness)

| # | Method | In the dark | Accidents | Typical change | Big change | Build risk | Clashes with swipes | Case / rim | Learnable |
|---|--------|-------------|-----------|----------------|-----------|-----------|--------------------|-----------|-----------|
| A | Drag around the rim like a watch crown | 3 | 3 | 4 | 4 | 2 | 2 | 1 | 3 |
| B | Drag up/down anywhere like a drum | 3 | 2 | 4 | 4 | 3 | 1 | 4 | 4 |
| C | Tap top / bottom (one value) | 5 | 4 | 2 | 3 | 5 | 5 | 5 | 5 |
| **D** | **Tap top / bottom with hours left, minutes right, hold repeats** | **5** | **4** | **4** | **3** | **5** | **5** | **5** | **5** |
| E | Drag a handle along an arc | 2 | 3 | 4 | 5 | 3 | 3 | 2 | 4 |

Scores 1–5, my judgement, not measured. Weighted total out of 95 (weights: dark 3, accidents 3, typical 2, big 1, build risk 3, swipe clash 3, case 2, learnable 2): A 50 · B 55 · C 84 · **D 88** · E 58.

- **A, the rim:** one continuous touch, but it needs a classifier to tell a rim turn from a swipe, the outer 8 mm may be hidden by the case, and there is no feel to find it in the dark.
- **B, drag anywhere:** the vertical axis already means "go to another screen", so a drag can't also mean "change the value" without a mode.
- **C, one value:** the most reliable, but a 15-minute change is three taps of five minutes and an hour is twelve.
- **D, chosen:** one tap moves an hour, three taps move 15 minutes, and holding repeats for big moves. It adds no gesture type, uses zones larger than a fingertip, and has nothing to misclassify.
- **E, a handle:** a small moving target is the hardest to hit half asleep.
- **Cost of D:** a 30-minute change is a hold of about 1.5 s, or six taps. Holding is one touch, so the count stays at three (§8).

The truest test is the real screen. Option A and D can both be tried on the board in the bring-up app if you want to feel them.

### 4.2 How screens connect

| # | Structure | Verdict |
|---|-----------|---------|
| **1** | **Cross: clock in the centre, four screens one swipe away. Vertical = waking (Week above, Tomorrow below). Horizontal = sleeping (Light left, Sound right).** | **Chosen.** Everything is one swipe away, the axes mean something, and the opposite swipe returns. |
| 2 | One row of screens you swipe through | Alarms are three swipes away, order must be learned, an overshoot lands on the wrong screen. |
| 3 | A home screen of icons you tap | Icons on a 44 mm screen are small targets and every action starts with an extra tap. |

### 4.3 Day and night

| # | Method | Verdict |
|---|--------|---------|
| **a** | **A schedule: night from 9 PM to 7 AM by default, both settable** | **Chosen.** No part needed, predictable, and the owner asked for it. |
| b | A light sensor | Not on the board. Could be added later as an optional part (D-025). |
| c | A manual day/night switch | Forgotten at night, and a bright screen at 3 a.m. is the worst failure. |

If the time is not trusted (after a power loss with no battery), the face uses the **night** level, never day.

## 5. Screen map

```
                      [ Day ]             tap a day on the Week screen
                         |
                     [ Week ]             swipe DOWN from the clock    (waking)
                         |
 [ Light ] ---------- [ CLOCK ] ---------- [ Sound ]
  swipe RIGHT             |              swipe LEFT                     (sleeping)
                     [ Tomorrow ]         swipe UP from the clock       (waking)

 Settings: hold the clock for 1.5 s. Pages sit side by side.
```

| Screen | Reached by | Back by | Auto-return |
|--------|-----------|---------|-------------|
| Clock | home | | |
| Tomorrow | swipe up | swipe down | 20 s |
| Week | swipe down | swipe up | 20 s |
| Day | tap a day on Week | swipe up (to Week) | 20 s |
| Sound | swipe left | swipe right | 20 s (the sound carries on) |
| Light | swipe right | swipe left | 20 s (the light carries on) |
| Settings | hold the clock 1.5 s | swipe down | 30 s |
| Ringing | automatic | tap = snooze · hold 2 s = dismiss | |

## 6. Screens

Wireframes show layout and wording. Exact positions are in §11 and in the concept page.

### 6.1 Clock, night (home)
```
        .-~~~~~~~~~~~~-.
      .'                '.
     /                    \
    |      11:14 PM        |       time, Ubuntu Light 104 px
    |    Wed 6:45 AM       |       the alarm that will really ring next
     \                    /
      '.                .'
        '-.~~~~~~~~~~~.-'
```
- Only the time and the next alarm. The face moves one step a minute (§9).
- The second line is the engine's real next alarm: an override, a skip, a snooze or "No alarm set" all show here (R4).

### 6.2 Clock, awake (a tap, for 5 s)
```
        .-~~~~~~~~~~~~-.
      .'     [cal]      '.
     /       Week         \
    | [sun] 11:14 PM [spk] |
    |   Wed 6:45 AM        |
     \  Hold for settings /
      '.   Tomorrow    .'
        '-.   [bell]  .-'
```
- Four small icons mark the swipes: calendar above (Week), bell below (Tomorrow), sun at the left (Light), speaker at the right (Sound). Only the vertical ones carry a word, so nothing crowds the time.
- The face brightens to the active level for 5 s, then dims. A tap changes nothing else.

### 6.3 Clock, day
Same layout in the day look: brighter, in the accent colour, with the date above the time ("Tuesday, Nov 12"). No pixel shifting is needed.

### 6.4 Tomorrow (swipe up)
```
        .-~~~~~~~~~~~~-.
      .'  ^ Hour  ^ Min '.            tap top: later
     /   Tomorrow · Wed   \
    |     6  :  45         |          big picker, Ubuntu Light 100 px
    |         AM           |
     \    Hold to skip    /           status line
      '.  v        v    .'            tap bottom: earlier
        '-.~~~~~~~~~~~.-'
```

| State | Status line | Gestures |
|-------|-------------|----------|
| Usual | Hold to skip | tap zones change the time for tomorrow only · **hold centre 1 s = skip** |
| Changed | Once only · was 6:45 AM (teal) | tap back to the usual time and the override clears itself |
| Skipped | Skipped · tap to restore (warm red), time struck through | **tap centre = bring it back** |
| No alarm | Tap above to set one | tap zones set one for tomorrow only |

An override is used once and then clears itself (R3).

### 6.5 Week (swipe down)
```
        .-~~~~~~~~~~~~-.
      .'      Mon       '.            seven wedges. Each shows the day
     /  Sun   6:45AM  Tue  \          and its time, or "off".
    |  off             6:45AM|         Tap a wedge to open that day.
    | Sat     Next      Wed  |         The centre shows the next alarm
    | off   Wed 6:45 AM 6:45AM|        (information only).
     \   Fri        Thu    /
      '.  6:45AM   6:45AM .'
        '-.~~~~~~~~~~~~.-'
```
Each wedge is about 14 mm wide at the finger. Swipe up returns to the clock.

### 6.6 Day (tap a day on Week)
Same layout as Tomorrow, with the title **Every Wednesday** and seven dots at the bottom for which day.

| State | Status line | Gestures |
|-------|-------------|----------|
| On | On · hold to turn off | tap zones = time · **hold centre 1 s = off** |
| Off | Off · tap to turn on | **tap centre = on** · any tap zone also turns it on |

**Carry-over:** swiping sideways to a day that is off starts it at the time you just set.

### 6.7 Sound (swipe left)
```
        .-~~~~~~~~~~~~-.
      .'       ^        '.            top: louder
     /     White noise    \
    |          ||          |           tap here: play / pause
    |      Volume 40%      |
     \  ( Sleep in 30 min ) /          tap the pill: 15 · 30 · 60 · 90 · all night
      '.       v         .'            bottom: quieter
        '-.~~~~~~~~~~~.-'
```
- Remembers the last volume and timer: **swipe left, tap** starts sleep sound (2 gestures).
- Swipe up/down on this screen is reserved to change the sound (Phase 5). Swipe right returns.
- The sound fades out by itself when the timer ends or the sunrise begins (R10).

### 6.8 Light (swipe right)
Same layout. Title **Night light** or **Reading light**, a big percentage or "Off", and a pill that switches between them. Tap the upper centre = on/off, top/bottom = brighter/dimmer. Each mode remembers its own brightness. Night light is red only and capped very low.

### 6.9 Sunrise in progress (clock face)
The clock at night dimness with "Sunrise · alarm 7:00 AM". The ring does the waking. **Hold 1.5 s** anywhere stops the sunrise and the alarm for today.

### 6.10 Ringing
```
        .-~~~~~~~~~~~~-.             full brightness, warm white
      .'                '.
     /       7:00 AM      \
    |  Tap to snooze · 2 left |
    |   Hold to turn off      |      rim fills as you hold
     \                    /
      '.                .'
        '-.~~~~~~~~~~~.-'
```
- **Tap anywhere** = snooze 9 min (up to 3, count shown). **Hold 2 s** = dismiss.
- After the third snooze: "No snoozes left", and a tap only flashes the text.
- Fallback if the screen is dead: hold the **BOOT** button 3 s (D-007).
- After 30 min without a response the sound stops and the face shows "Alarm 7:00 AM timed out" until touched (D-015).
- **After a snooze:** "Snoozed · rings 7:09 AM" on the clock. A hold cancels it.

### 6.11 Settings (hold the clock 1.5 s)
Pages sit side by side: **swipe sideways** between them, **tap top / bottom** to change the value, **swipe down** to leave. A page counter ("3 / 10") sits at the bottom.

| # | Page | Control | Default |
|---|------|---------|---------|
| 1 | Time zone | tap top / bottom to step, tap centre for next | Eastern |
| 2 | Set time | hour and minute zones | opens by itself if the time is untrusted |
| 3 | Day look | top / bottom = brightness 10–100 %, tap the pill = accent colour (Amber, Sky, Mint, Rose) | 50 %, Amber |
| 4 | Night hours | left = when night starts, right = when it ends, ±1 hour | 9 PM to 7 AM |
| 5 | Night dimness | top / bottom, 1 (dimmest) to 5 | 2 |
| 6 | Sunrise length | top / bottom, 10–60 min in 5s | 30 min |
| 7 | Alarm loudness | top / bottom, 30–100 % in 10s | 70 % (never below 30 %) |
| 8 | Preview alarm | hold 1.5 s | a 60 s run: sunrise, sound steps, snooze, dismiss (R26) |
| 9 | Wi-Fi | status only | read-only until Phase 6 |
| 10 | About | status only | version, battery, USB voltage, last reset reason |

Time shows in **12-hour AM/PM** (US). The picker steps through all 24 hours, so AM/PM flips on its own and can never be entered wrong.

### 6.12 First run (D-015: no default alarm)
1. Power on. If the time is untrusted, **Set time** opens. Otherwise the clock shows "No alarm set" and, for 5 minutes, "Swipe down to add one".
2. **Swipe down** → **tap a day** → **tap the hour / minute zones** to the time. Turning the time makes the day active.
3. Swipe sideways to the next day, tap (it takes the same time), repeat.

First alarm: 3 touches plus the taps for the time. Mon–Fri: about 11 touches plus the taps for the time.

## 7. Day and night look

| | **Night** | **Day** |
|---|----------|---------|
| When | the night hours (default 9 PM–7 AM) | the rest |
| Colour | amber only, blue under 15 % | your accent: Amber, Sky, Mint or Rose |
| Panel | dimness 1–5, about 1–6 % brightness | brightness 10–100 %, default 50 % |
| Face | time + next alarm only | date, time, next alarm |
| Burn-in | the whole face shifts one step a minute on a 12-point circle of 6 px | not needed |
| A tap | lifts to about 25 % for 5 s and shows the swipe icons | shows the swipe icons |
| Untrusted time | night level | |

- **Dimming is done through the panel's brightness setting, not by darkening the colours**, to avoid banding at the dark end (bench check UX-B1).
- The ring is not part of the look. It follows the Light screen and the sunrise.
- **What "vibe" should day have beyond colour, brightness and the date?** Open question for the owner.

## 8. Edit flows

| Task | Touches | Steps |
|------|---------|-------|
| Wake an hour earlier tomorrow | **2** | swipe up · tap lower left |
| Wake 15 minutes earlier tomorrow | **4** | swipe up · tap lower right three times (or hold about 0.8 s: 2) |
| Skip tomorrow | **2** | swipe up · hold |
| Undo a skip | **2** | swipe up · tap centre |
| Move every Wednesday an hour earlier | **3** | swipe down · tap Wed · tap lower left |
| Move it half an hour | **3** | swipe down · tap Wed · hold lower right (about 1.5 s) |
| Turn a day off | **3** | swipe down · tap the day · hold centre |
| Turn a day on | **3** | swipe down · tap the day · tap centre |
| Start white noise | **2** | swipe left · tap |
| Night light on | **2** | swipe right · tap |
| Snooze | **1** | tap anywhere |
| Dismiss | **1** | hold 2 s |

A hold counts as one touch. The weekday edit has a budget of 10 s (R2).

## 9. Alarm lifecycle and status line

| Engine state | Clock face | Ring | Sound |
|--------------|-----------|------|-------|
| Idle | time + next alarm | off | as chosen |
| White noise | time + next alarm | off | noise at set volume |
| Sunrise | "Sunrise · alarm 7:00 AM" | ramps up | noise fades out over 2 min |
| Ringing | ringing screen, full brightness | full | soft tone, louder each minute |
| Snoozed | "Snoozed · rings 7:09 AM" | off | silent |
| Timed out | "Alarm 7:00 AM timed out" until touched | on | stops |
| Dismissed | "Good morning" 2 s, then the clock | off | stops |

On battery the ring is dark, so the **screen ramps to full warm white** during the sunrise and the alarm.

One status line under the next alarm, highest priority first:

| Priority | Condition | Text |
|----------|-----------|------|
| 1 | Time untrusted | **Set time** (opens that page) |
| 2 | Alarm timed out | Alarm 7:00 AM timed out |
| 3 | Stored alarms damaged and reset | Alarms reset · swipe down |
| 4 | Next alarm may be missed on battery (R24) | **Plug me in · 31%** |
| 5 | Battery under 25 % | Battery 22% |
| 6 | Next alarm skipped | Wed skipped |
| 7 | Face awake, nothing else | Hold for settings |

On battery, a small dot beside the time replaces text. With **no battery fitted**, battery items never show.

## 10. Hardware that may not be fitted yet (D-025)

The firmware must run, keep time, and ring while parts are still arriving:

| Optional part | If missing |
|---------------|-----------|
| Backup battery | The battery chip reports it. Battery text and "Plug me in" never show. About says "Battery: none". A power cut stops the clock. |
| Ring | Light screen shows "No ring fitted" and sunrise uses the screen. Set by a build flag. |
| Protection parts (fuse, capacitor, resistor) | Ring current cap stays at 0.3 A until the build flag says they are fitted (then 0.9 A). |
| Level shifter, custom PCB | No firmware difference. |
| Light sensor (future) | Day and night follow the schedule. |

## 11. Typography and layout rules

**Font: Ubuntu** (Light 300 for large numerals, Regular 400 for text, Medium 500 for emphasis). Ubuntu Font Licence 1.0 (text kept in `docs/licenses/`). Firmware uses bitmap fonts made from the TTFs with `lv_font_conv` (4 bits per pixel, digits and the characters actually used). Source files: `github.com/google/fonts/tree/main/ufl/ubuntu`.

| Role | Size | Weight |
|------|------|--------|
| Clock time | 104 px | Light |
| Picker digits | 100 px | Light |
| Settings value | 64 / 56 / 40 px | Light |
| Titles, status, pill text | 28–34 px | Regular |
| Hints, labels | 26 px | Regular |

**Rows on picker screens** (centre y in px): column labels 104 · title 145 · digits 222 · AM/PM 285 · status line 322 · chevrons 70 and 396 · day dots 428 · page counter 426.
**Rows on the clock:** date 140 · time 213 · next alarm 290 · status 336 · icons at the four edges (52, 414, x 44, x 422).

**Rules**
1. Every text and icon box sits inside the safe circle (radius 209 px).
2. No two boxes overlap, with 3 px padding. Text boxes use the real font's measured width.
3. No text under 26 px.
4. Strings are written to fit: if a new string fails the check it is shortened, never shrunk.
5. Wording must fit the narrowest row it can appear on. At 336 px down the usable width is about 370 px.

**The automatic check.** `layoutCheck()` in the concept page draws all **188** screen states (every screen, every day name, the widest times, every status line, both looks, every settings page), measures each box in the loaded Ubuntu font and reports anything outside the circle, overlapping, or under 26 px. To run it, open `mockups/ux-concept.html`, wait for the fonts, and run `layoutCheck()` in the browser console. **Result on 2026-11-13: 1,667 boxes, 0 issues.**

## 12. Acceptance checks

Each becomes a numbered test in TEST_PLAN.md in Phase 3.

| ID | Check | Requirement |
|----|-------|-------------|
| UX-1 | Change Wednesday's time by an hour in 3 touches and under 10 s | R2 |
| UX-2 | Tomorrow-only change in 2 touches; the override clears after it fires | R3 |
| UX-3 | A tap alone cannot skip tomorrow, turn a day off or dismiss an alarm | R34, R23 |
| UX-4 | A palm or an accidental press during an alarm snoozes at most; only a still 2 s hold dismisses | R23 |
| UX-5 | Eyes closed in the dark: find the clock and snooze within 2 s | R5, R23 |
| UX-6 | Night face reads from 1 m in a dark room and doesn't light it | R6 |
| UX-7 | Every sub-screen returns to the clock after 20 s (settings 30 s) with changes kept | R35 |
| UX-8 | The night face moves one step a minute | R6 |
| UX-9 | The second line always matches the engine's real next alarm | R4 |
| UX-10 | First alarm on a fresh clock: swipe, tap, tap zones | R1, R5 |
| UX-11 | Mon–Fri entry with the carry-over rule | R1 |
| UX-12 | Every tap zone is at least 100 px, and a swipe starting anywhere works | R5 |
| UX-13 | `layoutCheck()` reports zero issues for every state | R37 |
| UX-14 | The face switches between day and night at the set hours, and uses night when the time is untrusted | R6 |
| UX-15 | The firmware boots, keeps time and rings with no battery and with no ring fitted | R36 |
| UX-B1 | Bench: the dimmest panel level shows no banding or flicker | R6 |
| UX-B2 | Bench: swipe, tap and hold are recognised reliably on the real panel, including near the edge (try the picker in the bring-up app first) | R5 |

## 13. What the concept page does and doesn't show

It implements the gesture model: clock (night and day looks), Tomorrow, Week, Day, Sound, Light, Ringing, ten settings pages, hold to open settings, hold-repeat, carry-over, auto-return and the morning demo. It does not simulate the one-step-a-minute pixel shift, the 5 s tap-wake dimming (it shows the tapped state), first run, or the warning priorities beyond static examples.
