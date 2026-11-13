# UX specification

Phase 2 (revised 2026-11-13) · Decisions: [D-011](DECISIONS.md), [D-015](DECISIONS.md), [D-016](DECISIONS.md), [D-024](DECISIONS.md), [D-028](DECISIONS.md), [D-032](DECISIONS.md), [D-034](DECISIONS.md)
Interactive concept (touch only, Ubuntu type, 257 checked screen states): `/mockups/ux-concept.html`, published at https://claude.ai/artifact/UGtnZ6MqkrSfuM5BShyG5a (private link).
Light and colour behaviour: [LIGHT.md](LIGHT.md). Offline and long-life rules: [LONGEVITY.md](LONGEVITY.md).

## 1. Principles

1. **Touch only.** Nothing on the device turns or clicks. The BOOT and PWR buttons are never needed for daily use.
2. **Few words.** At most **four lines of text on any screen** (the week is the one exception). Icons and the layout carry the meaning. One-time tips can teach a gesture and then never appear again.
3. **Four gestures:** swipe (go), tap (start, or nudge), turn (run a finger around the rim to set a value), hold (anything that could stop an alarm).
4. **The clock is home.** Every other screen is one swipe away, and the opposite swipe comes back.
5. **An alarm can only be stopped on purpose.** Turning a day off, skipping tomorrow, cancelling a snooze or dismissing needs a **hold**. Turning on, snoozing and bringing back are **taps**.
6. **Nothing to confirm, nothing to save.** Every change applies and saves at once. Any sub-screen returns to the clock after 20 s without a touch.
7. **Two ways to set a value, so one always works.** The rim dial is the fast way. Tapping above and below is the sure way, in case the case ever covers part of the rim.
8. **No text collides.** Every line sits in a safe zone, and an automatic check measures every screen state in the real font (§11).

## 2. Physical facts

| Fact | Value |
|------|-------|
| Screen | 466 × 466 px, round, about 44.5 mm across, so **10.5 px per mm** |
| Fingertip | about 10 mm, about **105 px** |
| Tap zones | at least **100 px** tall and wide |
| Dial band | the outer ring from **radius 130 px to the edge**, about 10 mm wide, so even if the case hides the last 3 mm the dial still works |
| Safe text circle | radius **209 px**. Every text and icon box sits inside it. |
| Smallest text | **26 px** |

## 3. Gestures

| Gesture | Definition | Feedback |
|---------|-----------|----------|
| **Tap** | Under 0.4 s, moves under 16 px | Soft ripple |
| **Swipe** | Moves at least 56 px, one dominant axis, starting anywhere. A drag that starts on the rim and heads inward is a swipe. | Screens slide 0.24 s |
| **Turn** | Touch starts in the dial band and its first 16 px of movement follow the rim. Clockwise is later or more. | A marker on the rim follows the finger |
| **Hold** | Full time, moves under 16 px. The rim fills clockwise after 0.15 s. Letting go early cancels. | Rim fills |

- **Turn speed sets the step** (times): under 30°/s steps **1 minute** · 30–120°/s **5 min** · 120–300°/s **15 min** · over 300°/s **30 min**. One step is 6° of rim. For volume and brightness, 3° is 1 %, linear.
- **Tap zones** (the backup): top = later or more, bottom = earlier or less. On time screens the left half moves the **hour** and the right half the **minutes** (±5, carrying into the hour). Holding a zone repeats, and after about 1.4 s minutes jump by 15.
- **Hold times:** turn a day off or skip tomorrow 1.0 s · open settings 1.5 s · start a preview 1.5 s · dismiss the alarm 2.0 s.
- Movement of 16–56 px that is not a turn is ignored. A second finger is ignored. No sound on touch.
- **Which screens have a dial:** Tomorrow, Day, Time, Sound, Light, Brightness, Alarm volume. Pages with several values (Date, Sleep hours) and the Region list use the tap zones.
- **The marker.** On time screens it sits on a 12-hour face (top is 12). On volume and brightness it sweeps a 270° arc. A faint track shows that the rim can be turned.

