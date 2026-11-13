# UX specification

Phase 2 · 2026-11-13 · Decisions: [D-011](DECISIONS.md), [D-015](DECISIONS.md), [D-016](DECISIONS.md), [D-024](DECISIONS.md)
Interactive concept (open in a browser): `/mockups/ux-concept.html`, published at https://claude.ai/artifact/UGtnZ6MqkrSfuM5BShyG5a (private link).

## 1. Principles

1. **The clock is home.** Every other screen is one swipe away, and the opposite swipe always comes back.
2. **Four gestures only:** swipe (go), turn the edge (change a value), tap (turn on, snooze), hold (anything that could stop an alarm).
3. **An alarm can only be stopped on purpose.** Anything that makes an alarm not ring (turn a day off, skip tomorrow, dismiss) needs a **hold**. Anything that only makes it ring (turn on, snooze) is a **tap**. A sleepy hand can't silence tomorrow by accident.
4. **Nothing to confirm, nothing to save.** Every change applies and saves the instant it is made. Leaving a screen can't discard anything.
5. **Always back to the clock.** After 20 s without a touch any screen returns to the clock, so the clock is never left on a menu overnight.
6. **Dark first.** Only amber light, dimmed through the panel, and a face that moves slightly every minute to protect the screen.
7. **One line of news.** The face shows at most one status line at a time (§8).

## 2. Physical facts (design numbers)

| Fact | Value |
|------|-------|
| Screen | 466 × 466 px, round, about 44.5 mm across, so **10.5 px per mm** |
| Fingertip | about 10 mm, so **about 105 px** |
| Minimum tap zone | **100 px** (9.5 mm) in both directions |
| Rim band (dial zone) | outer **80 px** (r from 153 to 233 px, about 7.6 mm wide) |
| Inner area | r under 153 px (about 29 mm across) |
| Smallest text | **26 px**. Secondary lines 28 px, the time 150 px. |

The busiest screen, the week, has seven wedges, each about **14 mm** wide at the finger's position and 13 mm deep. Every other screen has one or two touch zones.

## 3. Gesture vocabulary

| Gesture | Definition | Used for | Feedback |
|---------|-----------|----------|----------|
| **Tap** | Touch under 0.4 s, moves under 16 px | Play/pause, turn a day on, bring tomorrow back, snooze, wake the face | Soft ripple at the finger |
| **Swipe** | Moves at least 56 px (5 mm) along one dominant axis | Go to a screen; the opposite swipe goes back | The screens slide 0.24 s |
| **Turn the edge** | Touch starts in the rim band and the first 16 px of movement follow the rim (tangential) | Time, volume, brightness | The value changes live; the dial marker follows the finger |
| **Hold** | Touch for the full time, moving under 16 px | Turn off a day, skip tomorrow, dismiss, open settings | After 0.15 s the rim fills clockwise. Letting go early cancels. |

- **Why tangential:** a drag from the rim that heads toward the centre is still a swipe, so a swipe can start anywhere. Only a drag that follows the rim turns the dial.
- **Turn speed sets the step** (time dials only): under 30°/s steps 1 min · 30–120°/s steps 5 min · 120–300°/s steps 15 min · over 300°/s steps 60 min. One step is 6° of rim. Volume and brightness are linear: the 270° arc is 0–100 %.
- **Hold times:** turn a day off or skip tomorrow 1.0 s · open settings 1.5 s · dismiss the alarm 2.0 s.
- **Ignored touches:** any second finger, and anything beyond r = 228 px (the bezel).
- **No sound on touch.** The clock never clicks at night.
- **No double-tap, no multi-finger gestures, no motion gestures** (D-011).

## 4. Screen map

```
                     [ Day ]            tap a day on the Week screen
                        |
                    [ Week ]            swipe DOWN from the clock
                        |
 [ Light ] ---------- [ CLOCK ] ---------- [ Sound ]
  swipe RIGHT            |             swipe LEFT
                    [ Tomorrow ]        swipe UP from the clock

 Settings: hold the clock for 1.5 s.  Settings pages sit side by side.
```

