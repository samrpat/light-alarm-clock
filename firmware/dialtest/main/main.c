/*
 * Dial and swipe test for the sunrise alarm clock. NOT the product firmware.
 *
 * It lets you feel on the real glass what the concept page does in a browser:
 *   - a time (6:45 AM) with TWO DOTS on two rings, like the hands of a clock
 *       outer dot = minutes: 5 minutes per 30 degrees, and the hour moves on as it passes the top
 *       inner dot = hours:   1 hour per 30 degrees
 *   - grab a dot (touch it, pause a moment) and slide it around the ring
 *   - swipe anywhere else, in any direction: it must NEVER change the time
 * There are no buttons. Counters show how each of your touches was understood, so you can
 * look for any mix-up between swiping and turning.
 *
 * The rules (same as docs/UX.md section 3):
 *   tap    : under 0.4 s, moves under 16 px
 *   swipe  : moves 56 px or more
 *   dial   : starts within 56 px of a dot, after at least 90 ms, and the first 16 px of
 *            movement follow the ring (tangent), not toward or away from the centre
 *   16-56 px of movement that is not a dial turn is ignored
 */
#include <math.h>
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "esp_log.h"
#include "bsp/esp-bsp.h"
#include "bsp/display.h"
#include "lvgl.h"

static const char *TAG = "dialtest";

#define CX 233
#define CY 233
#define RM 188          /* radius of the minutes ring (outer) */
#define RH 148          /* radius of the hours ring (inner) */
#define GRAB 56         /* how close a touch must land to a dot to grab it */
#define STEP_DEG 30.0f

enum { DOT_NONE, DOT_MIN, DOT_HR };

static int s_minutes = 6 * 60 + 45;
static lv_obj_t *s_dot_min, *s_dot_hr, *s_time, *s_last, *s_count;
static int n_dial, n_swipe, n_tap, n_ignored;

static struct {
    int dot;
    bool moved, dial;
    int x0, y0;
    uint32_t t0;
    float ang_last, acc;
    int applied;
} g;

static int wrap(int m) { return ((m % 1440) + 1440) % 1440; }
static float angle_of(int x, int y) { return atan2f((float)(x - CX), (float)(CY - y)) * 57.29578f; }
static void pos_of(float deg, int r, int *x, int *y)
{
    float a = deg * 0.0174533f;
    *x = CX + (int)(r * sinf(a));
    *y = CY - (int)(r * cosf(a));
}
static void dot_pos(int dot, int *x, int *y)
{
    int m = wrap(s_minutes);
    if (dot == DOT_MIN) pos_of((m % 60) * 6.0f, RM, x, y);
    else pos_of(((m / 60) % 12) * 30.0f + (m % 60) * 0.5f, RH, x, y);
}

static void refresh(void)
{
    int m = wrap(s_minutes), h = m / 60;
    lv_label_set_text_fmt(s_time, "%d:%02d %s", (h + 11) % 12 + 1, m % 60, h < 12 ? "AM" : "PM");
    int x, y;
    dot_pos(DOT_MIN, &x, &y);
    lv_obj_set_pos(s_dot_min, x - 21, y - 21);
    dot_pos(DOT_HR, &x, &y);
    lv_obj_set_pos(s_dot_hr, x - 16, y - 16);
    lv_label_set_text_fmt(s_count, "dial %d  swipes %d  taps %d\nignored %d", n_dial, n_swipe, n_tap, n_ignored);
}

static void say(const char *t) { lv_label_set_text(s_last, t); }

