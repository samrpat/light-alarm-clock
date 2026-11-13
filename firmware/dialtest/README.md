# Dial and swipe test

A small program for the real board. It is **not** the alarm clock firmware. It tests the touch dial and checks that swiping never gets confused with turning.

- Two dots on two rings, like clock hands. **Outer dot: minutes** (5 minutes per 30°, the hour moves on by itself). **Inner dot: hours** (1 hour per 30°).
- Touch a dot, pause a moment, then slide it around its ring.
- Swipe anywhere else, in any direction, and the time must not change.
- There are no buttons. Taps do nothing to the time.
- The screen counts dial turns, swipes, taps and ignored touches, and says how your last touch was understood.

What to try: swipe left, right, up and down starting near the rim, next to a dot, and in the middle. Watch for the time changing when you did not turn a dot. Also try grabbing a dot with no pause (a quick flick should count as a swipe) and with a pause (it should turn).

## Flash it

Close any serial monitor first. Then, with the board on USB:

```bash
. ~/esp/esp-idf/export.sh
```

```bash
cd ~/Documents/Projects/Claude/alarm/firmware/dialtest && idf.py flash
```

To go back to the earlier hardware test, flash `firmware/bringup` the same way.