```
 y 0   +----------+-----------+
       |  ▲ hour  |  ▲ min    |   top zone 0–120 px: later / more
 y 120 +----------+-----------+
       |      centre band     |   tap: turn on · hold: turn off
 y 346 +----------+-----------+
       |  ▼ hour  |  ▼ min    |   bottom zone 346–466 px: earlier / less
       +----------+-----------+
   plus: the rim (radius 130 px out) turns
```

## 4. Options considered

### 4.1 Setting a value by touch
| # | Method | Verdict |
|---|--------|---------|
| A | **Drag around the rim** | **Kept (owner prefers it).** One continuous touch for any change, with a marker that shows where you are. Risk: the case may cover the outer millimetres, and there is no feel in the dark. Mitigation: a 10 mm band, and the taps below. |
| B | Drag up/down anywhere | Rejected: the vertical axis already means "go to another screen". |
| C | Tap above / below, one value | Kept inside D. |
| **D** | **Tap above / below, hours left and minutes right, hold repeats** | **Kept as the backup and for precision.** No gesture type to misclassify, big zones. |
| E | Drag a small handle | Rejected: hardest target half asleep. |

The earlier weighted scores (A 50, B 55, C 84, D 88, E 58 out of 95) were my judgement before the owner chose the dial; they favoured D on reliability and A on speed. Having both costs a little more code and gives a working fallback. **Test UX-B2 compares them on the real panel.**

### 4.2 How screens connect
**Cross** (chosen): clock in the centre, four screens one swipe away. Vertical is waking (Week above, Tomorrow below), horizontal is sleeping (Light left, Sound right). A row of screens or an icon home screen were rejected (more swipes, small targets).

### 4.3 Day and night
Not a switch. The face **changes continuously** from wake to bedtime and winds down (§7). If the time is untrusted the face uses the night level, never day.

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
| Day | tap a day on Week | swipe up | 20 s |
| Sound | swipe left | swipe right | 20 s (the sound carries on) |
| Light | swipe right | swipe left | 20 s (the light carries on) |
| Settings | hold the clock 1.5 s | swipe down | 30 s |
| Ringing | automatic | tap = snooze · hold 2 s = stop | |

## 6. Screens (text budget in brackets)

Exact layouts are in the concept page. The wireframes show content only.

### 6.1 Clock (home) [2–3 lines]
```
        .~~~~~~~~~~~~~~~~.
      '    Tue, Nov 12     '        date: day and early wind-down only
     |      9:00 AM        |       time, Ubuntu Light 104 px, colour follows the day
     |    Wed 6:45 AM      |       the alarm that will really ring next
      .                  .
        '~~~~~~~~~~~~~~'
```
- The second line is the engine's real next alarm: an override, a skip or "No alarm set" all show here (R4).
- **Tapped (5 s):** four small icons mark the swipes (calendar above, bell below, sun at the left, speaker at the right) and a small gear marks settings. No words.
- The face's colour and brightness follow §7. There is no sun marker or arc: the face changes by colour and brightness alone.

### 6.2 Tomorrow (swipe up) [title, time, one state word]
Title **Tomorrow**, a big picker (hour, minute, AM/PM), a rim marker, chevrons above and below.

| State | Under the time | Gestures |
|-------|----------------|----------|
| Usual | nothing | turn the rim or tap to change for tomorrow only · **hold centre 1 s = skip** |
| Changed | "Once only" (teal) | turning back to the usual time clears it |
| Skipped | "Skipped" (red), time struck through | **tap centre = bring it back** |
| None | "Tap above to set one" | |

### 6.3 Week (swipe down)
Seven wedges (day, time or "off"), the next alarm in the middle. Tap a wedge to open that day. Each wedge is about 14 mm wide at the finger.

### 6.4 Day (tap a day on Week) [title, time, "Off" only when off]
Title is the day's name, dots show which day. Same picker as Tomorrow. **Tap centre = on, hold centre = off.** Swiping sideways to a day that is off starts it at the time just set (carry-over).

