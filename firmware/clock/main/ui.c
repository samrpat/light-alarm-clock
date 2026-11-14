/* Gestures and state for the interface of UX.md: swipe goes, tap does, drag a dot changes, hold switches (D-036).
 * Carried over from the concept page's logic. Runs inside the LVGL task on core 0. It talks to the alarm engine
 * only by messages and reads only its snapshot, so nothing here can delay an alarm (R18). */
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "esp_timer.h"
#include "esp_log.h"
#include "bsp/esp-bsp.h"
#include "lvgl.h"
#include "ui.h"
#include "gfx.h"
#include "hw.h"
#include "store.h"
#include "tz.h"
#include "civil.h"

#define RR 188
#define RH 148
static const char *TAG = "ui";
static ui_t S; static lv_obj_t *canvas; static tz_t tz; static sched_t sch;
static int64_t last_touch_us, aw_t0, done_until, hold_refresh_until, awake_until; static float aw_from, aw_to; static bool aw_anim;
static int32_t tgt_day; static bool bench_frozen;
static const char *REG_TZ[REG_COUNT] = {"EST5EDT,M3.2.0/2,M11.1.0/2", "CST6CDT,M3.2.0/2,M11.1.0/2", "MST7MDT,M3.2.0/2,M11.1.0/2", "PST8PDT,M3.2.0/2,M11.1.0/2",
                                        "AKST9AKDT,M3.2.0/2,M11.1.0/2", "HST10", "GMT0BST,M3.5.0/1,M10.5.0/2", "CET-1CEST,M3.5.0/2,M10.5.0/3", ""};
static const int TIMER_N[5] = {15, 30, 60, 90, 0};

static int wrap(int m) { return ((m % 1440) + 1440) % 1440; }
static int64_t us(void) { return esp_timer_get_time(); }
static void redraw(void) { if (canvas) lv_obj_invalidate(canvas); }
static void load_tz(void) { char z[64]; store_load_region(z, sizeof z); if (!tz_parse(z, &tz)) tz_parse(REG_TZ[0], &tz); }
static int wake_min(void) { if (S.days[S.wd].on) return S.days[S.wd].min; for (int i = 0; i < 7; i++) if (S.days[i].on) return S.days[i].min; return 420; }
static int phase_p(float *w) {
    int wk = wake_min(), span = wrap(S.ns - wk), since = wrap(S.now - wk);
    if (since < span) { int left = span - since; if (left > 180) { *w = 0; return 0; } *w = 1 - left / 180.0f; return 1; }
    *w = 1; return 2;
}

/* ---- the world: time, schedule, engine state ---- */
static void refresh(void) {
    if (bench_frozen) return;
    int64_t now = (int64_t)time(NULL), loc = tz_utc_to_local(&tz, now); int32_t today = (int32_t)floor_div(loc, 86400); int sod = (int)(loc - (int64_t)today * 86400);
    S.now = sod / 60; S.wd = weekday_from_days(today); civil_from_days(today, &S.y, &S.mo, &S.d);
    bool on_time = S.scr == SCR_SET && S.pg == PG_TIME;
    S.secs = on_time ? sod % 60 : 0;
    if (on_time && !S.sec_mode && !S.drag) { S.clk = S.now; S.sec = sod % 60; }
    if (!(S.scr == SCR_SET && S.pg == PG_DATE)) { S.dat.m = S.mo - 1; S.dat.d = S.d; S.dat.y = S.y; }
    alarm_snapshot_t e; alarm_snapshot_get(&e);
    if (us() > hold_refresh_until && !S.drag) {
        alarm_sched_get(&sch);
        for (int i = 0; i < 7; i++) { if (S.scr == SCR_DAY && i == S.sel && !sch.day[i].on) { S.days[i].on = false; continue; } S.days[i] = sch.day[i]; }   /* an off day keeps the carried-over time */
        int cand = sch.one_day == today ? sch.one_min : sch.day[S.wd].on ? sch.day[S.wd].min : -1;
        S.tgt_today = cand >= 0 && S.now < cand; tgt_day = today + (S.tgt_today ? 0 : 1); int twd = weekday_from_days(tgt_day);
        S.tgt_usual = sch.day[twd].on ? sch.day[twd].min : -1; S.tgt_one = sch.one_day == tgt_day ? sch.one_min : -1; S.tgt_skip = sch.skip_day == tgt_day;
    }
    S.eng_state = e.state; S.dawn_t = e.state == SM_SUNRISE ? e.dawn_t_s / 2 * 2 : 0; S.snz_left = e.snoozes_left;
    S.has_next = e.next_utc != 0;
    if (S.has_next) { int64_t nl = tz_utc_to_local(&tz, e.next_utc); int32_t nd = (int32_t)floor_div(nl, 86400); S.next_wd = weekday_from_days(nd); S.next_min = (int)((nl - (int64_t)nd * 86400) / 60); S.next_far = e.next_utc - now > 86400; }
    static const char *DN[7] = {"Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"};
    S.status[0] = 0; S.status_warn = true;
    if (e.no_time) snprintf(S.status, sizeof S.status, "Set time");
    else if (e.sched_damaged) snprintf(S.status, sizeof S.status, "Alarms lost: set again");
    else if (g_time_estimated) snprintf(S.status, sizeof S.status, "Check time");
    else if (e.state == SM_MISSED) snprintf(S.status, sizeof S.status, "Alarm timed out");
    else if (e.state == SM_SNOOZED) { S.status_warn = false; snprintf(S.status, sizeof S.status, "Snoozed"); }
    else if (sch.skip_day != NO_DAY && sch.skip_day >= today) { S.status_warn = false; snprintf(S.status, sizeof S.status, "%s skipped", DN[weekday_from_days(sch.skip_day)]); }
    /* the alarm takes the screen, and gives it back */
    if (e.state == SM_RINGING && S.scr != SCR_RING) { S.scr = SCR_RING; S.done = false; S.hold = 0; S.pick = 0; S.fo.menu = false; }
    if (S.scr == SCR_RING && e.state != SM_RINGING && !(S.done && us() < done_until)) { S.scr = SCR_CLOCK; S.done = false; }
}

