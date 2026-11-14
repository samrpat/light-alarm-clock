#include "ui.h"
#include "gfx.h"
#include "icons.h"
#include <math.h>
#include <stdio.h>
#include <string.h>

#define RR 188
#define RH 148
#define OFFC 0x6b6670
#define WARN 0xE5703E
#define T(x, y, sz, str, col, w) gfx_text((x), (y), (sz), (w), (str), (col), 1)
static const col_t NA = {224, 128, 28}, BK = {0, 0, 0}, WH = {255, 255, 255};
static const char *DN[7] = {"Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"};
static const char *DF[7] = {"Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday", "Sunday"};
static const char *MON[12] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
static const char *TIMER_T[5] = {"15 min", "30 min", "1 hour", "90 min", "All night"}; static const int TIMER_N[5] = {15, 30, 60, 90, 0};
static const struct { const char *name, *dst, *off; } REG[REG_COUNT] = {
    {"US Eastern", "Daylight saving on", "UTC\xE2\x88\x92" "5"}, {"US Central", "Daylight saving on", "UTC\xE2\x88\x92" "6"}, {"US Mountain", "Daylight saving on", "UTC\xE2\x88\x92" "7"},
    {"US Pacific", "Daylight saving on", "UTC\xE2\x88\x92" "8"}, {"US Alaska", "Daylight saving on", "UTC\xE2\x88\x92" "9"}, {"US Hawaii", "No daylight saving", "UTC\xE2\x88\x92" "10"},
    {"UK", "Daylight saving on", "UTC\xC2\xB1" "0"}, {"Central Europe", "Daylight saving on", "UTC+1"}, {"Manual", "You change the clock", "Fixed"}};
const char *ui_region_name(int i) { return REG[i].name; }

enum { P_FOCUS, P_PHONE, P_ALARM, P_SOUND, P_LIGHT, P_NLIGHT, P_TIME, P_DATE, P_REGION, P_BED, P_ABOUT, P_SND0 };
typedef struct { col_t a, b, bg; } pal_t;
static const pal_t PAL[P_SND0 + SND_COUNT] = {
    {{255, 128, 98}, {232, 60, 78}, {56, 14, 18}}, {{110, 175, 255}, {70, 100, 245}, {10, 24, 66}}, {{255, 150, 70}, {255, 84, 120}, {64, 20, 30}},
    {{64, 224, 208}, {108, 124, 255}, {14, 28, 70}}, {{255, 220, 100}, {255, 150, 40}, {64, 38, 6}}, {{255, 110, 70}, {210, 40, 40}, {60, 12, 8}},
    {{100, 205, 255}, {70, 120, 255}, {10, 26, 64}}, {{255, 110, 100}, {255, 160, 90}, {60, 14, 20}}, {{90, 225, 165}, {40, 175, 225}, {8, 46, 42}},
    {{160, 140, 255}, {96, 76, 230}, {24, 18, 70}}, {{110, 225, 140}, {60, 180, 200}, {14, 36, 30}},
    /* the sixteen sounds, in order: pink white brown green grey blue violet fan | rain storm ocean stream falls wind forest fire */
    {{255, 150, 190}, {230, 90, 160}, {60, 16, 40}}, {{235, 240, 250}, {170, 180, 200}, {30, 34, 44}}, {{215, 150, 95}, {150, 90, 50}, {44, 26, 12}}, {{110, 225, 140}, {40, 170, 120}, {10, 44, 26}},
    {{190, 195, 205}, {120, 125, 140}, {28, 30, 36}}, {{100, 180, 255}, {60, 110, 240}, {10, 26, 64}}, {{190, 140, 255}, {130, 80, 230}, {34, 16, 64}}, {{170, 190, 210}, {100, 120, 150}, {22, 28, 38}},
    {{130, 190, 230}, {70, 120, 190}, {14, 30, 50}}, {{160, 170, 235}, {90, 90, 180}, {18, 18, 54}}, {{64, 224, 208}, {108, 124, 255}, {14, 28, 70}}, {{120, 225, 235}, {60, 160, 210}, {10, 40, 52}},
    {{175, 220, 245}, {90, 150, 205}, {14, 34, 52}}, {{190, 230, 225}, {110, 170, 180}, {16, 40, 42}}, {{155, 220, 125}, {70, 150, 80}, {14, 42, 18}}, {{255, 175, 85}, {230, 80, 40}, {62, 22, 6}}};
static const struct { const char *name; int ico; } SND[SND_COUNT] = {
    {"Pink noise", ICO_NZ}, {"White noise", ICO_NZ}, {"Brown noise", ICO_NZ}, {"Green noise", ICO_NZ}, {"Grey noise", ICO_NZ}, {"Blue noise", ICO_NZ}, {"Violet noise", ICO_NZ}, {"Fan", ICO_FAN},
    {"Rain", ICO_RAIN}, {"Storm", ICO_BOLT}, {"Ocean", ICO_WAVE}, {"Stream", ICO_STRM}, {"Waterfall", ICO_FALLS}, {"Wind", ICO_WIND}, {"Forest", ICO_TREE}, {"Fire", ICO_FIRE}};
