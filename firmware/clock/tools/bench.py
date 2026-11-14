#!/usr/bin/env python3
"""Bench tool for the clock firmware. One program owns the USB port at a time (a lock file), it never resets the
board by accident, and every check prints PASS or FAIL.

  bench.py flash            build, flash, wait for READY
  bench.py status           one status line
  bench.py cmd "<line>"     send one console command (see main/console.c)
  bench.py selftest         engine checks on the real board with light and sound MUTED (about 2 minutes)
  bench.py soundcheck       the real alarm test of the Alarm page: 6 s of the alarm sound and the ring (R26)
  bench.py real             one REAL run: a real alarm 32 minutes ahead, nothing muted, nothing sped up.
                            Stop it the real way: tap = snooze, hold 2 s = stop (or hold BOOT 3 s).
Run inside the ESP-IDF environment (. ~/esp/esp-idf/export.sh) for pyserial.
"""
import sys, os, time, glob, fcntl, subprocess, datetime as dt
from zoneinfo import ZoneInfo
import serial

HERE = os.path.dirname(os.path.abspath(__file__)); PROJ = os.path.dirname(HERE)
TZ = ZoneInfo("America/New_York")          # the firmware's default region (D-017)
LOCK = "/tmp/clock-bench.lock"
NAMES = {0: "IDLE", 1: "DAWN", 2: "RINGING", 3: "SNOOZED", 4: "MISSED"}

def lock():
    f = open(LOCK, "a+")
    try: fcntl.flock(f, fcntl.LOCK_EX | fcntl.LOCK_NB)
    except BlockingIOError:
        f.seek(0); sys.exit(f"Another bench run holds the port ({f.read().strip() or 'unknown'}). Stop it first.")
    f.seek(0); f.truncate(); f.write(f"pid {os.getpid()} {' '.join(sys.argv[1:])}"); f.flush(); return f

def find_port():
    p = sorted(glob.glob("/dev/cu.usbmodem*"))
    if not p: sys.exit("Board not found on USB.")
    return p[0]

class Board:
    def __init__(self):
        s = serial.Serial(); s.port = find_port(); s.baudrate = 115200; s.timeout = 0.1
        s.dtr = False; s.rts = False; s.open(); self.s = s; self.buf = b""
        # On macOS, opening the port usually restarts the board (USB control lines). Be deterministic about it:
        # if a boot log appears, wait for the firmware's READY line before the first command.
        seen = b""; end = time.time() + 1.5
        while time.time() < end: seen += s.read(4096)
        if b"boot:" in seen or b"ESP-ROM" in seen or b"cpu_start" in seen:
            end = time.time() + 12
            while b"READY" not in seen and time.time() < end: seen += s.read(4096)
            time.sleep(0.4)
        s.reset_input_buffer()
    def lines(self, secs):
        end = time.time() + secs
        while time.time() < end:
            self.buf += self.s.read(4096)
            while b"\n" in self.buf:
                l, self.buf = self.buf.split(b"\n", 1); yield l.decode(errors="replace").strip()
    def cmd(self, line, want=("OK", "STATUS", "ERR"), secs=4):
        for attempt in range(3):
            self.s.write((line + "\n").encode()); self.s.flush()
            for l in self.lines(secs):
                if l.startswith(want): return l
        raise RuntimeError(f"no answer to '{line}'")
    def status(self):
        l = self.cmd("status", want=("STATUS",)); return {k: int(v) for k, v in (kv.split("=") for kv in l.split()[1:])}
    def wait(self, pred, secs, what):
        end = time.time() + secs; st = None
        while time.time() < end:
            st = self.status()
            if pred(st): return st
            time.sleep(0.4)
        raise AssertionError(f"timed out waiting for {what}; last {st}")
    def reboot(self):
        try: self.s.write(b"reboot\n"); self.s.flush()
        except (serial.SerialException, OSError): pass
        time.sleep(0.5)
        try: self.s.close()
        except Exception: pass
        time.sleep(6.0)                                       # let it boot fully before the port is opened again
        end = time.time() + 30
        while time.time() < end:
            time.sleep(1.0)
            try: self.__init__(); self.status(); return
            except (serial.SerialException, OSError, RuntimeError, SystemExit): pass
        raise RuntimeError("board did not come back")