/* ---- brightness: day setting, a smooth wind-down, the dimmest readable level at night (UX.md section 7) ---- */
#define NIGHT_LEVEL 6
static void brightness(void) {
    float w; int p = phase_p(&w); float ws = w * w * (3 - 2 * w), lvl;
    if (S.scr == SCR_RING) lvl = 100;
    else if (S.eng_state == SM_SUNRISE) { float v = (S.dawn_t / 60.0f + 30) / 40; v = v < 0 ? 0 : powf(v, 1.6f); lvl = NIGHT_LEVEL + (100 - NIGHT_LEVEL) * (v > 1 ? 1 : v); }
    else if (p == 2) lvl = NIGHT_LEVEL + 14 * S.aw + (us() < awake_until ? 10 : 0);
    else lvl = S.dayB + (NIGHT_LEVEL - S.dayB) * ws;
    static int last = -1; int v = (int)lroundf(lvl); if (v != last) { last = v; bsp_display_brightness_set(v); }
}

/* ---- commits: one message to the engine when the finger lifts ---- */
static void set_clock_from_page(void) {
    civil_t c = {S.dat.y, S.dat.m + 1, S.dat.d, 0, 0, 0}; if (c.d > days_in_month(c.y, c.m)) c.d = days_in_month(c.y, c.m);
    int64_t local = civil_to_secs(&c) + (int64_t)S.clk * 60 + S.sec; rtc_set(tz_local_to_utc(&tz, local, NULL), true);
}
static void commit(int kind) {
    hold_refresh_until = us() + 500000;
    if (S.scr == SCR_TMR) { if (S.tgt_one >= 0) alarm_post(CMD_SET_ONCE, tgt_day, S.tgt_one); else alarm_post(CMD_CLEAR_ONCE, tgt_day, 0); }
    else if (S.scr == SCR_DAY) alarm_post(CMD_SET_DAY, S.sel, S.days[S.sel].min);
    else if (S.scr == SCR_SOUND) store_set_i64("svol", S.vol);
    else if (S.scr == SCR_LIGHT) { store_set_i64("ltl0", S.lt.l[0]); store_set_i64("ltl1", S.lt.l[1]); }
    else if (S.scr == SCR_SET) switch (S.pg) {
        case PG_TIME: if (kind != DRAG_SEC) set_clock_from_page(); break;
        case PG_DATE: S.clk = S.now; S.sec = 0; { int64_t t = time(NULL); S.sec = (int)(tz_utc_to_local(&tz, t) % 60); } set_clock_from_page(); break;
        case PG_REGION: {
            char z[40];
            if (S.rg < REG_COUNT - 1) snprintf(z, sizeof z, "%s", REG_TZ[S.rg]);
            else { int east = tz_offset_at_utc(&tz, time(NULL)), w = -east; snprintf(z, sizeof z, "MAN%s%d:%02d", w < 0 ? "-" : "", abs(w) / 3600, abs(w) % 3600 / 60); }   /* Manual: the clock stays on today's offset, no daylight saving */
            store_set_str("tz", z); store_set_i64("rg", S.rg); load_tz(); alarm_post(CMD_TZ_RELOAD, 0, 0); break; }
        case PG_BEDTIME: store_set_i64("ns", S.ns); break;
        case PG_BRIGHT: store_set_i64("dayb", S.dayB); break;
        case PG_ALARM: g_alarm_vol = S.ld; store_set_i64("avol", S.ld); break;
        default: break;
    }
}