const char *ui_sound_name(int i) { return SND[i].name; }

static int wrap(int m) { return ((m % 1440) + 1440) % 1440; }
static void f12(char *b, size_t n, int m, bool space) { m = wrap(m); int h = m / 60; snprintf(b, n, "%d:%02d%s%s", (h + 11) % 12 + 1, m % 60, space ? " " : "", h < 12 ? "AM" : "PM"); }

/* the alarm is the start of the day (D-039) */
static int wake_min(const ui_t *s) { if (s->days[s->wd].on) return s->days[s->wd].min; for (int i = 0; i < 7; i++) if (s->days[i].on) return s->days[i].min; return 420; }
typedef struct { int p; float w; } phase_t;       /* p: 0 day, 1 wind-down, 2 night */
static phase_t phase(const ui_t *s) {
    int wk = wake_min(s), span = wrap(s->ns - wk), since = wrap(s->now - wk);
    if (since < span) { int left = span - since; if (left > 180) return (phase_t){0, 0}; return (phase_t){1, 1 - left / 180.0f}; }
    return (phase_t){2, 1};
}
typedef struct { float ws; bool night; col_t A, B, BG; uint32_t a, b, t, s, h, trk, alarm; } theme_t;
static theme_t theme(int key, const ui_t *s) {
    phase_t ph = phase(s); float w = ph.w, ws = w * w * (3 - 2 * w); const pal_t *P = &PAL[key]; col_t amb = mulc(NA, .85f);
    theme_t t; t.ws = ws; t.night = ph.p == 2; t.A = mixc(P->a, amb, ws); t.B = mixc(P->b, mulc(amb, .75f), ws); t.BG = mixc(P->bg, BK, ws);
    col_t tx = mixc(WH, amb, ws);
    t.a = rgb(t.A); t.b = rgb(t.B); t.t = rgb(tx); t.s = rgb(mixc(tx, t.A, .45f)); t.h = rgb(mixc(t.A, t.BG, .4f)); t.trk = rgb(mixc(t.BG, tx, .2f)); t.alarm = rgb(mixc(PAL[P_ALARM].a, amb, ws));
    return t;
}
static void bg(const theme_t *pc) { (void)pc; }          /* black everywhere: the colour is in the things, not behind them */