### 6.5 Sound (swipe left) [title, one pill]
```
          ▲
         Sound
    ▂ ▃ ▄ ▅ ▆ ▇ ▇ ▇ ▇ ▇          ten bars show the volume
           ||                    tap: play / pause
      ( moon  30 min )           tap: sleep timer 15 · 30 · 60 · 90 · all night
          ▼
```
Turning the rim sets the volume. Swipe up/down on this screen is reserved for choosing the sound (Phase 5). The sound fades out by itself when the timer ends or the dawn begins (R10).

### 6.6 Light (swipe right) [title, one number]
Title, a big percentage or "Off", and a two-icon switch (moon = night light, sun = reading light). Tap the number = on/off. Tap the switch = change mode. Turn the rim for brightness. Caps and colours follow the time of day (LIGHT.md §4).

### 6.7 Dawn and ringing [3 lines]
- **Dawn:** the clock at its current level with "Sunrise · alarm 7:00 AM". The ring follows LIGHT.md §3. **Hold 1.5 s** anywhere stops it for today.
- **Ringing:** the time, "Tap to snooze", "Hold to stop", and three dots for the snoozes left. **Tap anywhere = snooze 9 min** (3 times). **Hold 2 s = stop.** The rim fills as you hold.
- After 30 minutes the sound stops and the face says "Alarm 7:00 AM timed out" until touched (D-015). BOOT held 3 s stops the alarm even if the screen is dead.

### 6.8 Settings (hold the clock 1.5 s): eight pages
Swipe sideways between pages, turn the rim or tap to change, swipe down to leave. Dots at the bottom show the page.

| # | Page | You set | Default |
|---|------|---------|---------|
| 1 | **Time** | the hour and minute | after a power loss it opens by itself |
| 2 | **Date** | month, day, year | |
| 3 | **Region** | US Eastern, Central, Mountain, Pacific, Alaska, Hawaii; UK; Central Europe; **Manual**; Custom (from the SD card) | US Eastern |
| 4 | **Sleep hours** | bedtime (left) and wake (right) | 10:30 PM and 7:00 AM |
| 5 | **Brightness** | the daytime brightness | 60 % |
| 6 | **Alarm volume** | how loud the alarm gets | 70 % (never below 30 %) |
| 7 | **Preview alarm** | hold 1.5 s for a 60 s run-through of dawn, sound, snooze and stop | |
| 8 | **About** | nothing: version, battery, last reset reason | |