/* ---- dots ---- */
typedef struct { int t; float a, r, unit; } knob_t;
typedef struct { int lo, hi; int *v; } lin_t;
static bool lin_info(lin_t *l) {
    if (S.scr == SCR_SOUND && !S.pick) { *l = (lin_t){0, 100, &S.vol}; return true; }
    if (S.scr == SCR_LIGHT) { *l = (lin_t){5, 100, &S.lt.l[S.lt.m]}; return true; }
    if (S.scr == SCR_SET && S.pg == PG_BRIGHT) { *l = (lin_t){10, 100, &S.dayB}; return true; }
    if (S.scr == SCR_SET && S.pg == PG_ALARM) { *l = (lin_t){30, 100, &S.ld}; return true; }
    return false;
}
static int knobs(knob_t *k) {
    int m = -1; lin_t l;
    if (S.scr == SCR_SOUND && S.pick) return 0;
    if (S.scr == SCR_SET && S.pg == PG_TIME && S.sec_mode) { k[0] = (knob_t){DRAG_SEC, S.sec * 6.0f, RR, 6}; return 1; }
    if (S.scr == SCR_TMR) m = S.tgt_one >= 0 ? S.tgt_one : S.tgt_usual >= 0 ? S.tgt_usual : 420;
    else if (S.scr == SCR_DAY) m = S.days[S.sel].min;
    else if (S.scr == SCR_SET && S.pg == PG_TIME) m = S.clk;
    if (m >= 0) { m = wrap(m); k[0] = (knob_t){DRAG_MIN, (m % 60) * 6.0f, RR, 30}; k[1] = (knob_t){DRAG_HR, (m / 60 % 12) * 30.0f + (m % 60) * .5f, RH, 30}; return 2; }
    if (S.scr == SCR_SET && S.pg == PG_BEDTIME) { int q = wrap(S.ns); k[0] = (knob_t){DRAG_BED, (q / 60 % 12) * 30.0f + (q % 60) * .5f, RR, 7.5f}; return 1; }
    if (lin_info(&l)) { k[0] = (knob_t){DRAG_LIN, -135 + 270.0f * (*l.v - l.lo) / (l.hi - l.lo), RR, 30}; return 1; }
    if (S.scr == SCR_SET && S.pg == PG_DATE) { int n = S.df == 1 ? 31 : 12, v = S.df == 0 ? S.dat.m : S.df == 1 ? S.dat.d - 1 : S.dat.y; k[0] = (knob_t){DRAG_STEP, (v % n) * 360.0f / n, RR, 360.0f / n}; return 1; }
    if (S.scr == SCR_SET && S.pg == PG_REGION) { k[0] = (knob_t){DRAG_STEP, S.rg * 360.0f / REG_COUNT, RR, 360.0f / REG_COUNT}; return 1; }
    return 0;
}
static void adj_time(int dm) {
    if (S.scr == SCR_TMR) { int c = S.tgt_one >= 0 ? S.tgt_one : S.tgt_usual >= 0 ? S.tgt_usual : 420, v = wrap(c + dm); S.tgt_one = v == S.tgt_usual ? -1 : v; if (S.tgt_skip) { S.tgt_skip = false; alarm_post(CMD_UNSKIP, 0, 0); } }
    else if (S.scr == SCR_DAY) { S.days[S.sel].min = (uint16_t)wrap(S.days[S.sel].min + dm); S.days[S.sel].on = true; }
    else if (S.scr == SCR_SET && S.pg == PG_TIME) S.clk = wrap(S.clk + dm);
}
static void step_val(int dir) {
    if (S.pg == PG_DATE) { if (S.df == 0) S.dat.m = (S.dat.m + (dir > 0 ? 1 : 11)) % 12; else if (S.df == 1) S.dat.d = ((S.dat.d - 1 + (dir > 0 ? 1 : 30)) % 31) + 1; else { S.dat.y += dir; if (S.dat.y < 2026) S.dat.y = 2026; if (S.dat.y > 2099) S.dat.y = 2099; } }
    else if (S.pg == PG_REGION) S.rg = (S.rg + (dir > 0 ? 1 : REG_COUNT - 1)) % REG_COUNT;
}

/* ---- lamp and sound outputs (read by light_fx and audio_out) ---- */
static int64_t lamp_on_us;
static void lamp_apply(void) {
    float w; int p = phase_p(&w); rgbw_f c = {0, 0, 0, 0};
    if (S.lt.on) {
        float l = S.lt.l[S.lt.m] / 100.0f;
        if (S.lt.m == 0) c.r = l * 0.15f;                                               /* night light: red only, 20 % on the dial is about 3 % (LIGHT.md section 4) */
        else if (p == 2) { c.r = l * 0.08f; c.g = l * 0.02f; }                          /* reading light at night: red and amber, capped at 8 % */
        else { float cap = 1 - 0.8f * w; c.w = l * cap * (1 - 0.5f * w); c.r = l * cap * (0.15f + 0.45f * w); c.g = l * cap * (0.05f + 0.12f * w); }   /* warmer and lower through the wind-down */
        if (p == 2 && us() - lamp_on_us > 20LL * 60 * 1000000) S.lt.on = false;       /* at night it switches itself off after 20 minutes */
    }
    g_lamp = c;
}

