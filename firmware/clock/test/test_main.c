/* Host tests for the clock core (no hardware). Test IDs match docs/TEST_PLAN.md. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "civil.h"
#include "tz.h"
#include "sched.h"
#include "alarm_sm.h"
#include "dawn.h"
#include "storage.h"
#include "expected_tz.inc"

static int fails, checks;
#define CHECK(c) do { checks++; if (!(c)) { fails++; printf("  FAIL %s:%d  %s\n", __FILE__, __LINE__, #c); } } while (0)
static uint64_t rs = 88172645463325252ull;
static uint32_t rnd(void) { rs ^= rs << 13; rs ^= rs >> 7; rs ^= rs << 17; return (uint32_t)(rs >> 11); }
static void head(const char *id, const char *t) { printf("%s  %s\n", id, t); }

static int64_t U(int y, int m, int d, int hh, int mm, int ss) { civil_t c = {y, m, d, hh, mm, ss}; return civil_to_secs(&c); }

/* ---- T-39: dates ---- */
static void t_dates(void) {
    head("T-39", "dates: round trip, leap days, 2038, weekdays");
    for (int64_t d = days_from_civil(1970, 1, 1); d <= days_from_civil(2100, 12, 31); d++) {
        int y, m, dd; civil_from_days(d, &y, &m, &dd);
        CHECK(days_from_civil(y, m, dd) == d); CHECK(dd >= 1 && dd <= days_in_month(y, m));
    }
    for (int y = 2024; y <= 2060; y++) CHECK(days_in_month(y, 2) == ((y % 4 == 0) ? 29 : 28));
    CHECK(is_leap(2000) && !is_leap(2100) && is_leap(2052));
    for (int y = 2028; y <= 2056; y += 4) { CHECK(days_from_civil(y, 3, 1) - days_from_civil(y, 2, 28) == 2); CHECK(days_from_civil(y + 1, 3, 1) - days_from_civil(y + 1, 2, 28) == 1); }
    CHECK(U(2038, 1, 19, 3, 14, 8) == 2147483648LL); CHECK(U(2038, 1, 19, 3, 14, 9) == 2147483649LL);
    CHECK(sizeof(int64_t) == 8);
    CHECK(weekday_from_days(days_from_civil(2026, 11, 12)) == 3);   /* Thursday */
    CHECK(weekday_from_days(days_from_civil(2000, 1, 1)) == 5);     /* Saturday */
    CHECK(weekday_from_days(days_from_civil(2099, 12, 31)) == 3);   /* Thursday */
    civil_t c; secs_to_civil(-1, &c); CHECK(c.y == 1969 && c.m == 12 && c.d == 31 && c.hh == 23 && c.ss == 59);
}