- **Time is first** so that fixing the clock after a daylight saving change is two touches (LONGEVITY.md §3). The first three pages are touched once, or after a battery or power loss.
- Removed from earlier drafts: Wi-Fi, accent colour, night dimness (the night level is fixed at the panel's lowest comfortable level), sunrise length (fixed at 30 minutes, LIGHT.md), and the day look page (replaced by Sleep hours and Brightness).
- **Date, Sleep hours and Region** use the tap zones: left, middle and right for the columns.

### 6.9 First run
1. Power on. The Time and Date pages open in turn if the time is untrusted. Otherwise the clock shows "No alarm set" and, for 5 minutes, "Swipe down to add one".
2. Swipe down, tap a day, turn the rim to the time. The day turns on.
3. Swipe sideways to the next day and tap (it takes the same time).

## 7. The face that follows your day

The face is a function of the time between **wake** and **bedtime** (two settings), not a day/night switch.

| Phase | When | Face |
|-------|------|------|
| **Day** | wake to 3 hours before bed | Brightness follows the setting. Colour moves with the hour: gold at 6, warm white by 9, cool white at noon, warm white mid-afternoon, amber by 6 PM. The date shows. |
| **Wind-down** | the last 3 hours | The colour blends to amber and the blue drains out. Brightness falls on a smooth exponential curve. The date fades. (The evidence for 3 hours is in LIGHT.md §1.) |
| **Night** | bedtime to wake | Amber only, lowest level, time and next alarm only. The whole face shifts one step a minute on a 12-point circle of 6 px. A tap lifts it for 5 s. |
| **Morning** | wake time | The face brightens over about 15 minutes. |

- **Dimming is done through the panel's brightness setting, not by darkening the colours,** to avoid banding at the dark end (bench check UX-B1).
- The ring is not part of the face. It follows the Light screen, the dawn and LIGHT.md.
- The slider under the concept device scrubs the clock through the day.

## 8. Edit flows

| Task | Touches | Steps |
|------|---------|-------|
| Wake an hour earlier tomorrow | **2** | swipe up · flick the rim back (or tap the lower left) |
| Wake 15 minutes earlier tomorrow | **2** | swipe up · turn the rim slowly (or tap the lower right three times) |
| Skip tomorrow | **2** | swipe up · hold |
| Undo a skip | **2** | swipe up · tap centre |
| Move every Wednesday | **3** | swipe down · tap Wed · turn the rim |
| Turn a day off | **3** | swipe down · tap the day · hold centre |
| Fix the clock after a daylight saving change | **2** | hold the clock · tap the hour zone |
| Start white noise | **2** | swipe left · tap |
| Night light on | **2** | swipe right · tap |
| Snooze | **1** | tap anywhere |
| Stop the alarm | **1** | hold 2 s |

A hold or a turn counts as one touch. The weekday edit has a budget of 10 s (R2).

## 9. Alarm lifecycle and status line

| Engine state | Clock face | Ring |
|--------------|-----------|------|
| Idle | face of the day | off |
| Dawn | "Sunrise · alarm 7:00 AM" | LIGHT.md §3 |
| Ringing | ringing screen, full brightness | daylight stage |
| Snoozed | "Snoozed · rings 7:09 AM" | off |
| Timed out | "Alarm 7:00 AM timed out" until touched | on |

On battery the ring is dark, so the screen ramps to warm white during the dawn and the alarm instead.

One status line under the next alarm, highest priority first: **Set time** · Alarm timed out · Alarms reset · **Plug me in · 31%** (battery only) · Battery 22% · Wed skipped. With no battery fitted, battery items never show.

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
| Picker digits | 100 px Light |
| Settings value | 72 / 56 / 46 px Light |
| Titles, status, pill text | 28–34 px |
| Hints | 26 px |

**Rules.** Every text and icon box inside the safe circle (radius 209 px). No two boxes overlap (3 px padding). No text under 26 px. **No more than four text lines per screen** (the week excepted). A string that does not fit is shortened, never shrunk.

**The automatic check.** `layoutCheck()` in the concept page draws **257 screen states** (every screen, every day name, the widest times across the whole day, every status line, every region name, every settings page), measures every box in the loaded Ubuntu font and reports anything outside the circle, overlapping, under 26 px, or over four lines. Open `mockups/ux-concept.html`, wait for the fonts, run `layoutCheck()` in the browser console. **Result on 2026-11-13: 1,796 boxes, 0 issues.**

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
| UX-12 | Tap zones are at least 100 px; swipes start anywhere; an inward drag from the rim is a swipe | R5 |
| UX-13 | `layoutCheck()` reports zero issues | R37 |
| UX-14 | The face follows day, wind-down and night at the set hours, and uses night when the time is untrusted | R6 |
| UX-15 | Boots, keeps time and rings with no battery and with no ring | R36 |
| UX-16 | Daylight saving fix in 2 touches | R28 |
| UX-B1 | Bench: the dimmest panel level shows no banding or flicker | R6 |
| UX-B2 | Bench: the rim dial and the tap zones are both recognised reliably on the real panel, including at the edge; compare the two | R5 |

## 13. What the concept page shows

The gesture model including the rim dial (a simulated drag test passed: slow turns step by a minute, fast flicks by half an hour, an inward drag from the rim is a swipe), the dynamic face on a time-of-day slider, all screens and the eight settings pages, and the 30-minute dawn on the ring. It does not simulate the night pixel shift, the 15-minute morning brighten, first-run tips or the status-line priorities beyond static examples.