/* ---- navigation ---- */
static void nav(int scr) { S.scr = scr; S.pick = 0; S.fo.menu = false; S.fo.pick = 0; S.sec_mode = false; S.test = false; S.hold = 0; S.drag = 0; }
static void wake(void) { aw_from = S.aw; aw_to = 1; aw_t0 = us(); aw_anim = true; awake_until = us() + 4200000; }
static void focus_reset(void) { S.fo.run = false; S.fo.ph = 0; S.fo.rd = 1; S.fo.left = S.fo.len * 60; }

static void tap(float x, float y) {
    float r = hypotf(x - C, y - C), bx, by;
    switch (S.scr) {
    case SCR_CLOCK: wake(); break;
    case SCR_WEEK: if (r > 98) { float a = atan2f(x - C, C - y) * 57.29578f; a = fmodf(a + 360 + 180.0f / 7, 360); S.sel = (int)(a / (360.0f / 7)); nav(SCR_DAY); } break;
    case SCR_RING: if (!S.done && S.snz_left > 0) alarm_post(CMD_SNOOZE, 0, 0); break;
    case SCR_SOUND:
        if (S.pick == PICK_SND) { for (int i = 0; i < 8; i++) { gfx_pt(i * 45, 150, &bx, &by); if (hypotf(x - bx, y - by) <= 56) { S.snd = S.pp * 8 + i; S.play = true; store_set_i64("snd", S.snd); } } S.pick = 0; }
        else if (S.pick == PICK_TMR) { for (int i = 0; i < 5; i++) { gfx_pt(i * 72, 150, &bx, &by); if (hypotf(x - bx, y - by) <= 58) { S.ti = i; store_set_i64("sti", S.ti); } } S.pick = 0; }
        else if (y < 172) { S.pick = PICK_SND; S.pp = S.snd >> 3; }
        else if (y < 258) S.play = !S.play;
        else S.pick = PICK_TMR;
        break;
    case SCR_FOCUS:
        if (S.fo.menu && S.fo.pick) {
            static const int OPT[3][5] = {{15, 25, 35, 50, 60}, {3, 5, 10, 15, 20}, {2, 3, 4, 5, 6}}; int k = S.fo.pick - 1;
            for (int i = 0; i < 5; i++) { gfx_pt(i * 72, 150, &bx, &by); if (hypotf(x - bx, y - by) <= 58) { int v = OPT[k][i]; if (k == 0) S.fo.len = v; else if (k == 1) S.fo.brk = v; else S.fo.rds = v; focus_reset(); store_set_i64("folen", S.fo.len); store_set_i64("fobrk", S.fo.brk); store_set_i64("fords", S.fo.rds); } }
            S.fo.pick = 0;
        } else if (S.fo.menu) { int i = (int)lroundf((y - 150) / 83); if (i >= 0 && i <= 2 && fabsf(y - (150 + i * 83)) <= 40 && fabsf(x - C) <= 135) S.fo.pick = i + 1; else S.fo.menu = false; }
        else if (y < 268) { S.fo.run = false; S.fo.menu = true; S.fo.pick = 0; }
        else S.fo.run = !S.fo.run;
        break;
    case SCR_LIGHT:
        if (y < 250) S.lt.on = !S.lt.on; else { S.lt.m ^= 1; S.lt.on = true; store_set_i64("ltm", S.lt.m); }
        if (S.lt.on) lamp_on_us = us();
        break;
    case SCR_SET:
        if (S.pg == PG_DATE) S.df = y < 200 ? 0 : y < 290 ? 1 : 2;
        else if (S.pg == PG_TIME) { if (S.sec_mode) { S.sec_mode = false; set_clock_from_page(); } else if (y > 290) S.sec_mode = true; }
        else if (S.pg == PG_ALARM && r < 125) { S.test = !S.test; g_alarm_vol = S.ld; g_test_until_us = S.test ? us() + 6000000 : 0; }
        break;
    default: break;
    }
}
static void swipe(int dx, int dy) {            /* one of dx, dy is -1 or +1: the direction the finger moved */
    bool left = dx < 0, right = dx > 0, up = dy < 0, down = dy > 0;
    switch (S.scr) {
    case SCR_CLOCK: if (up) nav(SCR_TMR); else if (down) nav(SCR_WEEK); else if (left) nav(SCR_SOUND); else nav(SCR_LIGHT); break;
    case SCR_TMR: if (down) nav(SCR_CLOCK); break;
    case SCR_WEEK: if (up) nav(SCR_CLOCK); break;
    case SCR_DAY:
        if (up) nav(SCR_WEEK);
        else if (left || right) { int t = S.days[S.sel].min; S.sel = (S.sel + (left ? 1 : 6)) % 7; if (!S.days[S.sel].on) S.days[S.sel].min = (uint16_t)t; }
        break;
    case SCR_SOUND:
        if (S.pick) { if (S.pick == PICK_SND && (left || right)) S.pp ^= 1; else S.pick = 0; }
        else if (left) nav(SCR_FOCUS); else if (right) nav(SCR_CLOCK);
        break;
    case SCR_FOCUS: if (S.fo.menu) { S.fo.menu = false; S.fo.pick = 0; } else if (right) nav(SCR_SOUND); break;
    case SCR_LIGHT: if (left) nav(SCR_CLOCK); break;
    case SCR_SET: if (down) nav(SCR_CLOCK); else if (left || right) { S.pg = (S.pg + (left ? 1 : PG_COUNT - 1)) % PG_COUNT; S.sec_mode = false; S.test = false; } break;
    default: break;
    }
}
static void on_hold(void) {
    S.hold = 0;
    switch (S.scr) {
    case SCR_RING: alarm_post(CMD_DISMISS, 0, 0); S.done = true; done_until = us() + 1900000; break;
    case SCR_FOCUS: focus_reset(); S.fo.menu = false; break;
    case SCR_TMR: S.tgt_skip = !S.tgt_skip; hold_refresh_until = us() + 500000; if (S.tgt_skip) alarm_post(CMD_SKIP_DAY, tgt_day, 0); else alarm_post(CMD_UNSKIP, 0, 0); break;
    case SCR_DAY: S.days[S.sel].on = !S.days[S.sel].on; hold_refresh_until = us() + 500000; alarm_post(CMD_SET_DAY, S.sel, S.days[S.sel].on ? S.days[S.sel].min : -1); break;
    case SCR_CLOCK:
        if (S.eng_state == SM_SUNRISE || S.eng_state == SM_SNOOZED || S.eng_state == SM_MISSED) alarm_post(CMD_DISMISS, 0, 0);     /* stop it for today */
        else { nav(SCR_SET); S.pg = PG_TIME; S.clk = S.now; }
        break;
    default: break;
    }
}