static void dot(float x, float y, float r, const theme_t *pc, bool on) {
    gfx_dot_box(x, y, r);
    if (!on) { gfx_circle(x, y, r + 1.5f, 0x000000, 1); gfx_circle(x, y, r - 1.5f, OFFC, 1); return; }
    gfx_circle(x, y, r + 10, pc->a, .3f); gfx_circle(x, y, r + 1.5f, 0x000000, 1); gfx_circle_g(x, y, r - 1.5f, pc->a, pc->b);
    gfx_circle(x - r * .3f, y - r * .32f, r * .26f, 0xffffff, .45f);
}
static void rimT(int m, const theme_t *pc, bool on, bool fine, int drag) {
    m = wrap(m); float am = (m % 60) * 6.0f, ah = (m / 60 % 12) * 30.0f + (m % 60) * .5f, x, y;
    gfx_ring(C, C, RR, pc->trk, 3, 1); gfx_ring(C, C, RH, pc->trk, 3, .7f);
    for (int i = 0; i < 12; i++) { gfx_pt(i * 30, RR, &x, &y); gfx_circle(x, y, i ? 3 : 4.5f, pc->h, 1); gfx_pt(i * 30, RH, &x, &y); gfx_circle(x, y, i ? 2.5f : 3.5f, pc->h, 1); }
    if (fine) for (int i = 0; i < 60; i++) { if (i % 5 == 0) continue; float x0, y0; gfx_pt(i * 6, RR - 8, &x0, &y0); gfx_pt(i * 6, RR - 3, &x, &y); gfx_line(x0, y0, x, y, pc->h, 1.5f, .7f); }
    gfx_pt(ah, RH, &x, &y); dot(x, y, drag == DRAG_HR ? 21 : 16, pc, on);
    gfx_pt(am, RR, &x, &y); dot(x, y, drag == DRAG_MIN ? 27 : 21, pc, on);
}
static void rimL(float v, float lo, float hi, const theme_t *pc, int drag) {
    float a = -135 + 270 * (v - lo) / (hi - lo), x, y; gfx_pt(a, RR, &x, &y);
    gfx_arc(C, C, RR, -135, 135, pc->trk, 6, 1, true); gfx_arc(C, C, RR, -135, a < -134.8f ? -134.8f : a, pc->a, 8, 1, true); dot(x, y, drag == DRAG_LIN ? 27 : 21, pc, true);
}
static void rimS(int v, const theme_t *pc, int n, int drag) {
    float x, y; gfx_ring(C, C, RR, pc->trk, 3, 1);
    for (int i = 0; i < n; i++) { gfx_pt(i * 360.0f / n, RR, &x, &y); gfx_circle(x, y, 3, pc->h, 1); }
    gfx_pt((v % n) * 360.0f / n, RR, &x, &y); dot(x, y, drag == DRAG_STEP ? 27 : 21, pc, true);
}
static void picker(const ui_t *s, int m, const theme_t *pc, const char *title, bool on, const char *sub, uint32_t subcol, bool fine) {
    m = wrap(m); int h = m / 60; char b[8];
    rimT(m, pc, on, fine, s->drag); T(C, 140, 30, title, pc->a, 500);
    snprintf(b, sizeof b, "%d", (h + 11) % 12 + 1); T(160, 218, 80, b, on ? pc->t : OFFC, 300); T(C, 214, 66, ":", on ? pc->s : OFFC, 300);
    snprintf(b, sizeof b, "%02d", m % 60); T(306, 218, 80, b, on ? pc->t : OFFC, 300); T(C, 282, 30, h < 12 ? "AM" : "PM", on ? pc->a : OFFC, 500);
    if (sub && sub[0]) T(C, 324, 28, sub, subcol, 400);
}
static void play_ico(float x, float y, bool on, uint32_t col) {
    if (on) { gfx_pill(x - 14, y - 18, 9, 36, col, 1); gfx_pill(x + 5, y - 18, 9, 36, col, 1); }
    else gfx_tri(x - 12, y - 19, x + 19, y, x - 12, y + 19, col);
}
static float sun_k(float t_min) { float v = (t_min + 30) / 40; if (v <= 0) return 0; v = powf(v, 1.6f); return v > 1 ? 1 : v; }
static col_t sky_at(int m, int which) {
    static const struct { int m; col_t a, b; } SKY[5] = {{360, {255, 170, 120}, {150, 70, 160}}, {540, {110, 190, 255}, {50, 110, 230}}, {780, {80, 200, 255}, {40, 120, 255}}, {1020, {255, 180, 90}, {255, 95, 110}}, {1170, {240, 100, 140}, {70, 40, 140}}};
    m = wrap(m); if (m <= SKY[0].m) return which ? SKY[0].b : SKY[0].a;
    for (int i = 1; i < 5; i++) if (m <= SKY[i].m) { float k = (float)(m - SKY[i - 1].m) / (SKY[i].m - SKY[i - 1].m); return which ? mixc(SKY[i - 1].b, SKY[i].b, k) : mixc(SKY[i - 1].a, SKY[i].a, k); }
    return which ? SKY[4].b : SKY[4].a;
}
/* ---- the clock's effects: an orb that travels the rim once a day, sparks that drift and twinkle, a rising sun, ripples when the alarm rings ---- */
/* ---- the clock's colour: the edge ring is the day as a colour timeline; the bead is now, the pin is the next alarm ---- */
static col_t spec(float h) {                  /* h in hours, 0..24: indigo night, rose dawn, amber morning, sky noon, teal afternoon, coral dusk, violet evening */
    static const struct { float h; col_t c; } K[] = {{0, {70, 70, 210}}, {4, {150, 80, 230}}, {6.5f, {255, 110, 150}}, {8.5f, {255, 170, 80}}, {12, {90, 190, 255}}, {16, {80, 220, 190}}, {19, {255, 120, 120}}, {21.5f, {160, 90, 235}}, {24, {70, 70, 210}}};
    h = fmodf(h + 24, 24);
    for (int i = 1; i < 9; i++) if (h <= K[i].h) return mixc(K[i - 1].c, K[i].c, (h - K[i - 1].h) / (K[i].h - K[i - 1].h));
    return K[8].c;
}
static void fx_spectrum(const ui_t *s, float amt) {
    float now = s->now / 60.0f, ang = s->now / 4.0f; float x, y;
    for (int i = 0; i < 120; i++) { float a0 = i * 3.0f, h = (i + .5f) * .2f; bool past = a0 + 3 <= ang; gfx_arc(C, C, 214, a0, a0 + (past ? 3.4f : 3.0f), rgb(spec(h)), 8, (past ? 1.0f : .22f) * amt, false); }
    gfx_pt(ang, 214, &x, &y); col_t c = spec(now);
    for (int i = 3; i >= 1; i--) gfx_circle(x, y, 8 + i * 8, rgb(c), .12f * amt);
    gfx_circle(x, y, 11, 0xffffff, amt);
    if (s->has_next) { gfx_pt(s->next_min / 4.0f, 214, &x, &y); gfx_circle(x, y, 7, 0x000000, amt); gfx_circle(x, y, 4.5f, 0xffffff, amt); }
}
static void fx_dawn(const ui_t *s, float k) {       /* the ring warms and thickens, and a soft warm wash comes up behind the digits */
    float w = 8 + 16 * k; (void)s;
    gfx_circle(C, C, 205, 0xff9a50, .10f * k);
    for (int i = 0; i < 120; i++) gfx_arc(C, C, 214 - (w - 8) / 2, i * 3.0f, i * 3.0f + 3.4f, rgb(mixc(spec(7.0f + i * .05f), (col_t){255, 190, 100}, .4f + .6f * k)), w, .5f + .5f * k, false);
}
static void fx_alarm(const ui_t *s) {               /* ringing: the whole spectrum, breathing */
    int b = (int)(s->fx >> 2) % 12; float w = 10 + (b < 6 ? b : 12 - b) * 2;
    for (int i = 0; i < 120; i++) gfx_arc(C, C, 214, i * 3.0f, i * 3.0f + 3.4f, rgb(spec(5 + i * .12f)), w, 1, false);
}
static void fx_eq(const ui_t *s, uint32_t col) {              /* sound is playing: a small equalizer dances under the name */
    for (int i = 0; i < 7; i++) { int h = 6 + (int)(18 * (.5f + .5f * sinf(s->fx * .23f * (1 + .17f * i) + i * 1.3f))) / 3 * 3; gfx_pill(C - 49 + i * 14, 170 - h / 2.0f, 8, (float)h, col, 1); }
}
static void next_text(const ui_t *s, char *b, size_t n) { char t[12]; f12(t, sizeof t, s->next_min, true); if (s->next_far) snprintf(b, n, "%s %s", DN[s->next_wd], t); else snprintf(b, n, "%s", t); }