Swipe direction is the way your finger moves, and the new screen follows it: swipe up and Tomorrow rises from below, swipe down and Week drops in from above.

| Screen | Reached by | Back by | Auto-return |
|--------|-----------|---------|-------------|
| Clock | home | | |
| Tomorrow | swipe up | swipe down | 20 s |
| Week | swipe down | swipe up | 20 s |
| Day | tap a day on Week | swipe up (to Week) | 20 s |
| Sound | swipe left | swipe right | none while sound plays, otherwise 20 s |
| Light | swipe right | swipe left | none while the light is on, otherwise 20 s |
| Settings | hold the clock 1.5 s | swipe down | 30 s |
| Ringing | automatic | tap = snooze · hold 2 s = dismiss | |

"Auto-return" on Sound and Light only moves the *view* back to the clock. The sound and light carry on.

## 5. Screens

The wireframes show layout and wording only. Colours and sizes are in §9. The rim carries the dial where noted.

### 5.1 Clock (home, night)
```
        .-~~~~~~~~~~~~-.
      .'                '.
     /                    \
    |        23:14         |        time, 150 px, amber
    |      Wed  06:45      |        the alarm that will actually ring next
     \                    /
      '.                .'
        '-.~~~~~~~~~~~.-'
```
- Shows only the time and the next alarm. Nothing else is drawn, so little can burn in.
- The second line is what the alarm engine will really do: an override, a skip, a snooze or "No alarm set" all show here (R4).
- The whole face drifts in a 6 px circle, one step a minute (§9).

### 5.2 Clock, awake (after a tap, for 5 s)
```
        .-~~~~~~~~~~~~-.
      .'   Tue 12 Nov   '.
     /       ^ Week       \
    | < Light 23:14 Sound > |
    |     Wed 06:45  82%   |
     \    v Tomorrow      /
      '.                .'
        '-.~~~~~~~~~~~.-'
```
- Brighter for 5 s, with a hint arrow for each swipe. It shows the date and battery %, then settles back to night level.
- A tap here changes nothing else, so a stray touch is harmless.

### 5.3 Tomorrow (swipe up)
```
        .-~~~~~~~~~~~~-.          rim: 24-hour dial,
      .'  Tomorrow · Wed '.       marker at the alarm time
     /                      \
    |        06:45           |    turn the edge to change it, for tomorrow only
    |  Just this once · usually 07:00 |
     \  Sunrise starts 06:15 /
      '.                  .'
        '-.~~~~~~~~~~~~.-'
```

| State | Middle shows | Sub-line | Gestures |
|-------|--------------|----------|----------|
| Usual | the Wednesday time | Sunrise starts 06:15 | turn = change for tomorrow · **hold 1 s = skip** |
| Changed | the new time (green-teal tint) | Just this once · usually 07:00 | turn back to the usual time and the override clears itself |
| Skipped | the time, struck through | Skipped · tap to bring back | **tap = bring it back** |
| No alarm tomorrow | "No alarm" | Turn the edge to set one | turn = set one for tomorrow only |

An override is used once and then clears itself (R3). It also clears if you turn the dial back to the usual time.

### 5.4 Week (swipe down)
```
        .-~~~~~~~~~~~~-.
      .'  Mon     Tue   '.        seven wedges around the ring, each:
     /  6:45     6:45     \       day letters + time, or "off"
    | Sun                Wed |     tap a wedge to open that day
    | off     NEXT       6:45|
    | Sat    Wed 06:45   Thu |
     \  off   tap a day  6:45 /
      '.     Fri 6:45    .'
        '-.~~~~~~~~~~~~.-'
```
- Days that are on glow amber. Days that are off are dim with "off".
- The centre shows the next alarm. It is information only, not a button.
- Swipe up returns to the clock.