/* ---- touch: the rules of UX.md section 3, measured in the concept page ---- */
static struct { bool down, moved, held, grab, dial, dialed; float x, y, ang, acc, unit; int64_t t; int kt, applied, v0, hold_ms; } g;
static void dial_move(void) {
    if (g.kt == DRAG_LIN) { lin_t l; if (!lin_info(&l)) return; int v = (int)lroundf((g.v0 + g.acc * (l.hi - l.lo) / 270) / 5) * 5; v = v < l.lo ? l.lo : v > l.hi ? l.hi : v; if (v != *l.v) { *l.v = v; if (S.scr == SCR_LIGHT) { S.lt.on = true; lamp_on_us = us(); } } return; }
    float unit = g.kt == DRAG_BED ? 7.5f : g.kt == DRAG_STEP ? g.unit : g.kt == DRAG_SEC ? 6 : 30; int st = (int)lroundf(g.acc / unit);
    if (st == g.applied) return;
    int n = st - g.applied; g.applied = st;
    if (g.kt == DRAG_BED) { S.ns = wrap(S.ns + n * 15); int wk = wake_min(), d = wrap(wk - S.ns); if (d < 240) S.ns = wrap(wk - 240); else if (d > 720) S.ns = wrap(wk - 720); }
    else if (g.kt == DRAG_SEC) S.sec = ((S.sec + n) % 60 + 60) % 60;
    else if (g.kt == DRAG_MIN) adj_time(n * 5);
    else if (g.kt == DRAG_HR) adj_time(n * 60);
    else for (int i = 0; i < abs(n); i++) step_val(n > 0 ? 1 : -1);
}
static void on_touch(lv_event_t *e) {
    lv_event_code_t code = lv_event_get_code(e); lv_point_t p; lv_indev_get_point(lv_indev_active(), &p); float x = p.x, y = p.y; int64_t now = us();
    if (code == LV_EVENT_PRESSED) {
        if (hypotf(x - C, y - C) > C) return;
        last_touch_us = now; memset(&g, 0, sizeof g); g.down = true; g.x = x; g.y = y; g.t = now;
        knob_t ks[2]; int nk = knobs(ks); float bd = 57;
        for (int i = 0; i < nk; i++) { float kx, ky; gfx_pt(ks[i].a, ks[i].r, &kx, &ky); float d = hypotf(x - kx, y - ky); if (d < bd) { bd = d; g.grab = true; g.kt = ks[i].t; g.unit = ks[i].unit; } }
        if (g.grab && g.kt == DRAG_LIN) { lin_t l; lin_info(&l); g.v0 = *l.v; }
        float r = hypotf(x - C, y - C);
        if (S.scr == SCR_RING && !S.done) g.hold_ms = 2000;
        else if (S.scr == SCR_CLOCK) g.hold_ms = 1500;
        else if (S.scr == SCR_FOCUS && !S.fo.menu && (S.fo.run || S.fo.ph || S.fo.rd > 1 || S.fo.left != S.fo.len * 60)) g.hold_ms = 1000;
        else if (((S.scr == SCR_TMR && (S.tgt_one >= 0 || S.tgt_usual >= 0)) || S.scr == SCR_DAY) && !g.grab && r < 150) g.hold_ms = 1000;
    } else if (code == LV_EVENT_PRESSING && g.down) {
        if (!g.moved && hypotf(x - g.x, y - g.y) > 16) {
            g.moved = true; if (S.hold > 0) { S.hold = 0; redraw(); }
            if (g.grab && now - g.t < 90000) g.grab = false;                              /* a flick, not a grab */
            if (g.grab) {
                float r0 = hypotf(g.x - C, g.y - C); if (r0 < 1) r0 = 1;
                float ux = (g.x - C) / r0, uy = (g.y - C) / r0, mx = x - g.x, my = y - g.y, tang = fabsf(-uy * mx + ux * my), rad = fabsf(ux * mx + uy * my);
                if (tang > rad) { g.dial = true; S.drag = g.kt; g.ang = atan2f(g.x - C, C - g.y) * 57.29578f; g.acc = 0; } else g.grab = false;   /* toward or away from the centre: a swipe */
            }
        }
        if (g.dial) { float a = atan2f(x - C, C - y) * 57.29578f, d = a - g.ang; if (d > 180) d -= 360; if (d < -180) d += 360; g.ang = a; g.acc += d; g.dialed = true; dial_move(); }
        if (!g.moved && !g.held && g.hold_ms) {
            int64_t el = (now - g.t) / 1000;
            if (el > 450) S.hold = fminf(1, (float)(el - 450) / (g.hold_ms - 450));       /* the indicator waits 0.45 s (D-042) */
            if (el >= g.hold_ms) { g.held = true; on_hold(); }
        }
    } else if ((code == LV_EVENT_RELEASED || code == LV_EVENT_PRESS_LOST) && g.down) {
        g.down = false; last_touch_us = now; int kind = S.drag; S.drag = 0;
        if (S.hold > 0) S.hold = 0;
        if (g.dialed) commit(kind);
        else if (!g.held && code == LV_EVENT_RELEASED) {
            float dx = x - g.x, dy = y - g.y, d = hypotf(dx, dy);
            if (d >= 56) { if (fabsf(dx) > fabsf(dy)) swipe(dx > 0 ? 1 : -1, 0); else swipe(0, dy > 0 ? 1 : -1); }
            else if (d < 16 && now - g.t < 400000) tap(x, y);
        }
    }
}