/* ---- T-28: daylight saving, every transition 2026..2099 against an independent library ---- */
static const char *TZS[10] = {0};
typedef struct { int64_t utc; int32_t off; } exp_t;
static void t_dst(void) {
    head("T-28", "daylight saving: every transition 2026-2099, 10 regions, vs Python zoneinfo");
    for (int i = 0; i < 9; i++) TZS[i] = TZ_REGION_STR[i];
    TZS[9] = "AEST-10AEDT,M10.1.0/2,M4.1.0/3";
    const exp_t *E[10] = {(exp_t *)EXP_0, (exp_t *)EXP_1, (exp_t *)EXP_2, (exp_t *)EXP_3, (exp_t *)EXP_4, (exp_t *)EXP_5, (exp_t *)EXP_6, (exp_t *)EXP_7, (exp_t *)EXP_8, (exp_t *)EXP_9};
    int64_t total = 0;
    for (int i = 0; i < 10; i++) {
        tz_t tz; CHECK(tz_parse(TZS[i], &tz));
        int64_t t0 = U(2026, 1, 1, 0, 0, 0), t1 = U(2100, 1, 1, 0, 0, 0); int k = 0; int32_t prev = tz_offset_at_utc(&tz, t0 - 3600);
        for (int64_t t = t0; t < t1; t += 3600) {
            int32_t o = tz_offset_at_utc(&tz, t);
            if (o != prev) { CHECK(E[i][k].utc != 0 && E[i][k].utc == t && E[i][k].off == o); k++; total++; prev = o; }
        }
        CHECK(E[i][k].utc == 0);          /* no transition missing */
        /* the exact minute: one second before is the old offset, the instant itself the new one */
        for (int j = 0; E[i][j].utc; j++) { CHECK(tz_offset_at_utc(&tz, E[i][j].utc) == E[i][j].off); CHECK(tz_offset_at_utc(&tz, E[i][j].utc - 1) != E[i][j].off); }
    }
    printf("  %lld transitions compared\n", (long long)total);
}
static void t_gap(void) {
    head("T-28b", "skipped and repeated hours");
    tz_t tz; tz_parse(TZ_REGION_STR[0], &tz); local_kind_t k;
    int64_t u = tz_local_to_utc(&tz, U(2026, 3, 8, 2, 30, 0), &k);   /* does not exist */
    CHECK(k == LOCAL_GAP); CHECK(u == U(2026, 3, 8, 7, 0, 0));        /* rings at 3:00 EDT */
    u = tz_local_to_utc(&tz, U(2026, 11, 1, 1, 30, 0), &k);          /* happens twice */
    CHECK(k == LOCAL_AMBIGUOUS); CHECK(u == U(2026, 11, 1, 5, 30, 0)); /* first one, EDT */
    u = tz_local_to_utc(&tz, U(2026, 7, 4, 12, 0, 0), &k); CHECK(k == LOCAL_OK && u == U(2026, 7, 4, 16, 0, 0));
    CHECK(tz_utc_to_local(&tz, U(2026, 11, 1, 6, 30, 0)) == U(2026, 11, 1, 1, 30, 0));   /* second 1:30 */
    tz_parse(TZ_REGION_STR[7], &tz);
    u = tz_local_to_utc(&tz, U(2026, 3, 29, 2, 30, 0), &k); CHECK(k == LOCAL_GAP && u == U(2026, 3, 29, 1, 0, 0));
    tz_t bad; CHECK(!tz_parse("", &bad)); CHECK(!tz_parse("EST5EDT,M13.1.0,M11.1.0", &bad)); CHECK(!tz_parse("EST", &bad)); CHECK(tz_parse("<+0330>-3:30", &bad) && bad.std_east == 12600);
    char junk[40]; for (int i = 0; i < 20000; i++) { int n = rnd() % 38; for (int j = 0; j < n; j++) junk[j] = "ESTD0123456789,M.+-/<> :"[rnd() % 24]; junk[n] = 0; tz_t t; (void)tz_parse(junk, &t); }
    CHECK(1);   /* parser never crashes on junk (e.g. a bad region.txt) */
}