### 5.5 Day (tap a day on Week)
```
        .-~~~~~~~~~~~~-.          rim: 24-hour dial
      .'  Every Wednesday'.
     /                      \
    |        06:45           |    turn the edge to set the time
    |   On · hold to turn off |
     \       o o * o o o o  /     dots: which day (swipe sideways to move)
      '.                  .'
        '-.~~~~~~~~~~~~.-'
```

| State | Sub-line | Gestures |
|-------|----------|----------|
| On | On · hold to turn off | turn = time · **hold 1 s = off** |
| Off | Off · tap to turn on | **tap = on** · turning the dial also turns it on |

- **Carry-over:** swiping sideways to a day that is off starts it at the time you just set. Entering Monday to Friday is Monday (3 gestures) plus 2 per extra day (swipe, tap).
- Swipe up returns to Week.

### 5.6 Sound (swipe left)
```
        .-~~~~~~~~~~~~-.          rim: volume arc (270°, open at the bottom)
      .'                '.
     /      ||  (pause)   \
    |      White noise     |
    |   Playing · Volume 40 |
     \   [ 30 min timer ]   /      lower zone, at least 100 px tall: tap to cycle
      '.                  .'      15 · 30 · 60 · 90 min · all night
        '-.~~~~~~~~~~~~.-'
```
- **Tap** the middle = play / pause. **Turn the edge** = volume. **Tap the lower zone** = cycle the sleep timer.
- It remembers the last volume and timer, so starting sleep sound from the clock is **swipe left, tap** (2 gestures).
- Swipe up/down is reserved to change the sound in Phase 5. Swipe right returns to the clock.
- The sound fades out by itself when the timer ends or the sunrise begins (R10). No screen is needed.

### 5.7 Light (swipe right)
```
        .-~~~~~~~~~~~~-.          rim: brightness arc
      .'                '.
     /    Night light     \
    |         60%          |
    |   Tap to turn off     |
     \  [ Switch to reading ] /     lower zone: tap to switch the colour
      '.                  .'
        '-.~~~~~~~~~~~~.-'
```
- **Tap** the middle = light on / off. **Turn the edge** = brightness. **Tap the lower zone** = night light (deep red) or reading light (warm white).
- Each mode remembers its own brightness. Swipe left returns to the clock.
- Night light is capped very low by default, red only.

### 5.8 Sunrise in progress (clock face)
```
        .-~~~~~~~~~~~~-.
      .'                '.
     /                    \
    |        06:42         |
    |  Sunrise · alarm 07:00 |
     \                    /
      '.                .'
        '-.~~~~~~~~~~~.-'
```
- The face stays at night dimness. The ring does the waking.
- **Hold 1.5 s anywhere** stops the sunrise and the alarm for today ("I'm up"). A tap only wakes the face.

### 5.9 Ringing
```
        .-~~~~~~~~~~~~-.          screen at full brightness, warm white
      .'                '.
     /        07:00         \
    |   Tap to snooze · 2 left |
    |    Hold to turn off     |
     \  (rim fills as you hold) /
      '.                  .'
        '-.~~~~~~~~~~~~.-'
```
- **Tap anywhere** = snooze 9 min (up to 3; the count shows). **Hold 2 s** = dismiss.
- After the third snooze the text becomes "No snoozes left" and a tap does nothing but flash it.
- Fallback if the screen is dead: hold the **BOOT** button for 3 s (D-007).
- After 30 min without a response the sound stops and the face shows "Alarm 07:00 timed out" until touched (D-015).

### 5.10 After a snooze
```
    |        06:58         |
    |  Snoozed · rings 07:09 |
```
Hold 1.5 s cancels the snooze (turns the alarm off for today).

### 5.11 Settings (hold the clock 1.5 s)