static void on_touch(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_point_t p;
    lv_indev_get_point(lv_indev_active(), &p);
    uint32_t now = lv_tick_get();

    if (code == LV_EVENT_PRESSED) {
        g.moved = g.dial = false;
        g.x0 = p.x; g.y0 = p.y; g.t0 = now; g.acc = 0; g.applied = 0;
        int mx, my, hx, hy;
        dot_pos(DOT_MIN, &mx, &my);
        dot_pos(DOT_HR, &hx, &hy);
        float dm = hypotf((float)(p.x - mx), (float)(p.y - my));
        float dh = hypotf((float)(p.x - hx), (float)(p.y - hy));
        g.dot = DOT_NONE;
        if (dm <= GRAB && dm <= dh) g.dot = DOT_MIN;                  /* the nearest dot wins */
        else if (dh <= GRAB) g.dot = DOT_HR;
    } else if (code == LV_EVENT_PRESSING) {
        float mvx = (float)(p.x - g.x0), mvy = (float)(p.y - g.y0);
        if (!g.moved && hypotf(mvx, mvy) > 16.0f) {
            g.moved = true;
            if (g.dot && (now - g.t0) < 90) g.dot = DOT_NONE;         /* a flick, not a grab */
            if (g.dot) {
                float r0 = hypotf((float)(g.x0 - CX), (float)(g.y0 - CY));
                if (r0 < 1.0f) r0 = 1.0f;
                float ux = (g.x0 - CX) / r0, uy = (g.y0 - CY) / r0;
                float tang = fabsf(-uy * mvx + ux * mvy), rad = fabsf(ux * mvx + uy * mvy);
                if (tang > rad) { g.dial = true; g.ang_last = angle_of(g.x0, g.y0); }
                else g.dot = DOT_NONE;                                /* toward or away from the centre: a swipe */
            }
        }
        if (g.dial) {
            float a = angle_of(p.x, p.y), d = a - g.ang_last;
            if (d > 180) d -= 360;
            if (d < -180) d += 360;
            g.ang_last = a;
            g.acc += d;
            int steps = (int)lroundf(g.acc / STEP_DEG);
            if (steps != g.applied) {
                s_minutes = wrap(s_minutes + (steps - g.applied) * (g.dot == DOT_MIN ? 5 : 60));
                g.applied = steps;
                n_dial++;
                say(g.dot == DOT_MIN ? "Minutes dial" : "Hours dial");
                refresh();
            }
        }
    } else if (code == LV_EVENT_RELEASED) {
        float mvx = (float)(p.x - g.x0), mvy = (float)(p.y - g.y0), dist = hypotf(mvx, mvy);
        if (g.dial) {
            say("Dial turn finished");
        } else if (dist >= 56.0f) {
            n_swipe++;
            say(fabsf(mvx) > fabsf(mvy) ? (mvx > 0 ? "Swipe right (time unchanged)" : "Swipe left (time unchanged)")
                                        : (mvy > 0 ? "Swipe down (time unchanged)" : "Swipe up (time unchanged)"));
        } else if (dist < 16.0f && (now - g.t0) < 400) {
            n_tap++;
            say("Tap (taps do nothing to the time)");
        } else {
            n_ignored++;
            say("Ignored (16 to 56 px, or a slow press)");
        }
        refresh();
    }
}

static lv_obj_t *label(lv_obj_t *parent, const lv_font_t *font, uint32_t color, int y)
{
    lv_obj_t *l = lv_label_create(parent);
    lv_obj_set_style_text_font(l, font, 0);
    lv_obj_set_style_text_color(l, lv_color_hex(color), 0);
    lv_obj_set_style_text_align(l, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_width(l, 260);
    lv_obj_align(l, LV_ALIGN_TOP_MID, 0, y);
    return l;
}

static lv_obj_t *circle(lv_obj_t *parent, int d, uint32_t color, int border)
{
    lv_obj_t *o = lv_obj_create(parent);
    lv_obj_set_size(o, d, d);
    lv_obj_set_style_radius(o, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(o, lv_color_hex(color), 0);
    lv_obj_set_style_border_width(o, border, 0);
    lv_obj_set_style_border_color(o, lv_color_black(), 0);
    lv_obj_remove_flag(o, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_remove_flag(o, LV_OBJ_FLAG_SCROLLABLE);
    return o;
}

static void build_ui(void)
{
    lv_obj_t *scr = lv_screen_active();
    lv_obj_set_style_bg_color(scr, lv_color_black(), 0);
    lv_obj_add_flag(scr, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(scr, on_touch, LV_EVENT_ALL, NULL);

    for (int ring = 0; ring < 2; ring++) {                       /* twelve small dots on each ring, like clock marks */
        int r = ring ? RH : RM;
        for (int i = 0; i < 12; i++) {
            int s = ring ? (i == 0 ? 8 : 6) : (i == 0 ? 10 : 7);
            lv_obj_t *d = circle(scr, s, ring ? 0x5A3812 : 0x7A4A18, 0);
            int x, y;
            pos_of(i * 30.0f, r, &x, &y);
            lv_obj_set_pos(d, x - s / 2, y - s / 2);
        }
    }

    s_time = label(scr, &lv_font_montserrat_48, 0xFFB869, 150);
    s_last = label(scr, &lv_font_montserrat_20, 0xB27C4A, 225);
    s_count = label(scr, &lv_font_montserrat_20, 0x8A6B4A, 275);
    say("Grab a dot and slide it");

    s_dot_hr = circle(scr, 32, 0xB27C4A, 3);
    s_dot_min = circle(scr, 42, 0xE0801C, 3);
    refresh();
}

void app_main(void)
{
    ESP_LOGI(TAG, "dial test starting");
    bsp_display_start();
    bsp_display_brightness_set(60);
    bsp_display_lock(-1);
    build_ui();
    bsp_display_unlock();
}