/* ---- T-01, T-03, T-04: schedule ---- */
static int64_t local_utc(const tz_t *tz, int y, int m, int d, int hh, int mm) { return tz_local_to_utc(tz, U(y, m, d, hh, mm, 0), NULL); }
static void t_sched(void) {
    head("T-01/03/04", "weekly schedule, skip, tomorrow-only, next alarm");
    tz_t tz; tz_parse(TZ_REGION_STR[0], &tz); sched_t s; sched_init(&s);
    int64_t now = local_utc(&tz, 2026, 11, 12, 8, 0);                 /* Thursday */
    CHECK(!next_alarm(&s, &tz, now).valid);                           /* a new clock has no alarms (D-015) */
    for (int i = 0; i < 5; i++) { s.day[i].on = true; s.day[i].min = 405; }   /* Mon-Fri 6:45 */
    occurrence_t o = next_alarm(&s, &tz, now); CHECK(o.valid && o.utc == local_utc(&tz, 2026, 11, 13, 6, 45));   /* Friday */
    o = next_alarm(&s, &tz, local_utc(&tz, 2026, 11, 13, 6, 45)); CHECK(o.utc == local_utc(&tz, 2026, 11, 16, 6, 45));  /* strictly after: Monday */
    o = next_alarm(&s, &tz, local_utc(&tz, 2026, 11, 13, 6, 44)); CHECK(o.utc == local_utc(&tz, 2026, 11, 13, 6, 45));
    sched_skip_next(&s, &tz, now); o = next_alarm(&s, &tz, now); CHECK(o.utc == local_utc(&tz, 2026, 11, 16, 6, 45));    /* Friday skipped */
    sched_unskip(&s); o = next_alarm(&s, &tz, now); CHECK(o.utc == local_utc(&tz, 2026, 11, 13, 6, 45));
    sched_set_tomorrow(&s, &tz, now, 9 * 60); o = next_alarm(&s, &tz, now); CHECK(o.utc == local_utc(&tz, 2026, 11, 13, 9, 0) && o.from_override);
    sched_skip_next(&s, &tz, now); o = next_alarm(&s, &tz, now); CHECK(o.utc == local_utc(&tz, 2026, 11, 16, 6, 45));    /* skip the override */
    sched_expire(&s, local_day_of(&tz, now) + 2); CHECK(s.one_day == NO_DAY && s.skip_day == NO_DAY);
    sched_init(&s); sched_set_tomorrow(&s, &tz, now, 600);            /* one-off on a day with no alarm */
    o = next_alarm(&s, &tz, now); CHECK(o.valid && o.utc == local_utc(&tz, 2026, 11, 13, 10, 0));
    /* each weekday is independent (R1) */
    sched_init(&s); for (int i = 0; i < 7; i++) { s.day[i].on = (i % 2 == 0); s.day[i].min = (uint16_t)(60 + i * 10); }
    int64_t t = local_utc(&tz, 2027, 1, 3, 12, 0);                    /* Sunday noon */
    for (int i = 0; i < 14; i++) { o = next_alarm(&s, &tz, t); int wd = weekday_from_days(o.local_day); CHECK(s.day[wd].on && o.min == s.day[wd].min); t = o.utc; }
    /* every minute of the day can be set (1-minute resolution) */
    sched_init(&s); s.day[0].on = true; for (int m = 0; m < 1440; m++) { s.day[0].min = (uint16_t)m; o = next_alarm(&s, &tz, local_utc(&tz, 2027, 1, 3, 12, 0)); CHECK(o.valid && o.min == m && weekday_from_days(o.local_day) == 0); }
    /* DST nights: ring once, 1:30 in the repeated hour and 2:30 in the skipped hour */
    sched_init(&s); for (int i = 0; i < 7; i++) { s.day[i].on = true; s.day[i].min = 90; }
    int64_t first = U(2026, 11, 1, 5, 30, 0);                         /* 1:30 EDT */
    o = next_alarm(&s, &tz, first - 60); CHECK(o.utc == first && o.kind == LOCAL_AMBIGUOUS);
    o = next_alarm(&s, &tz, first); CHECK(o.utc == local_utc(&tz, 2026, 11, 2, 1, 30));          /* not the repeated 1:30 EST */
    o = next_alarm(&s, &tz, first + 3600); CHECK(o.utc == local_utc(&tz, 2026, 11, 2, 1, 30));
    for (int i = 0; i < 7; i++) s.day[i].min = 150;
    o = next_alarm(&s, &tz, U(2026, 3, 8, 6, 0, 0)); CHECK(o.utc == U(2026, 3, 8, 7, 0, 0) && o.kind == LOCAL_GAP);
    o = next_alarm(&s, &tz, U(2026, 3, 8, 7, 0, 0)); CHECK(o.utc == local_utc(&tz, 2026, 3, 9, 2, 30));
}
static void t_sched_prop(void) {
    head("T-04b", "next alarm vs brute-force minute scan, 1500 random schedules");
    tz_t tz; int viol = 0;
    for (int n = 0; n < 1500; n++) {
        tz_parse(TZ_REGION_STR[rnd() % 8], &tz); sched_t s; sched_init(&s);
        for (int i = 0; i < 7; i++) { s.day[i].on = rnd() % 3 != 0; s.day[i].min = (uint16_t)(rnd() % 1440); if (rnd() % 4 == 0) s.day[i].min = (uint16_t)(60 + rnd() % 240); }
        int64_t now = U(2026 + rnd() % 70, 1 + rnd() % 12, 1 + rnd() % 28, rnd() % 24, rnd() % 60, rnd() % 60);
        if (rnd() % 3 == 0) { s.skip_day = local_day_of(&tz, now) + (int)(rnd() % 3); }
        if (rnd() % 3 == 0) { s.one_day = local_day_of(&tz, now) + (int)(rnd() % 3); s.one_min = (uint16_t)(rnd() % 1440); }
        occurrence_t o = next_alarm(&s, &tz, now);
        bool any = s.one_day != NO_DAY; for (int i = 0; i < 7; i++) any |= s.day[i].on;
        if (!o.valid) { if (any) viol++; continue; }
        if (o.utc <= now) viol++;
        for (int64_t t = (now / 60 + 1) * 60; t < o.utc; t += 60) {
            int64_t loc = tz_utc_to_local(&tz, t); int32_t d = (int32_t)floor_div(loc, 86400); int mn = (int)((loc - (int64_t)d * 86400) / 60);
            bool has; int want; if (s.one_day == d) { has = true; want = s.one_min; } else { has = s.day[weekday_from_days(d)].on; want = s.day[weekday_from_days(d)].min; }
            if (has && s.skip_day != d && want == mn && tz_local_to_utc(&tz, loc, NULL) == t) { viol++; break; }
        }
        if (o.kind != LOCAL_GAP && tz_utc_to_local(&tz, o.utc) != (int64_t)o.local_day * 86400 + o.min * 60) viol++;
    }
    CHECK(viol == 0);
}