Pages sit side by side. **Swipe left/right** to move, **turn the edge** to change the value, **swipe down** to leave. It auto-returns after 30 s.
```
        .-~~~~~~~~~~~~-.
      .'  Sunrise length '.
     /                    \
    |        30 min        |       turn the edge: 10 to 60 min
    |    < 3 of 8 >         |
     \                    /
      '.                .'
        '-.~~~~~~~~~~~~.-'
```

| # | Page | Values | Notes |
|---|------|--------|-------|
| 1 | Time zone | Eastern (default), Central, Mountain, Pacific, Alaska, Hawaii, UTC | Phone sync can set it (Phase 6). |
| 2 | Set time | Hours, then minutes (tap switches the field) | Opens by itself if the time is untrusted (§8). Wi-Fi or phone sets it automatically. |
| 3 | Sunrise length | 10–60 min, default 30 | |
| 4 | Alarm loudness | 30–100 %, default 70 | Never below 30 %. |
| 5 | Night dimness | 1–5, default 2 | The always-on face. Level 1 is the dimmest. |
| 6 | Preview alarm | Hold 1.5 s to start | A compressed 60 s run: sunrise, sound steps, snooze, dismiss. Also checks the ring and speaker (R26). |
| 7 | Wi-Fi | Shows connected / not connected | Read-only until Phase 6 (credentials come from the phone). |
| 8 | About | Version, battery %, USB voltage, last reset reason | For fault finding. |

### 5.12 First run (D-015: no default alarm)

1. Power on. If the time is untrusted, **Set time** opens (§8). Otherwise the clock shows **"No alarm set"** and a hint: "Swipe down to add one" for the first 5 minutes.
2. **Swipe down** (Week, all days off) → **tap a day** (Day, shows 07:00 dimmed as the start) → **turn the edge** to the time. The day turns on at the first turn.
3. Swipe sideways to the next day, tap (it takes the same time), and repeat.

**First alarm: 3 gestures. Mon–Fri: 11 gestures.**

## 6. Edit flows (gesture count)

| Task | Gestures | Steps |
|------|----------|-------|
| Change Wednesday's time for good | **3** | swipe down · tap Wed · turn the edge |
| Wake earlier tomorrow only | **2** | swipe up · turn the edge |
| Skip tomorrow | **2** | swipe up · hold 1 s |
| Undo a skip | **2** | swipe up · tap |
| Turn Saturday on | **3** | swipe down · tap Sat · (optional turn) |
| Turn a day off | **3** | swipe down · tap the day · hold 1 s |
| Start white noise | **2** | swipe left · tap |
| Change the sleep timer | **3** | swipe left · tap the lower zone (cycle) · (swipe right) |
| Night light on | **2** | swipe right · tap |
| Snooze | **1** | tap anywhere |
| Dismiss | **1** | hold 2 s |
| Set Mon–Fri at one time | **11** | 3 for Monday + 2 for each next day |

The 3-gesture weekday edit has a budget of **10 s** (R2): about 1 s per swipe or tap and up to 6 s of turning.

## 7. Alarm lifecycle on the screen

| Engine state | Clock face | Ring | Sound |
|--------------|-----------|------|-------|
| Idle | time + next alarm | off | as chosen |
| White noise running | time + next alarm | off | noise, volume set |
| Sunrise (from alarm − length) | "Sunrise · alarm 07:00" | ramps up | noise fades out over the first 2 min |
| Ringing (alarm time) | Ringing screen, full brightness | full | soft tone, stepping louder each minute |
| Snoozed | "Snoozed · rings 07:09" | off | silent |
| Timed out (30 min) | "Alarm 07:00 timed out" until touched | stays on | stops |
| Dismissed | "Good morning" for 2 s, then the clock | off | stops |

On battery (no USB) the ring is dark, so during a sunrise and the alarm the **screen ramps to full warm white** as a smaller light (D-012).

## 8. Status line and warnings

One line under the next alarm, highest priority first. A lower one waits.