static void draw_clock(const ui_t *s) {
    phase_t ph = phase(s); float w = ph.w, ws = w * w * (3 - 2 * w), aw = s->aw; bool night = ph.p == 2, dawn = s->eng_state == 1;
    uint32_t tc, sc, hc, wc = WARN; char b[40];
    if (dawn) {
        float k = sun_k(s->dawn_t / 60.0f), kt = sqrtf(k); col_t am = mulc(NA, .6f);
        fx_dawn(s, k);
        tc = rgb(mixc(am, WH, kt)); sc = rgb(mixc(mulc(am, .72f), (col_t){255, 236, 220}, kt)); hc = sc;
    } else if (night) {
        float vis = .5f + .35f * aw; col_t am = mulc(NA, vis);
        tc = rgb(am); sc = rgb(mulc(am, .72f)); hc = rgb(mulc(am, .5f));
    } else {
        float k = .55f + .45f * s->dayB / 100.0f; col_t top = mulc(mixc(sky_at(s->now, 0), BK, ws), k), bot = mulc(mixc(sky_at(s->now, 1), BK, ws), k), am = mulc(NA, .85f);
        fx_spectrum(s, 1 - ws); (void)top; (void)bot;
        tc = rgb(mixc(mixc(WH, spec(s->now / 60.0f), .22f), am, ws)); sc = rgb(mixc(mixc(WH, top, .12f), mulc(am, .78f), ws)); hc = sc; if (ws < .6f) wc = tc;
    }
    if (!dawn && (ph.p == 0 || (ph.p == 1 && ph.w < .5f))) { snprintf(b, sizeof b, "%s, %s %d", DN[s->wd], MON[s->mo - 1], s->d); T(C, 128, 28, b, sc, 500); }
    gfx_time(C, 208, 124, s->now, tc, 28);
    if (dawn) T(C, 296, 26, "Sunrise", sc, 400);
    else if (!s->has_next) T(C, 296, 26, "No alarm set", sc, 400);
    else { next_text(s, b, sizeof b); float tw = gfx_text_w(26, 400, b); gfx_icon(ICO_BELL, C + 14 - tw / 2 - 18, 295, sc, .72f, 1, true); T(C + 14, 296, 26, b, sc, 400); }
    if (s->status[0]) T(C, 338, 26, s->status, s->status_warn ? WARN : wc, 500);
    if (aw > .01f && !dawn) {
        if (!s->status[0]) gfx_icon(ICO_SET, C, 346, hc, 1, aw, true);
        gfx_icon(ICO_CAL, C, 58, hc, 1, aw, true); gfx_icon(ICO_BELL, C, 408, hc, 1, aw, true); gfx_icon(ICO_LAMP, 66, 300, hc, 1, aw, true); gfx_icon(ICO_SPK, 400, 300, hc, 1, aw, true);
    }
}
static void draw_tmr(const ui_t *s) {
    theme_t pc = theme(P_ALARM, s); bg(&pc); const char *title = s->tgt_today ? "Today" : "Tomorrow";
    int t = s->tgt_one >= 0 ? s->tgt_one : s->tgt_usual;
    if (t < 0) { picker(s, 420, &pc, title, false, "Drag a dot to set one", pc.s, false); return; }
    picker(s, t, &pc, title, !s->tgt_skip, s->tgt_skip ? "Skipped" : s->tgt_one >= 0 ? "Once only" : "", s->tgt_skip ? WARN : pc.s, false);
    if (s->tgt_skip) gfx_line(106, 218, 360, 218, WARN, 5, 1);
}
static void draw_week(const ui_t *s) {
    theme_t pc = theme(P_ALARM, s); bg(&pc); char b[16];
    for (int i = 0; i < 7; i++) {
        float a0 = i * 360.0f / 7 - 180.0f / 7 + 1.5f, a1 = a0 + 360.0f / 7 - 3, x, y; const sched_day_t *d = &s->days[i]; gfx_pt((a0 + a1) / 2, 148, &x, &y);
        uint32_t dk = pc.night ? pc.t : 0x2a0a12;
        if (d->on) gfx_arc(C, C, 160, a0, a1, pc.night ? 0x2a1a0a : rgb(mixc(pc.A, pc.B, i / 6.0f)), 124, pc.night ? 1 : .95f, false);
        else gfx_arc(C, C, 160, a0, a1, pc.trk, 124, .55f, false);
        T(x, y - 15, 28, DN[i], d->on ? dk : pc.h, 500);
        if (d->on) { f12(b, sizeof b, d->min, false); T(x, y + 16, 26, b, dk, 500); } else T(x, y + 16, 26, "off", pc.h, 400);
    }
    gfx_circle(C, C, 90, 0x000000, .55f);
    if (!s->has_next) T(C, 233, 34, "None", pc.t, 500);
    else { T(C, 214, 34, DN[s->next_wd], pc.t, 500); f12(b, sizeof b, s->next_min, true); T(C, 254, 30, b, pc.a, 500); }
}
static void draw_day(const ui_t *s) { theme_t pc = theme(P_ALARM, s); bg(&pc); const sched_day_t *d = &s->days[s->sel]; picker(s, d->min, &pc, DF[s->sel], d->on, d->on ? "" : "Off", pc.h, false); }
static void draw_sound(const ui_t *s) {
    theme_t pc = theme(P_SND0 + s->snd, s); bg(&pc); if (s->play) fx_eq(s, pc.a); float x, y; char b[8];
    if (s->pick == PICK_SND) {
        for (int i = 0; i < 8; i++) {
            int id = s->pp * 8 + i; theme_t q = theme(P_SND0 + id, s); gfx_pt(i * 45, 150, &x, &y);
            if (id == s->snd) gfx_ring(x, y, 52, pc.t, 4, 1);
            gfx_circle_g(x, y, 44, q.a, q.b); gfx_circle(x - 13, y - 15, 11, 0xffffff, .22f); gfx_icon(SND[id].ico, x, y, 0x15101c, 1.15f, 1, true);
        }
        T(C, 206, 26, s->pp ? "Nature" : "Noise", pc.s, 400); T(C, 242, 30, SND[s->snd].name, pc.t, 500);
        gfx_pill(C - 18, 270, s->pp ? 6 : 14, 6, s->pp ? pc.trk : pc.t, 1); gfx_pill(C - 18 + (s->pp ? 10 : 18), 270, s->pp ? 14 : 6, 6, s->pp ? pc.t : pc.trk, 1);
    } else if (s->pick == PICK_TMR) {
        for (int i = 0; i < 5; i++) {
            gfx_pt(i * 72, 150, &x, &y); bool sel = i == s->ti;
            if (sel) gfx_circle_g(x, y, 50, pc.a, pc.b); else gfx_circle(x, y, 50, pc.trk, 1);
            if (i < 4) { snprintf(b, sizeof b, "%d", TIMER_N[i]); T(x, y, 38, b, sel ? 0x15101c : pc.t, sel ? 500 : 400); } else gfx_icon(ICO_MOON, x, y, sel ? 0x15101c : pc.t, 1.3f, 1, true);
        }
        T(C, 214, 28, "Sleep timer", pc.t, 500); T(C, 252, 26, s->ti < 4 ? "Plays again before dawn" : "Plays all night", pc.s, 400);
    } else {
        static const int BH[4] = {18, 30, 24, 14};
        rimL(s->vol, 0, 100, &pc, s->drag);
        gfx_pill(C - 125, 112, 250, 56, pc.trk, .7f); gfx_circle_g(C - 98, 140, 16, pc.a, pc.b); T(C + 6, 140, 28, SND[s->snd].name, pc.t, 500); gfx_icon(ICO_CHV, C + 104, 140, pc.s, 1, 1, true);
        for (int i = 0; i < 4; i++) for (int sg = -1; sg <= 1; sg += 2) gfx_pill(C + sg * (52 + i * 17) - 5, 218 - BH[i] / 2.0f, 10, BH[i], s->play ? pc.a : pc.trk, 1);
        play_ico(C, 218, s->play, pc.t);
        gfx_pill(C - 125, 268, 250, 56, pc.trk, .7f); gfx_icon(ICO_MOON, C - 92, 296, pc.a, 1, 1, true); T(C, 296, 28, TIMER_T[s->ti], pc.t, 400); gfx_icon(ICO_CHV, C + 104, 296, pc.s, 1, 1, true);
    }
}
static void draw_focus(const ui_t *s) {
    theme_t pc = theme(P_FOCUS, s); bg(&pc); bool brk = s->fo.ph == 1; int tot = (brk ? s->fo.brk : s->fo.len) * 60; float x, y; char b[24];
    static const int OPT[3][5] = {{15, 25, 35, 50, 60}, {3, 5, 10, 15, 20}, {2, 3, 4, 5, 6}};
    static const char *TT[3] = {"Focus length", "Break length", "Rounds"}, *UN[3] = {"minutes", "minutes", "focus rounds in a cycle"};
    if (s->fo.menu && s->fo.pick) {
        int k = s->fo.pick - 1, cur = k == 0 ? s->fo.len : k == 1 ? s->fo.brk : s->fo.rds;
        for (int i = 0; i < 5; i++) { gfx_pt(i * 72, 150, &x, &y); bool sel = OPT[k][i] == cur; if (sel) gfx_circle_g(x, y, 50, pc.a, pc.b); else gfx_circle(x, y, 50, pc.trk, 1); snprintf(b, sizeof b, "%d", OPT[k][i]); T(x, y, 38, b, sel ? 0x15101c : pc.t, sel ? 500 : 400); }
        T(C, 214, 28, TT[k], pc.t, 500); T(C, 252, 26, UN[k], pc.s, 400);
    } else if (s->fo.menu) {
        const char *rn[3] = {"Focus", "Break", "Rounds"};
        for (int k = 0; k < 3; k++) {
            float yy = 150 + k * 83; gfx_pill(C - 125, yy - 28, 250, 56, pc.trk, .7f); T(C - 62, yy, 28, rn[k], pc.s, 400);
            if (k < 2) snprintf(b, sizeof b, "%d min", k ? s->fo.brk : s->fo.len); else snprintf(b, sizeof b, "%d", s->fo.rds);
            T(C + 44, yy, 28, b, pc.t, 500); gfx_icon(ICO_CHV, C + 104, yy, pc.s, 1, 1, true);
        }
    } else {
        gfx_ring(C, C, RR, pc.trk, 6, 1); float fr = 1 - (float)s->fo.left / tot;
        if (s->fo.run || s->fo.left != tot) gfx_arc(C, C, RR, 0, fr * 360 < .4f ? .4f : fr * 360, brk ? pc.s : pc.a, 8, 1, true);
        int n = s->fo.rds; float x0 = C - (n - 1) * 11;
        for (int i = 0; i < n; i++) { bool dn = brk ? i < s->fo.rd : i < s->fo.rd - 1, cu = !brk && i == s->fo.rd - 1; gfx_circle(x0 + i * 22, 272, 7, dn ? pc.t : cu ? pc.a : pc.trk, 1); if (cu && s->fo.run) gfx_circle(x0 + i * 22, 272, 11 + 3 * sinf(s->fx * .15f), pc.a, .3f); }
        gfx_pill(C - 125, 84, 250, 56, pc.trk, .7f); T(C - 6, 112, 28, brk ? "Break" : "Focus", pc.t, 500); gfx_icon(ICO_CHV, C + 104, 112, pc.s, 1, 1, true);
        snprintf(b, sizeof b, "%d:%02d", s->fo.left / 60, s->fo.left % 60); T(C, 206, 96, b, pc.t, 300);
        if (s->fo.run) gfx_pill(C - 125, 300, 250, 56, pc.trk, .7f); else gfx_pill_g(C - 125, 300, 250, 56, pc.a, pc.b);
        play_ico(C - 60, 328, s->fo.run, s->fo.run ? pc.t : 0x15101c); T(C + 16, 328, 28, s->fo.run ? "Pause" : s->fo.left != tot ? "Resume" : "Start", s->fo.run ? pc.t : 0x15101c, 500);
    }
}
static void draw_light(const ui_t *s) {
    int m = s->lt.m; theme_t pc = theme(m ? P_LIGHT : P_NLIGHT, s); bg(&pc); int l = s->lt.l[m]; char b[8];
    rimL(l, 5, 100, &pc, s->drag); T(C, 112, 30, "Lamp", pc.a, 500);
    if (s->lt.on) { snprintf(b, sizeof b, "%d%%", l); T(C, 200, 76, b, pc.t, 300); } else T(C, 200, 76, "Off", OFFC, 300);
    gfx_pill(C - 125, 265, 250, 56, pc.trk, .6f); gfx_pill_g(m ? C : C - 125, 265, 125, 56, pc.a, pc.b);
    gfx_icon(ICO_MOON, C - 62, 293, m ? pc.s : 0x2a0d06, 1, 1, true); gfx_icon(ICO_SUN, C + 62, 293, m ? 0x2a1a04 : pc.s, 1, 1, true);
}
static void draw_ring(const ui_t *s) {
    fx_alarm(s); float x0, y0, x1, y1;
    if (s->done) { for (int i = 0; i < 12; i++) { gfx_pt(i * 30, 120, &x0, &y0); gfx_pt(i * 30, 210, &x1, &y1); gfx_line(x0, y0, x1, y1, 0xffffff, 10, .3f); } T(C, 222, 40, "Good", 0xffffff, 500); T(C, 266, 40, "morning", 0xffffff, 500); return; }
    gfx_time(C, 190, 124, s->now, 0xffffff, 28); T(C, 268, 30, s->snz_left ? "Tap to snooze" : "No snoozes left", 0xffffff, 500); T(C, 312, 26, "Hold to stop", 0xffe9d6, 400);
    for (int k = 0; k < 3; k++) gfx_circle(C - 20 + k * 20, 352, 6, 0xffffff, k < s->snz_left ? 1 : .3f);
}
static void draw_set(const ui_t *s) {
    static const int KEY[PG_COUNT] = {P_TIME, P_DATE, P_REGION, P_PHONE, P_BED, P_LIGHT, P_ALARM, P_ABOUT};
    theme_t pc = theme(KEY[s->pg], s); bg(&pc); float x, y; char b[40], t[16];
    switch (s->pg) {
    case PG_TIME:
        if (s->sec_mode) {
            gfx_ring(C, C, RR, pc.trk, 3, 1);
            for (int i = 0; i < 60; i++) { gfx_pt(i * 6, RR, &x, &y); gfx_circle(x, y, i % 5 == 0 ? 3.4f : 1.8f, pc.h, 1); }
            gfx_arc(C, C, RR, 0, s->sec * 6 < .3f ? .3f : s->sec * 6, pc.a, 8, 1, true); gfx_pt(s->sec * 6, RR, &x, &y); dot(x, y, s->drag == DRAG_SEC ? 27 : 21, &pc, true);
            T(C, 128, 26, "Seconds", pc.a, 500); snprintf(b, sizeof b, "%02d", s->sec); T(C, 222, 96, b, pc.t, 300); f12(t, sizeof t, s->clk, true); T(C, 296, 28, t, pc.s, 400); T(C, 344, 26, "Tap to start", pc.s, 500);
        } else { snprintf(b, sizeof b, "Seconds %02d", s->sec); picker(s, s->clk, &pc, "Time", true, b, pc.s, true); }
        break;
    case PG_DATE: {
        static const float UL[3][3] = {{C - 44, C + 44, 180}, {C - 66, C + 66, 296}, {C - 56, C + 56, 352}};
        rimS(s->df == 0 ? s->dat.m : s->df == 1 ? s->dat.d - 1 : s->dat.y, &pc, s->df == 1 ? 31 : 12, s->drag);
        T(C, 150, 44, MON[s->dat.m], s->df == 0 ? pc.a : pc.s, 500); snprintf(b, sizeof b, "%d", s->dat.d); T(C, 236, 110, b, pc.t, 300);
        snprintf(b, sizeof b, "%d", s->dat.y); T(C, 326, 44, b, s->df == 2 ? pc.a : pc.s, 300); gfx_line(UL[s->df][0], UL[s->df][2], UL[s->df][1], UL[s->df][2], pc.a, 4, 1);
        break; }
    case PG_REGION:
        rimS(s->rg, &pc, REG_COUNT, s->drag); T(C, 128, 30, "Region", pc.a, 500); T(C, 206, 46, REG[s->rg].name, pc.t, 300); T(C, 262, 38, REG[s->rg].off, pc.s, 300); T(C, 318, 28, REG[s->rg].dst, pc.h, 400);
        break;
    case PG_PHONE:
        gfx_circle(C, 200, 46, pc.trk, 1); gfx_icon(ICO_BT, C, 200, pc.t, 1.7f, 1, true); T(C, 104, 30, "Phone", pc.a, 500); T(C, 296, 34, "Off", pc.t, 400); T(C, 338, 26, "Time sync: not built yet", pc.s, 400);
        break;
    case PG_BEDTIME: {
        int bed = wrap(s->ns), wk = wake_min(s), dur = wrap(wk - bed); float ab = (bed / 60 % 12) * 30.0f + (bed % 60) * .5f, span = dur * .5f > 359 ? 359 : dur * .5f, wx, wy;
        gfx_ring(C, C, RR, pc.trk, 3, 1);
        for (int i = 0; i < 18; i++) gfx_arc(C, C, RR, ab - 90 + i * 5, ab - 90 + i * 5 + 5.6f, pc.a, 10, (i + 1) / 18.0f * .45f, false);
        gfx_arc(C, C, RR, ab, ab + span, pc.a, 12, 1, true);
        gfx_pt(ab + span, RR, &wx, &wy); gfx_circle(wx, wy, 19.5f, pc.alarm, 1); gfx_circle(wx, wy, 16.5f, 0x000000, 1); gfx_icon(ICO_BELL, wx, wy, pc.alarm, .75f, 1, false);
        gfx_pt(ab, RR, &x, &y); dot(x, y, s->drag == DRAG_BED ? 27 : 21, &pc, true); gfx_icon(ICO_MOON, x, y, 0x140c33, .8f, 1, false);
        T(C, 128, 30, "Bedtime", pc.a, 500); f12(t, sizeof t, bed, true); T(C, 204, 56, t, pc.t, 300);
        f12(t, sizeof t, bed - 180, true); snprintf(b, sizeof b, "Wind down %s", t); T(C, 266, 28, b, pc.s, 400); f12(t, sizeof t, wk, true); snprintf(b, sizeof b, "Alarm %s", t); T(C, 310, 28, b, pc.alarm, 500);
        break; }
    case PG_BRIGHT: {
        float v = (s->dayB - 10) / 90.0f, r = 26 + 34 * v; rimL(s->dayB, 10, 100, &pc, s->drag);
        gfx_circle(C, 196, r + 24, pc.a, .1f + .14f * v); gfx_circle(C, 196, r + 10, pc.a, .16f + .2f * v); gfx_circle_g(C, 196, r, pc.a, pc.b);
        T(C, 109, 30, "Brightness", pc.a, 500); snprintf(b, sizeof b, "%d%%", s->dayB); T(C, 298, 46, b, pc.t, 300); T(C, 344, 26, "Night stays dim", pc.s, 400);
        break; }
    case PG_ALARM: {
        int nw = (int)lroundf(1 + (s->ld - 30) / 70.0f * 2); rimL(s->ld, 30, 100, &pc, s->drag);
        for (int i = 1; i <= 3; i++) { float r = 44 + i * 20; uint32_t c = i <= nw ? pc.a : pc.trk; gfx_arc(C, 196, r, 62, 118, c, 7, 1, true); gfx_arc(C, 196, r, 242, 298, c, 7, 1, true); }
        gfx_icon(ICO_BELL, C, 196, pc.t, 2.6f, 1, true); T(C, 104, 30, "Alarm", pc.a, 500); snprintf(b, sizeof b, "%d%%", s->ld); T(C, 296, 46, b, pc.t, 300); T(C, 340, 26, s->test ? "Tap to stop" : "Tap to test", pc.s, 400);
        break; }
    default: {
        float f = s->bat.present ? s->bat.pct / 100.0f : 0; gfx_arc(C, C, 132, -135, 135, pc.trk, 6, 1, true); if (f > 0.01f) gfx_arc(C, C, 132, -135, -135 + 270 * f, pc.a, 10, 1, true);
        T(C, 146, 28, "Battery", pc.a, 500);
        if (s->bat.present) { snprintf(b, sizeof b, "%d%%", s->bat.pct); T(C, 212, 72, b, pc.t, 300); snprintf(b, sizeof b, "%d.%02d V", s->bat.mv / 1000, s->bat.mv % 1000 / 10); T(C, 270, 28, b, pc.s, 400); }
        else { T(C, 212, 46, "None", pc.t, 300); T(C, 270, 28, "Runs on USB", pc.s, 400); }
        T(C, 346, 26, "Version 0.3", pc.h, 400);
        break; }
    }
    for (int i = 0; i < PG_COUNT; i++) { float cx = C + (i - (PG_COUNT - 1) / 2.0f) * 16, w = i == s->pg ? 14 : 6; gfx_pill(cx - w / 2, 378, w, 6, i == s->pg ? pc.t : pc.trk, 1); }
}
void ui_draw(const ui_t *s) {
    switch (s->scr) {
    case SCR_CLOCK: draw_clock(s); break;
    case SCR_TMR: draw_tmr(s); break;
    case SCR_WEEK: draw_week(s); break;
    case SCR_DAY: draw_day(s); break;
    case SCR_SOUND: draw_sound(s); break;
    case SCR_FOCUS: draw_focus(s); break;
    case SCR_LIGHT: draw_light(s); break;
    case SCR_RING: draw_ring(s); break;
    default: draw_set(s); break;
    }
    if (s->hold > 0) gfx_arc(C, C, 224, 0, 360 * s->hold, 0xffffff, 10, 1, true);
}
void ui_fresh(ui_t *s) {
    memset(s, 0, sizeof *s); s->scr = SCR_CLOCK; s->now = 870; s->wd = 3; s->y = 2026; s->mo = 11; s->d = 12;
    for (int i = 0; i < 7; i++) { s->days[i].on = false; s->days[i].min = 420; }
    s->tgt_usual = s->tgt_one = -1; s->sel = 0; s->vol = 40; s->ti = 1; s->lt.l[0] = 20; s->lt.l[1] = 60; s->snz_left = 3;
    s->dat.m = 10; s->dat.d = 12; s->dat.y = 2026; s->dayB = 60; s->ns = 1350; s->ld = 70; s->clk = 870;
    s->fo.len = 25; s->fo.brk = 5; s->fo.rds = 4; s->fo.left = 1500; s->fo.rd = 1;
}