/* ---- alarm state machine ---- */
typedef struct { alarm_sm_t sm; sm_cfg_t c; } rig_t;
static sm_out_t tick(rig_t *r, int64_t now, int64_t next, sm_event_t ev) { return sm_step(&r->sm, &r->c, now, next, ev); }
static void t_sm(void) {
    head("T-22/23", "alarm state machine: dawn, escalation, snooze, dismiss, auto-stop");
    rig_t r; sm_init(&r.sm); sm_default_cfg(&r.c);
    int64_t T = 100000, nxt = T; sm_out_t o;
    for (int64_t t = T - 2400; t < T - 1800; t++) { o = tick(&r, t, nxt, EV_NONE); CHECK(o.state == SM_IDLE && !o.light_on); }
    o = tick(&r, T - 1800, nxt, EV_NONE); CHECK(o.state == SM_SUNRISE && o.light_on && o.dawn_t_s == -1800);
    o = tick(&r, T - 1, nxt, EV_NONE); CHECK(o.state == SM_SUNRISE && o.dawn_t_s == -1 && o.sound_stage == 0);
    o = tick(&r, T, nxt, EV_NONE); CHECK(o.state == SM_RINGING && o.sound_stage == 1);
    nxt = T + 86400;
    o = tick(&r, T + 59, nxt, 0); CHECK(o.sound_stage == 1); o = tick(&r, T + 60, nxt, 0); CHECK(o.sound_stage == 2);
    o = tick(&r, T + 120, nxt, 0); CHECK(o.sound_stage == 3); o = tick(&r, T + 180, nxt, 0); CHECK(o.sound_stage == 4); o = tick(&r, T + 900, nxt, 0); CHECK(o.sound_stage == 4);
    o = tick(&r, T + 1000, nxt, EV_SNOOZE); CHECK(o.state == SM_SNOOZED && o.sound_stage == 0 && o.light_on && o.snoozes_left == 2);
    o = tick(&r, T + 1539, nxt, 0); CHECK(o.state == SM_SNOOZED); o = tick(&r, T + 1540, nxt, 0); CHECK(o.state == SM_RINGING && o.sound_stage == 1);
    tick(&r, T + 1600, nxt, EV_SNOOZE); tick(&r, T + 2200, nxt, 0); o = tick(&r, T + 2200, nxt, EV_SNOOZE); CHECK(o.state == SM_SNOOZED && o.snoozes_left == 0);
    o = tick(&r, T + 2800, nxt, 0); CHECK(o.state == SM_RINGING);
    o = tick(&r, T + 2810, nxt, EV_SNOOZE); CHECK(o.state == SM_RINGING);                 /* no snoozes left: tap does nothing */
    o = tick(&r, T + 2900, nxt, EV_DISMISS); CHECK(o.ended && r.sm.handled == T && !o.light_on && o.sound_stage == 0);
    o = tick(&r, T + 2901, nxt, 0); CHECK(o.state == SM_IDLE && r.sm.armed == nxt);
    /* nobody answers: sound gives up after 30 min (D-015), light stays, then it is closed */
    sm_init(&r.sm); T = 500000; nxt = T;
    for (int64_t t = T - 1800; t <= T + 1799; t++) { o = tick(&r, t, nxt, 0); if (t >= T) nxt = T + 86400; }
    CHECK(o.state == SM_RINGING && o.sound_stage == 4);
    o = tick(&r, T + 1800, nxt, 0); CHECK(o.state == SM_MISSED && o.sound_stage == 0 && o.light_on && o.missed);
    o = tick(&r, T + 1800 + 3599, nxt, 0); CHECK(o.state == SM_MISSED);
    o = tick(&r, T + 1800 + 3600, nxt, 0); CHECK(o.ended && !o.light_on);
    /* dismiss early during the dawn ("I'm up"): it does not ring */
    sm_init(&r.sm); T = 900000; nxt = T; tick(&r, T - 1000, nxt, 0); o = tick(&r, T - 900, nxt, EV_DISMISS); CHECK(o.ended && o.state == SM_IDLE);
    nxt = T + 86400; for (int64_t t = T - 800; t < T + 100; t++) { o = tick(&r, t, nxt, 0); CHECK(o.state == SM_IDLE && o.sound_stage == 0); }
    /* alarm switched off or skipped during the dawn cancels it (hold) */
    sm_init(&r.sm); nxt = T; tick(&r, T - 1500, nxt, 0); o = tick(&r, T - 1400, T + 86400, 0); CHECK(o.ended && !o.light_on);
    /* no alarm set: stays idle for a month */
    sm_init(&r.sm); for (int64_t t = 0; t < 30 * 86400; t += 7) { o = tick(&r, t, 0, 0); CHECK(o.state == SM_IDLE && !o.light_on && o.sound_stage == 0); }
}
static void t_sm_reboot(void) {
    head("T-21/20", "reboot during dawn or ringing resumes right; missed alarm catches up");
    rig_t r; sm_cfg_t c; sm_default_cfg(&c); r.c = c; int64_t T = 777000; sm_out_t o;
    sm_init(&r.sm); tick(&r, T - 1000, T, 0); alarm_sm_t saved = r.sm;                       /* power cut mid-dawn */
    r.sm = saved; o = tick(&r, T - 400, T, 0); CHECK(o.state == SM_SUNRISE && o.dawn_t_s == -400);
    sm_init(&r.sm); tick(&r, T, T, 0); tick(&r, T + 200, T + 86400, 0); saved = r.sm;          /* power cut while ringing */
    r.sm = saved; o = tick(&r, T + 260, T + 86400, 0); CHECK(o.state == SM_RINGING && o.sound_stage == 4);
    r.sm = saved; o = tick(&r, T + 20, T + 86400, 0); CHECK(o.state == SM_RINGING && o.sound_stage == 1 + (int)((T + 20 - saved.ring_t0) / 60));
    /* off through the alarm time, back 10 minutes late: fires at once, soft */
    sm_init(&r.sm); tick(&r, T - 5000, T, 0); saved = r.sm; CHECK(saved.armed == T);
    r.sm = saved; o = tick(&r, T + 600, T + 86400, 0); CHECK(o.state == SM_RINGING && o.sound_stage == 1 && r.sm.ring_t0 == T + 600);
    r.sm = saved; o = tick(&r, T + 3600, T + 86400, 0); CHECK(o.state == SM_RINGING);          /* exactly at the window edge */
    r.sm = saved; o = tick(&r, T + 3601, T + 86400, 0); CHECK(o.state == SM_IDLE && r.sm.handled == T);   /* too late: not rung, next day armed */
    /* a snooze survives a reboot */
    sm_init(&r.sm); tick(&r, T, T, 0); tick(&r, T + 30, T + 86400, EV_SNOOZE); saved = r.sm;
    r.sm = saved; o = tick(&r, T + 400, T + 86400, 0); CHECK(o.state == SM_SNOOZED); o = tick(&r, T + 30 + 540, T + 86400, 0); CHECK(o.state == SM_RINGING);
    /* setting the clock by hand must not fire a "missed" alarm */
    sm_init(&r.sm); tick(&r, T - 5000, T, 0); o = tick(&r, T + 100, T + 86400, EV_TIME_SET); CHECK(o.state == SM_IDLE && r.sm.armed == T + 86400);
}
static int rings_in(const tz_t *tz, const sched_t *s, int64_t from, int64_t to) {
    rig_t r; sm_init(&r.sm); sm_default_cfg(&r.c); int rings = 0; bool was = false; sm_event_t ev = EV_NONE;
    for (int64_t t = from; t < to; t++) {
        occurrence_t o = next_alarm(s, tz, t); sm_out_t q = tick(&r, t, o.valid ? o.utc : 0, ev); ev = EV_NONE;
        if (q.state == SM_RINGING && !was) { rings++; ev = EV_DISMISS; }
        was = q.state == SM_RINGING;
    }
    return rings;
}
static void t_sm_dst_night(void) {
    head("T-21b", "one ring across the repeated hour, one across the skipped hour");
    tz_t tz; tz_parse(TZ_REGION_STR[0], &tz); sched_t s; sched_init(&s);
    for (int i = 0; i < 7; i++) { s.day[i].on = true; s.day[i].min = 90; }
    CHECK(rings_in(&tz, &s, U(2026, 11, 1, 0, 0, 0), U(2026, 11, 1, 12, 0, 0)) == 1);      /* 1:30 happens twice */
    for (int i = 0; i < 7; i++) s.day[i].min = 150;
    CHECK(rings_in(&tz, &s, U(2026, 3, 8, 3, 0, 0), U(2026, 3, 8, 15, 0, 0)) == 1);        /* 2:30 does not exist */
    for (int i = 0; i < 7; i++) s.day[i].min = 405;
    CHECK(rings_in(&tz, &s, U(2026, 11, 1, 0, 0, 0), U(2026, 11, 3, 0, 0, 0)) == 2);        /* normal days: one each */
}