| Priority | Condition | Text | Also |
|----------|-----------|------|------|
| 1 | Time untrusted (clock lost power with no battery) | **Set time** (opens the Set time page) | Alarms still try to ring from the last known time. |
| 2 | Alarm timed out, not touched | Alarm 07:00 timed out | Cleared by a tap. |
| 3 | Stored alarms were damaged and reset | Alarms reset · swipe down to add | Soft chime on the next touch. |
| 4 | Next alarm could be missed on battery (R24) | **Plug me in** · 31% | Chime at bedtime. |
| 5 | Battery under 25 % | Battery 22% | |
| 6 | On battery | a small dot beside the time | No text. |

The 6th level is a dot, so the face stays quiet when everything is fine.

## 9. Night theme

| Item | Spec |
|------|------|
| Colour | Amber only. Blue under 15 % of the colour (blue light delays sleep). No white until the alarm rings. |
| Night palette (start values, tune on the panel) | time `#E0801C` · secondary `#A55F1A` · hint `#7A4A18` · skipped / warning `#C0502A` · changed-for-tomorrow `#4FA58D` |
| Awake palette (tap-wake and screens) | time `#FFB869` · secondary `#B27C4A` · hint `#6F5843` |
| Ringing palette | time `#FFF0DC` (warm white) |
| Panel brightness | Night: the dimness level (1–5) mapped to about 1–6 % of the panel's range. Tap-wake and screens: about 25 %. Ringing: 100 %. |
| How it dims | **Through the panel's brightness setting, not by darkening the colours.** Dark colours show colour banding on AMOLED. The panel setting keeps the full 256 levels. To be checked on the real panel (UX-B1). |
| No light sensor | The board has none, so night level is always on. A tap lifts it to 25 % for 5 s for daytime reading. If that proves too dim in daylight, a Day/Night schedule is the fix (open decision). |
| Burn-in (D-016) | Clock face only. It moves one position a minute along a 12-point circle of radius 6 px, so the same pixels aren't lit all night. Nothing else is drawn at night. |
| Large type | The time at 150 px needs a custom digits-only font (0–9 and colon) built with `lv_font_conv` in Phase 3. The bring-up test's largest built-in font is 48 px. |

## 10. Acceptance checks (UX)

Each becomes a numbered test in TEST_PLAN.md in Phase 3.

| ID | Check | Requirement |
|----|-------|-------------|
| UX-1 | Change Wednesday's time in ≤ 3 gestures and ≤ 10 s | R2 |
| UX-2 | Tomorrow-only change in 2 gestures; the override clears itself after it fires | R3 |
| UX-3 | A tap alone cannot skip tomorrow, turn a day off or dismiss an alarm | R34, R23 |
| UX-4 | A palm or a 3 s accidental press during an alarm snoozes at most. It never dismisses (no 2 s still hold) | R23 |
| UX-5 | Eyes closed, in the dark: find the clock and snooze within 2 s | R5, R23 |
| UX-6 | Night face reads from 1 m in a dark room and doesn't light the room | R6 |
| UX-7 | Any screen returns to the clock after 20 s idle (settings 30 s), with changes kept | R35 |
| UX-8 | The face position changes once a minute | R6 |
| UX-9 | The second line always matches the engine's real next alarm, including overrides and skips | R4 |
| UX-10 | First alarm on a fresh clock in 3 gestures | R1, R5 |
| UX-11 | Mon–Fri in ≤ 11 gestures with the carry-over rule | R1 |
| UX-12 | Every tap zone is at least 100 px, and a swipe can start anywhere including the rim | R5 |
| UX-B1 | Bench: dimmest panel level shows no visible colour banding or flicker at night | R6 |

## 11. What the concept page does and doesn't show

The published concept page implements the gesture model for the clock, Tomorrow, Week, Day, Sound, Light and Ringing. It was updated for the hold-to-turn-off rule (D-024). It does **not** yet show settings, first run, warnings or the sunrise screen; those exist only as the wireframes above.