def local_parts(utc):
    d = dt.datetime.fromtimestamp(utc, TZ); return d.weekday(), d.hour, d.minute
def plus_week(utc):
    d = dt.datetime.fromtimestamp(utc, TZ); n = (d.replace(tzinfo=None) + dt.timedelta(days=7)).replace(tzinfo=TZ); return int(n.timestamp())
def minute_after(now, secs): return (int(now + secs) // 60 + 1) * 60

class Checks:
    def __init__(self): self.n = 0; self.bad = []
    def ok(self, cond, text, detail=""):
        self.n += 1; print(("  PASS  " if cond else "  FAIL  ") + text + (f"   [{detail}]" if detail and not cond else ""), flush=True)
        if not cond: self.bad.append(text)

def selftest(b):
    c = Checks(); real_now = int(time.time())
    def set_day(T): wd, h, m = local_parts(T); b.cmd(f"day {wd} {h:02d}:{m:02d}")
    print("Self-test: light and sound are muted, the clock is moved about on purpose.", flush=True)
    try:
        b.cmd("mute on"); b.cmd("dismiss"); b.cmd("clear"); b.cmd(f"time @{real_now}")
        st = b.wait(lambda s: s["state"] == 0 and s["next"] == 0, 5, "idle and empty"); c.ok(True, "empty schedule: idle, no alarm")
        c.ok(st["mute"] == 1 and st["trusted"] == 1, "muted, clock trusted after setting", str(st))
        T = minute_after(real_now, 45 * 60); set_day(T)
        st = b.wait(lambda s: s["next"] == T, 5, "next alarm"); c.ok(st["state"] == 0, "weekday alarm stored; next alarm is the right instant")
        b.cmd(f"time @{T - 1803}"); st = b.status(); c.ok(st["state"] == 0, "3 s before the dawn: still idle", str(st))
        st = b.wait(lambda s: s["state"] == 1, 8, "dawn"); c.ok(-1800 <= st["dawn_t"] <= -1796 and st["light"] == 1, "dawn starts 30:00 before the alarm", str(st))
        b.cmd(f"time @{T - 4}"); st = b.wait(lambda s: s["state"] == 2, 9, "ringing"); c.ok(st["stage"] == 1 and st["el"] <= 3 and abs(st["now"] - T) <= 3, "rings at the alarm time, first step", str(st))
        for k, at in ((2, 61), (3, 121), (4, 181)):
            b.cmd(f"time @{T + at}"); st = b.wait(lambda s: s["stage"] == k, 4, f"step {k}"); c.ok(abs(st["dawn_t"] - at) <= 4, f"sound step {k} at +{at} s; daylight stage follows", str(st))
        b.cmd("snooze"); st = b.wait(lambda s: s["state"] == 3, 3, "snoozed"); c.ok(st["stage"] == 0 and st["light"] == 1 and st["snz"] == 2, "snooze: silent at once, light stays, 2 snoozes left", str(st))
        b.cmd(f"time @{st['now'] + 538}"); st = b.wait(lambda s: s["state"] == 2, 8, "rings after snooze"); c.ok(st["stage"] == 1, "rings again 9 minutes after the snooze, from the first step", str(st))
        b.cmd("snooze"); b.wait(lambda s: s["state"] == 3, 3, "snooze 2"); b.cmd(f"time @{b.status()['now'] + 541}"); b.wait(lambda s: s["state"] == 2, 6, "ring 3")
        b.cmd("snooze"); st = b.wait(lambda s: s["state"] == 3, 3, "snooze 3"); b.cmd(f"time @{st['now'] + 541}"); b.wait(lambda s: s["state"] == 2, 6, "ring 4")
        b.cmd("snooze"); time.sleep(1.2); st = b.status(); c.ok(st["state"] == 2 and st["snz"] == 0, "a fourth snooze is refused: still ringing", str(st))
        T7 = plus_week(T); b.cmd("dismiss"); st = b.wait(lambda s: s["state"] == 0, 3, "dismissed"); st = b.wait(lambda s: s["next"] == T7, 4, "next week")
        c.ok(st["light"] == 0 and st["stage"] == 0, "dismiss: off; next alarm is the same time next week")
        b.cmd(f"time @{T7 - 900}"); b.wait(lambda s: s["state"] == 1, 6, "dawn 2"); b.reboot()
        st = b.status(); c.ok(st["state"] == 1 and -900 <= st["dawn_t"] <= -860 and st["trusted"] == 1 and st["mute"] == 1, "restart in the middle of the dawn: resumes at the right point, time kept", str(st))
        b.cmd("dismiss"); T14 = plus_week(T7); st = b.wait(lambda s: s["state"] == 0 and s["next"] == T14, 5, "cancelled"); c.ok(True, "hold during the dawn cancels it: that day is skipped")
        b.cmd("unskip"); st = b.wait(lambda s: s["state"] == 1 and s["next"] == T7, 6, "re-opened"); c.ok(True, "bringing the day back re-opens the alarm (dawn resumes)")
        b.cmd("dismiss"); b.cmd("clear"); b.wait(lambda s: s["state"] == 0 and s["next"] == 0, 5, "cleared")
        T2 = minute_after(real_now, 3 * 3600); set_day(T2); b.cmd(f"time @{T2 - 3000}"); b.wait(lambda s: s["next"] == T2 and s["state"] == 0, 5, "armed")
        time.sleep(1.5); b.cmd(f"rawtime @{T2 + 600}"); b.reboot()
        st = b.wait(lambda s: s["state"] == 2, 5, "catch-up"); c.ok(st["stage"] <= 1 and st["el"] <= 20, "power off through the alarm, back 10 min late: rings at once, softly", str(st))
        b.cmd("dismiss"); b.cmd("clear"); b.wait(lambda s: s["state"] == 0 and s["next"] == 0, 5, "cleared")
        T3 = minute_after(real_now, 6 * 3600); set_day(T3); b.cmd(f"time @{T3 - 3000}"); b.wait(lambda s: s["next"] == T3 and s["state"] == 0, 5, "armed")
        time.sleep(1.5); b.cmd(f"rawtime @{T3 + 3700}"); b.reboot(); time.sleep(2)
        st = b.status(); c.ok(st["state"] == 0 and st["next"] == plus_week(T3), "back more than an hour late: does not ring, next week is armed", str(st))
        b.s.write(b"dump\n"); dump = [l for l in b.lines(2.0) if "DUMP" in l]
        c.ok(len(dump) == 2 and all("usable=1" in l for l in dump), "both stored schedule copies are good after all of this", " | ".join(d[-60:] for d in dump))
    except (AssertionError, RuntimeError) as e:
        c.ok(False, f"stopped: {e}")
    finally:
        try:
            b.cmd("dismiss"); b.cmd("clear"); b.cmd(f"time @{int(time.time())}"); b.cmd("mute off")
            st = b.wait(lambda s: s["state"] == 0 and s["next"] == 0 and s["mute"] == 0, 6, "clean"); c.ok(abs(st["now"] - time.time()) <= 2 and st["trusted"] == 1, "left clean: real time, no alarms, not muted")
        except Exception as e: c.ok(False, f"cleanup failed: {e}")
    print(f"\n{c.n - len(c.bad)}/{c.n} passed" + ("" if not c.bad else "  FAILED: " + "; ".join(c.bad)), flush=True); return not c.bad

def real(b):
    now = int(time.time()); b.cmd("mute off"); b.cmd("dismiss"); b.cmd("clear"); b.cmd(f"time @{now}")
    T = minute_after(now, 31 * 60 + 30); wd, h, m = local_parts(T); b.cmd(f"day {wd} {h:02d}:{m:02d}")
    st = b.wait(lambda s: s["next"] == T, 5, "alarm set")
    f = lambda u: dt.datetime.fromtimestamp(u, TZ).strftime("%-I:%M:%S %p")
    os.makedirs(os.path.join(HERE, "logs"), exist_ok=True); logp = os.path.join(HERE, "logs", time.strftime("real-%Y%m%d-%H%M%S.log"))
    print(f"REAL RUN  alarm {f(T)}  dawn starts {f(T - 1800)}  log {logp}", flush=True)
    log = open(logp, "w"); c = Checks(); prev = None; seen = {}; t_end = T + 45 * 60
    while time.time() < t_end:
        try: st = b.status()
        except RuntimeError as e: print("  (no answer, retrying)", e, flush=True); time.sleep(2); continue
        log.write(f"{time.strftime('%H:%M:%S')} {st}\n"); log.flush(); key = (st["state"], st["stage"])
        if prev is None or key != prev:
            print(f"  {time.strftime('%H:%M:%S')}  {NAMES[st['state']]}  step {st['stage']}  dawn_t {st['dawn_t']}", flush=True)
            if key not in seen: seen[key] = st
        if prev and prev[0] in (1, 2, 3, 4) and st["state"] == 0: break
        side = os.path.join(HERE, "send.txt")                 # a console line dropped here is sent during the run (e.g. vol 90)
        if os.path.exists(side):
            for line in open(side).read().splitlines():
                if line.strip(): print("  sent:", line.strip(), "->", b.cmd(line.strip()), flush=True)
            os.remove(side)
        prev = key; time.sleep(1.0 if abs(st["now"] - T) < 240 or abs(st["now"] - (T - 1800)) < 20 else 10.0)
    d = seen.get((1, 0)); c.ok(bool(d) and -1800 <= d["dawn_t"] <= -1797, "dawn began 30:00 before the alarm", str(d))
    r = seen.get((2, 1)); c.ok(bool(r) and abs(r["now"] - T) <= 2, "alarm rang on the second", str(r))
    for k in (2, 3, 4):
        s = seen.get((2, k)); c.ok(s is None or abs(s["el"] - 60 * (k - 1)) <= 2, f"sound step {k} came on time" + ("" if s else " (not reached: stopped earlier)"), str(s))
    c.ok(prev is not None and st["state"] == 0, "ended by a stop (hold, or BOOT button)")
    b.cmd("clear"); print(f"\n{c.n - len(c.bad)}/{c.n} passed" + ("" if not c.bad else "  FAILED: " + "; ".join(c.bad)) + f"\nlog: {logp}", flush=True); return not c.bad

def main():
    if len(sys.argv) < 2: sys.exit(__doc__)
    what = sys.argv[1]; lk = lock()
    if what == "flash":
        r = subprocess.run(["idf.py", "-p", find_port(), "flash"], cwd=PROJ, capture_output=True, text=True)
        if r.returncode: print(r.stdout[-1500:], r.stderr[-1500:]); sys.exit("FLASH FAILED")
        time.sleep(2.5); b = Board(); print("flashed;", b.cmd("status")); return
    b = Board()
    if what == "status": print(b.cmd("status"))
    elif what == "cmd": print(b.cmd(sys.argv[2]))
    elif what == "layoutcheck":
        b.s.write(b"layoutcheck\n")
        for l in b.lines(60):
            if l.startswith("LAYOUT"): print(l)
            if l.startswith("LAYOUTCHECK"): break
    elif what == "shot":                      # shot <out.png> ["ui args"] ...  (several pairs allowed)
        for i in range(2, len(sys.argv), 2):
            if i + 1 < len(sys.argv): b.cmd("ui " + sys.argv[i + 1]); time.sleep(0.4)
            b.s.write(b"shot\n"); rows = []; w = h = 0
            for l in b.lines(30):
                if l.startswith("SHOT END"): break
                if l.startswith("SHOT "): w, h = map(int, l.split()[1:3]); continue
                if w and len(l) == w * 4: rows.append(l)
            import zlib, struct
            raw = b"".join(b"\0" + b"".join(bytes(((v >> 11) * 255 // 31, (v >> 5 & 63) * 255 // 63, (v & 31) * 255 // 31)) for v in (int(r[k:k + 4], 16) for k in range(0, w * 4, 4))) for r in rows)
            ch = lambda t, d: struct.pack(">I", len(d)) + t + d + struct.pack(">I", zlib.crc32(t + d))
            open(sys.argv[i], "wb").write(b"\x89PNG\r\n\x1a\n" + ch(b"IHDR", struct.pack(">IIBBBBB", w, len(rows), 8, 2, 0, 0, 0)) + ch(b"IDAT", zlib.compress(raw)) + ch(b"IEND", b""))
            print("saved", sys.argv[i], w, len(rows))
        b.cmd("ui live")
    elif what == "soundcheck": b.cmd("mute off"); print(b.cmd("test"))
    elif what == "selftest": sys.exit(0 if selftest(b) else 1)
    elif what == "real": sys.exit(0 if real(b) else 1)
    else: sys.exit(__doc__)
    del lk

if __name__ == "__main__": main()