/* ---- T-12, T-10, T-47: light and sound curves ---- */
static void t_dawn(void) {
    head("T-12/10", "dawn curve, current cap, pre-dawn sound fades");
    CHECK(dawn_F(-1800) == 0 && dawn_F(0) == 1 && dawn_F(600) == 1);
    CHECK(fabsf(dawn_F(-900) - 0.0447f) < 0.001f); CHECK(fabsf(dawn_F(-600) - 0.126f) < 0.002f); CHECK(fabsf(dawn_F(-300) - 0.355f) < 0.002f);
    float prev = 0, maxstep = 0, maxrel = 0; float peak = 0;
    for (int t = -1800; t <= 600; t++) {
        rgbw_f c = dawn_rgbw(t); float F = dawn_F(t); CHECK(F >= prev); if (t > -1800 && prev > 0.0005f) { float rel = (F - prev) / prev; if (rel > maxrel) maxrel = rel; }
        if (F - prev > maxstep) maxstep = F - prev; prev = F;
        float sum = c.r + c.g + c.b + c.w; float ma = sum * 18.0f * 24; if (ma > peak) peak = ma;
        CHECK(c.r >= 0 && c.g >= 0 && c.b >= 0 && c.w >= 0 && c.r <= 1 && c.w <= 1 && c.b <= .26f);
    }
    printf("  largest one-second rise: %.4f of full (%.2f %% per second at most)\n  peak current estimate %.0f mA\n", maxstep, maxrel * 100, peak);
    CHECK(maxstep < 0.01f); CHECK(maxrel < 0.0045f); CHECK(peak < 900.0f);                 /* under the 0.9 A cap (R14) */
    rgbw_f z = dawn_rgbw(0), a = dawn_rgbw(-1); CHECK(fabsf(z.w - a.w) < 0.01f && fabsf(z.r - a.r) < 0.01f);   /* no jump at the alarm time */
    z = dawn_rgbw(600); CHECK(z.w == 1 && fabsf(z.b - .25f) < 1e-4f && z.r < 1e-4f);
    /* pre-dawn sound */
    CHECK(presound_gain(-3600, 0) == 0 && presound_gain(-2000, 0) == 0);                    /* all night: no pre-window */
    for (int n = 15; n <= 90; n += 15) {
        int32_t start = -(30 + n) * 60; CHECK(presound_gain(start - 1, n) == 0); CHECK(presound_gain(start, n) == 0);
        CHECK(presound_gain(-1800 - 1, n) > 0.999f || n < 10); CHECK(presound_gain(-1800 + 180, n) < 0.001f); CHECK(presound_gain(-100, n) == 0 && presound_gain(0, n) == 0);
        float p = 0; int rises = 0, falls = 0;
        for (int t = start; t <= -1500; t++) { float g = presound_gain(t, n); CHECK(g >= 0 && g <= 1); if (g > p + 0.02f / 60 * 60) {} if (fabsf(g - p) > 0.01f && t > start) CHECK(0); if (g > p) rises++; if (g < p) falls++; p = g; }
        CHECK(rises > 0 && falls > 0);
        CHECK(fabsf(presound_gain(start + (n < 10 ? n : 10) * 60 / 2, n) - 0.5f) < 0.001f);   /* halfway through the fade-in */
    }
}

