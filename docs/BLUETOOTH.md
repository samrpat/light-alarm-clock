# Bluetooth time sync (optional)

2026-11-13 · Decision: [D-043](DECISIONS.md) · Requirements: R16, N1, N3 · Phase 6

**Scope:** set the clock's time from your phone. Nothing else for now. It is a convenience: the clock works fully without it, and the time can always be set by hand (hours, minutes and seconds).

## How it works for you

1. Settings → **Phone** page → tap.
2. The clock turns Bluetooth on for **about two minutes** and shows "Searching".
3. Your phone finds it. The clock shows a **six-digit code** ("Code 482 913"); confirm it on the phone.
4. The clock reads the time, shows "Time synced", and **turns Bluetooth off again**.

The first time pairs the phone (a bond is stored). Later syncs need only the tap and no code.

## What is sent
Only the current date and time. Region and daylight saving stay on the clock (a wrong phone time zone can never change your alarm). The clock refuses a time before 2026 or after 2099.

## Safety rules
- Bluetooth is **off unless you tap Phone**, and off again after two minutes or after a sync.
- A code is shown on the clock for every new phone (LE Secure Connections, numeric comparison), so a stranger cannot pair.
- Only the paired phone may write the time.
- Wi-Fi stays **never used**. No server, account or cloud anywhere (D-030 still holds for everything else).
- The Bluetooth code runs on the non-critical core. If it ever misbehaves, it cannot delay an alarm (D-002), and it can be removed without touching the alarm path.

## The phone side: options (I have not tested any of these)

| Option | What you need | Catch |
|--------|---------------|-------|
| **A. iPhone Current Time Service** | Nothing: iPhones offer their time to paired Bluetooth accessories | I could not verify it works with this board. iPhone only. |
| **B. A small web page using Web Bluetooth** | Chrome on Android or a computer | **iPhone Safari does not support Web Bluetooth.** Needs HTTPS, so it would be hosted (for example on this repository's GitHub page), which is a small dependency on a host, only for the convenience. |
| **C. A generic Bluetooth app** (nRF Connect, LightBlue) | The app | You write the time by hand into a characteristic. Works, but clumsy. |
| **D. A small custom app** | Building and keeping an app | The most work and the most to maintain |

**Recommendation:** make the clock speak the standard **Current Time** characteristic (0x2A2B), so options A and C work with no custom code, and add a tiny web page for B if you use Android. **What phone do you have?** That decides which is worth building.

## Power and size
Advertising for two minutes costs a few milliamps for a short time: negligible against the battery. The Bluetooth stack adds roughly 100 to 150 KB of code (my estimate), well inside the 4 MB program slot.

## Seconds without a phone
On the Time page, tap the seconds line: the clock **pauses**, drag the dot to the seconds you want (one step per second), then tap to **start at the beep**. This is how you set it to a time signal when no phone is handy.