/* ---- drawing and the 50 ms heartbeat ---- */
static void on_draw(lv_event_t *e) {
    int ox = 0, oy = 0; if (S.scr == SCR_CLOCK) { ox = (S.now * 7) % 5 - 2; oy = (S.now * 3) % 5 - 2; }      /* the face moves a step a minute (D-016) */
    gfx_begin(lv_event_get_layer(e), ox, oy); gfx_check_reset(); ui_draw(&S);
}
static void focus_tick(void) {
    static int64_t last; int64_t n = us(); if (!S.fo.run) { last = n; return; }
    if (n - last < 1000000) return;
    last += 1000000;
    if (--S.fo.left > 0) return;
    if (S.fo.ph == 0) { if (S.fo.rd >= S.fo.rds) { focus_reset(); g_chime = 3; } else { S.fo.ph = 1; S.fo.left = S.fo.brk * 60; g_chime = 2; } }
    else { S.fo.ph = 0; S.fo.rd++; S.fo.left = S.fo.len * 60; g_chime = 3; }
}
static void tick(lv_timer_t *t) {
    (void)t; int64_t n = us(); static ui_t drawn;
    refresh(); focus_tick();
    if (aw_anim) { float d = aw_to > aw_from ? 320000 : 600000, p = (n - aw_t0) / d; if (p >= 1) { p = 1; aw_anim = false; } S.aw = aw_from + (aw_to - aw_from) * p * p * (3 - 2 * p); }
    else if (S.aw > 0.99f && n > awake_until) { aw_from = S.aw; aw_to = 0; aw_t0 = n; aw_anim = true; }
    if (S.scr != SCR_CLOCK) { S.aw = 0; aw_anim = false; }
    if (S.test && n > g_test_until_us) S.test = false;
    int limit = S.scr == SCR_SET ? 30 : 20;                                                 /* back to the clock by itself (R35) */
    if (!bench_frozen && S.scr != SCR_CLOCK && S.scr != SCR_RING && !g.down && !(S.scr == SCR_FOCUS && S.fo.run) && n - last_touch_us > limit * 1000000LL) nav(SCR_CLOCK);
    if (g_snd_ended) { g_snd_ended = false; S.play = false; }
    lamp_apply(); g_snd_play = S.play; g_snd_id = S.snd; g_snd_vol = S.vol; g_snd_timer_min = TIMER_N[S.ti];
    brightness();
    if (memcmp(&S, &drawn, sizeof S)) { drawn = S; redraw(); }
}
void ui_start(void) {
    load_tz(); ui_fresh(&S);
    S.vol = (int)store_get_i64("svol", 40); S.snd = (int)store_get_i64("snd", 0) % SND_COUNT; S.ti = (int)store_get_i64("sti", 1) % 5;
    S.lt.m = (int)store_get_i64("ltm", 0) & 1; S.lt.l[0] = (int)store_get_i64("ltl0", 20); S.lt.l[1] = (int)store_get_i64("ltl1", 60);
    S.rg = (int)store_get_i64("rg", 0) % REG_COUNT; S.ns = wrap((int)store_get_i64("ns", 1350)); S.dayB = (int)store_get_i64("dayb", 60); S.ld = g_alarm_vol;
    S.fo.len = (int)store_get_i64("folen", 25); S.fo.brk = (int)store_get_i64("fobrk", 5); S.fo.rds = (int)store_get_i64("fords", 4); focus_reset();
    last_touch_us = us();
    bsp_display_lock(-1);
    lv_obj_t *scr = lv_screen_active(); lv_obj_set_style_bg_color(scr, lv_color_black(), 0); lv_obj_remove_flag(scr, LV_OBJ_FLAG_SCROLLABLE);
    canvas = lv_obj_create(scr); lv_obj_remove_style_all(canvas); lv_obj_set_size(canvas, 466, 466); lv_obj_set_pos(canvas, 0, 0);
    lv_obj_add_flag(canvas, LV_OBJ_FLAG_CLICKABLE); lv_obj_remove_flag(canvas, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_event_cb(canvas, on_draw, LV_EVENT_DRAW_MAIN, NULL); lv_obj_add_event_cb(canvas, on_touch, LV_EVENT_ALL, NULL);
    lv_timer_create(tick, 50, NULL);
    bsp_display_unlock();
    ESP_LOGI(TAG, "interface up");
}

/* ---- bench: show any state, check the layout rules on the device ---- */
static int kv(const char *args, const char *key, int def) { char pat[24]; snprintf(pat, sizeof pat, "%s=", key); const char *p = strstr(args, pat); return p ? atoi(p + strlen(pat)) : def; }
void ui_bench_goto(const char *a) {
    static const char *N[SCR_COUNT] = {"clock", "tmr", "week", "day", "sound", "focus", "light", "ring", "set"};
    bsp_display_lock(-1);
    if (strstr(a, "live")) { bench_frozen = false; nav(SCR_CLOCK); bsp_display_unlock(); return; }
    bench_frozen = true; ui_t s; ui_fresh(&s);
    for (int i = 0; i < SCR_COUNT; i++) if (strstr(a, N[i])) s.scr = i;
    s.now = kv(a, "now", 870); s.pg = kv(a, "pg", 0); s.sel = kv(a, "sel", 2); s.pick = kv(a, "pick", 0); s.pp = kv(a, "pp", 0); s.snd = kv(a, "snd", 0); s.play = kv(a, "play", 0);
    s.aw = kv(a, "aw", 0) / 100.0f; s.hold = kv(a, "hold", 0) / 100.0f; s.eng_state = kv(a, "eng", 0); s.dawn_t = kv(a, "dawn", 0); s.fo.menu = kv(a, "menu", 0); s.fo.pick = kv(a, "fpick", 0);
    s.fo.run = kv(a, "run", 0); s.fo.left = kv(a, "left", 1500); s.lt.on = kv(a, "on", 0); s.lt.m = kv(a, "m", 0); s.sec_mode = kv(a, "secmode", 0); s.df = kv(a, "df", 0); s.rg = kv(a, "rg", 0); s.test = kv(a, "test", 0);
    s.done = kv(a, "done", 0); s.tgt_skip = kv(a, "skip", 0); s.tgt_one = kv(a, "one", -1); s.bat.present = kv(a, "bat", 0); s.bat.pct = 82; s.bat.mv = 4020; s.drag = kv(a, "drag", 0);
    if (kv(a, "alarms", 1)) { for (int i = 0; i < 5; i++) { s.days[i].on = true; s.days[i].min = 405; } s.has_next = true; s.next_wd = 4; s.next_min = 405; s.tgt_usual = 405; }
    if (strstr(a, "status=")) { const char *p = strstr(a, "status=") + 7; snprintf(s.status, sizeof s.status, "%s", p); for (char *c = s.status; *c; c++) if (*c == '_') *c = ' '; s.status_warn = true; }
    S = s; redraw(); bsp_display_unlock();
}
static int check_one(ui_t *s, const char *what, int *n) {
    char msg[120]; gfx_begin(NULL, 0, 0); gfx_check_reset(); ui_draw(s); (*n)++;
    bool limit = !(s->scr == SCR_WEEK || s->pick || s->fo.pick);
    int bad = gfx_check(msg, sizeof msg, limit); if (bad) printf("LAYOUT %s: %s\n", what, msg);
    return bad ? 1 : 0;
}
int ui_bench_layoutcheck(void) {
    int bad = 0, n = 0; ui_t s; char w[48];
    static const int NOW[6] = {600, 870, 1200, 1260, 1300, 1410}; static const int MINS[6] = {0, 425, 1255, 779, 60, 1439};
    for (int a = 0; a < 6; a++) for (int v = 0; v < 6; v++) {                               /* clock: every hour of the day, every status */
        ui_fresh(&s); s.now = NOW[a]; s.aw = v & 1;
        if (v >= 2) { s.has_next = true; s.next_min = MINS[v]; s.next_wd = 2; s.next_far = v == 3; for (int i = 0; i < 7; i++) { s.days[i].on = true; s.days[i].min = 405; } }
        if (v == 4) snprintf(s.status, sizeof s.status, "Alarms lost: set again");
        if (v == 5) snprintf(s.status, sizeof s.status, "Wed skipped");
        snprintf(w, sizeof w, "clock now=%d v=%d", NOW[a], v); bad += check_one(&s, w, &n);
    }
    for (int d = -1800; d <= 0; d += 300) { ui_fresh(&s); s.eng_state = 1; s.dawn_t = d; s.has_next = true; s.next_min = 405; bad += check_one(&s, "clock dawn", &n); }
    for (int a = 0; a < 6; a++) for (int m = 0; m < 6; m++) for (int v = 0; v < 4; v++) {     /* the pickers: every awkward time, on, off, skipped, once only */
        ui_fresh(&s); s.now = NOW[a]; s.scr = SCR_TMR; s.tgt_usual = v == 3 ? -1 : MINS[m]; s.tgt_one = v == 1 ? MINS[(m + 1) % 6] : -1; s.tgt_skip = v == 2; s.tgt_today = m & 1;
        snprintf(w, sizeof w, "tmr m=%d v=%d", MINS[m], v); bad += check_one(&s, w, &n);
        ui_fresh(&s); s.now = NOW[a]; s.scr = SCR_DAY; s.sel = (m + v) % 7; s.days[s.sel].min = (uint16_t)MINS[m]; s.days[s.sel].on = v & 1; snprintf(w, sizeof w, "day %d m=%d", s.sel, MINS[m]); bad += check_one(&s, w, &n);
    }
    for (int a = 0; a < 6; a++) for (int v = 0; v < 4; v++) { ui_fresh(&s); s.now = NOW[a]; s.scr = SCR_WEEK; for (int i = 0; i < 7; i++) { s.days[i].on = (v >> (i % 2)) & 1; s.days[i].min = (uint16_t)MINS[(i + v) % 6]; } s.has_next = v > 0; s.next_wd = v; s.next_min = 1439; bad += check_one(&s, "week", &n); }
    for (int a = 0; a < 6; a += 5) for (int i = 0; i < SND_COUNT; i++) for (int v = 0; v < 4; v++) {
        ui_fresh(&s); s.now = NOW[a]; s.scr = SCR_SOUND; s.snd = i; s.pick = v == 1 ? PICK_SND : v == 2 ? PICK_TMR : 0; s.pp = i >> 3; s.ti = i % 5; s.vol = v == 3 ? 100 : 0; s.play = i & 1;
        snprintf(w, sizeof w, "sound %s v=%d", ui_sound_name(i), v); bad += check_one(&s, w, &n);
    }
    for (int a = 0; a < 6; a += 5) for (int v = 0; v < 9; v++) {
        ui_fresh(&s); s.now = NOW[a]; s.scr = SCR_FOCUS; s.fo.menu = v >= 5; s.fo.pick = v >= 6 ? v - 5 : 0; s.fo.run = v & 1; s.fo.ph = v == 2; s.fo.len = v == 3 ? 60 : 25; s.fo.rds = v == 4 ? 6 : 4; s.fo.left = v == 3 ? 3599 : 1;
        snprintf(w, sizeof w, "focus v=%d", v); bad += check_one(&s, w, &n);
        ui_fresh(&s); s.now = NOW[a]; s.scr = SCR_LIGHT; s.lt.on = v & 1; s.lt.m = (v >> 1) & 1; s.lt.l[s.lt.m] = v > 4 ? 100 : 5; bad += check_one(&s, "light", &n);
    }
    for (int v = 0; v < 3; v++) { ui_fresh(&s); s.scr = SCR_RING; s.now = MINS[v + 1]; s.snz_left = v; s.done = v == 2; s.hold = v ? .5f : 0; bad += check_one(&s, "ring", &n); }
    for (int a = 0; a < 6; a += 5) for (int pg = 0; pg < PG_COUNT; pg++) for (int v = 0; v < 9; v++) {
        ui_fresh(&s); s.now = NOW[a]; s.scr = SCR_SET; s.pg = pg; s.clk = MINS[v % 6]; s.sec = v * 7; s.sec_mode = v == 7; s.df = v % 3; s.dat.m = v + 2; s.dat.d = v == 1 ? 28 : 7; s.dat.y = v == 2 ? 2099 : 2026;
        s.rg = v % REG_COUNT; s.ns = MINS[v % 6] / 15 * 15; s.dayB = 10 + v * 11; s.ld = 30 + v * 8; s.test = v & 1; s.bat.present = v & 1; s.bat.pct = v == 1 ? 100 : 7; s.bat.mv = 4200;
        for (int i = 0; i < 7; i++) { s.days[i].on = true; s.days[i].min = (uint16_t)MINS[(v + 1) % 6]; }
        snprintf(w, sizeof w, "set pg=%d v=%d", pg, v); bad += check_one(&s, w, &n);
    }
    printf("LAYOUTCHECK states=%d bad=%d\n", n, bad); return bad;
}