/* ---- T-17, T-15, T-16, T-44: storage and the clock chip ---- */
static void t_storage(void) {
    head("T-17/15/16", "CRC, A/B records, power cut at every byte, schedule format, RTC chip, drift");
    CHECK(crc32_calc("123456789", 9) == 0xCBF43926u);
    sched_t s; sched_init(&s); s.day[1].on = true; s.day[1].min = 405; s.skip_day = 20000; s.one_day = 20001; s.one_min = 555;
    uint8_t buf[SCHED_BYTES]; sched_serialize(&s, buf); sched_t u; { uint8_t again[SCHED_BYTES]; CHECK(sched_deserialize(&u, buf, SCHED_BYTES)); sched_serialize(&u, again); CHECK(!memcmp(again, buf, SCHED_BYTES) && u.day[1].min == 405 && u.skip_day == 20000 && u.one_min == 555); }
    CHECK(!sched_deserialize(&u, buf, SCHED_BYTES - 1)); buf[0] = 0xFF; buf[1] = 0x7F; CHECK(!sched_deserialize(&u, buf, SCHED_BYTES));   /* 32767 minutes */
    for (int i = 0; i < 20000; i++) { uint8_t b[SCHED_BYTES]; for (int j = 0; j < SCHED_BYTES; j++) b[j] = (uint8_t)rnd(); if (sched_deserialize(&u, b, SCHED_BYTES)) for (int d = 0; d < 7; d++) CHECK(u.day[d].min < 1440); }
    rec_t A, B, X; sched_serialize(&s, buf);
    rec_pack(&A, SCHED_SCHEMA, 1, buf, SCHED_BYTES); rec_pack(&B, SCHED_SCHEMA, 2, buf, SCHED_BYTES);
    CHECK(rec_pick(&A, &B) == 1); CHECK(rec_next_slot(&A, &B) == 0);
    for (size_t i = 0; i < sizeof(rec_t); i++) for (int bit = 0; bit < 8; bit++) { X = B; ((uint8_t *)&X)[i] ^= (uint8_t)(1 << bit); if (i >= offsetof(rec_t, payload) + SCHED_BYTES && i < offsetof(rec_t, crc)) continue; /* padding is zero and not covered by len but covered by crc */ CHECK(!rec_valid(&X) || X.seq == B.seq); }
    X = B; X.crc ^= 1; CHECK(rec_pick(&A, &X) == 0); CHECK(rec_pick(&X, &X) == -1);
    rec_t wrap1, wrap2; rec_pack(&wrap1, 1, 0xFFFFFFFFu, buf, 4); rec_pack(&wrap2, 1, 0, buf, 4); CHECK(rec_pick(&wrap1, &wrap2) == 1);
    /* power cut at every byte of a write: a good copy always exists and it is the old or the new one */
    rec_t slot[2]; rec_pack(&slot[0], 1, 10, buf, SCHED_BYTES); rec_pack(&slot[1], 1, 11, buf, SCHED_BYTES); int bad = 0;
    for (int w = 0; w < 50; w++) {
        int best = rec_pick(&slot[0], &slot[1]); uint32_t seq = slot[best].seq + 1; int dst = rec_next_slot(&slot[0], &slot[1]);
        sched_t ns = s; ns.day[2].min = (uint16_t)(w * 7); uint8_t nb[SCHED_BYTES]; sched_serialize(&ns, nb); rec_t neu; rec_pack(&neu, 1, seq, nb, SCHED_BYTES);
        for (size_t cut = 0; cut <= sizeof(rec_t); cut++) { rec_t torn = slot[dst]; memcpy(&torn, &neu, cut);
            rec_t pair[2]; pair[dst] = torn; pair[1 - dst] = slot[1 - dst]; int p = rec_pick(&pair[0], &pair[1]);
            if (p < 0) bad++; else if (pair[p].seq != seq && pair[p].seq != slot[best].seq) bad++; }
        slot[dst] = neu;
    }
    CHECK(bad == 0);
    /* PCF85063A */
    uint8_t reg[7]; int64_t utc; bool osc;
    for (int i = 0; i < 20000; i++) { int64_t t = U(2000, 1, 1, 0, 0, 0) + (int64_t)(rnd() % 3150000000u); pcf85063_encode(t, reg); CHECK(pcf85063_decode(reg, &utc, &osc) && utc == t && !osc); }
    pcf85063_encode(U(2028, 2, 29, 23, 59, 59), reg); reg[0] |= 0x80; CHECK(pcf85063_decode(reg, &utc, &osc) && osc && utc == U(2028, 2, 29, 23, 59, 59));
    reg[3] = 0x31; reg[5] = 0x02; CHECK(!pcf85063_decode(reg, &utc, &osc));                  /* 31 February: rejected, never a silent wrong time */
    float p = drift_ppm(30 * 86400, 45); CHECK(fabsf(p - 17.36f) < 0.05f); CHECK(drift_ppm(3 * 86400, 5) == 0); CHECK(drift_ppm(30 * 86400, 6000) == 0);
}

int main(void) {
    t_dates(); t_dst(); t_gap(); t_sched(); t_sched_prop(); t_sm(); t_sm_reboot(); t_sm_dst_night(); t_dawn(); t_storage();
    printf("\n%d checks, %d failed\n", checks, fails); return fails ? 1 : 0;
}
