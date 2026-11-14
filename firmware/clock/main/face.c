/* The screen, for now. GRAPHICS ARE TEMPORARY (the real faces of UX.md come in step 3c), but the stop gestures are
 * the real ones (R23, D-036): while the alarm rings a TAP snoozes and a 2 s HOLD stops it, with a rim that fills
 * while you hold. This task only reads the snapshot and posts messages; it can never delay an alarm (R18). */
#include <time.h>
#include <string.h>
#include <stdio.h>
#include "esp_timer.h"
#include "bsp/esp-bsp.h"
#include "lvgl.h"
#include "hw.h"
#include "tz.h"
#include "store.h"

static lv_obj_t *t_lbl, *s_lbl, *n_lbl, *arc; static tz_t tz;
static int64_t press_t0; static bool fired;
#define HOLD_US 2000000

static bool stoppable(sm_state_t st) { return st == SM_SUNRISE || st == SM_RINGING || st == SM_SNOOZED || st == SM_MISSED; }
static void on_touch(lv_event_t *e) {
    lv_event_code_t c = lv_event_get_code(e); alarm_snapshot_t s; alarm_snapshot_get(&s); int64_t now = esp_timer_get_time();
    if (c == LV_EVENT_PRESSED) { press_t0 = now; fired = false; }
    else if (c == LV_EVENT_PRESSING) {
        if (!stoppable(s.state) || fired) return;
        int64_t held = now - press_t0;
        if (held > 450000) { lv_obj_remove_flag(arc, LV_OBJ_FLAG_HIDDEN); lv_arc_set_value(arc, (int)((held - 450000) * 100 / (HOLD_US - 450000))); }   /* indicator waits 0.45 s (D-042) */
        if (held >= HOLD_US) { fired = true; alarm_post(CMD_DISMISS, 0, 0); lv_obj_add_flag(arc, LV_OBJ_FLAG_HIDDEN); }
    } else if (c == LV_EVENT_RELEASED || c == LV_EVENT_PRESS_LOST) {
        lv_obj_add_flag(arc, LV_OBJ_FLAG_HIDDEN);
        if (c == LV_EVENT_RELEASED && !fired && now - press_t0 < 500000 && s.state == SM_RINGING) alarm_post(CMD_SNOOZE, 0, 0);
    }
}
static void fmt12(char *b, size_t n, int64_t local) { int m = (int)((local % 86400 + 86400) % 86400 / 60); snprintf(b, n, "%d:%02d %s", ((m / 60 + 11) % 12) + 1, m % 60, m / 60 < 12 ? "AM" : "PM"); }
static void tick(lv_timer_t *t) {
    (void)t; alarm_snapshot_t s; alarm_snapshot_get(&s); char b[24];
    if (s.no_time) lv_label_set_text(t_lbl, "Set time");
    else { fmt12(b, sizeof b, tz_utc_to_local(&tz, time(NULL))); b[strlen(b) - 3] = 0; lv_label_set_text(t_lbl, b); }
    const char *l2 = "";
    switch (s.state) {
    case SM_SUNRISE: l2 = "Dawn  ·  hold to cancel"; break;
    case SM_RINGING: l2 = s.snoozes_left > 0 ? "Tap to snooze" : "No snoozes left"; break;
    case SM_SNOOZED: l2 = "Snoozed"; break;
    case SM_MISSED:  l2 = "Alarm missed"; break;
    default: l2 = s.sched_damaged ? "Alarms lost: set again" : g_time_estimated ? "Check time" : ""; break;
    }
    lv_label_set_text(s_lbl, l2);
    if (s.state == SM_RINGING || s.state == SM_SNOOZED || s.state == SM_MISSED) lv_label_set_text(n_lbl, "Hold to stop");
    else if (s.next_utc) { char a[16]; fmt12(a, sizeof a, tz_utc_to_local(&tz, s.next_utc)); lv_label_set_text_fmt(n_lbl, "Alarm %s", a); }
    else lv_label_set_text(n_lbl, s.no_time ? "" : "No alarm set");
}
void face_start(void) {
    char z[64]; store_load_region(z, sizeof z); if (!tz_parse(z, &tz)) tz_parse("EST5EDT,M3.2.0/2,M11.1.0/2", &tz);
    bsp_display_lock(-1);
    lv_obj_t *scr = lv_screen_active(); lv_obj_set_style_bg_color(scr, lv_color_black(), 0);
    lv_obj_add_flag(scr, LV_OBJ_FLAG_CLICKABLE); lv_obj_add_event_cb(scr, on_touch, LV_EVENT_ALL, NULL);
    arc = lv_arc_create(scr); lv_obj_set_size(arc, 440, 440); lv_obj_center(arc); lv_arc_set_rotation(arc, 270); lv_arc_set_bg_angles(arc, 0, 360);
    lv_arc_set_range(arc, 0, 100); lv_obj_remove_style(arc, NULL, LV_PART_KNOB); lv_obj_remove_flag(arc, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_arc_width(arc, 10, LV_PART_MAIN); lv_obj_set_style_arc_width(arc, 10, LV_PART_INDICATOR);
    lv_obj_set_style_arc_color(arc, lv_color_hex(0x2a1a0a), LV_PART_MAIN); lv_obj_set_style_arc_color(arc, lv_color_hex(0xE5703E), LV_PART_INDICATOR);
    lv_obj_add_flag(arc, LV_OBJ_FLAG_HIDDEN);
    t_lbl = lv_label_create(scr); lv_obj_set_style_text_font(t_lbl, &lv_font_montserrat_48, 0); lv_obj_set_style_text_color(t_lbl, lv_color_hex(0xE0801C), 0); lv_obj_align(t_lbl, LV_ALIGN_CENTER, 0, -34);
    s_lbl = lv_label_create(scr); lv_obj_set_style_text_font(s_lbl, &lv_font_montserrat_20, 0); lv_obj_set_style_text_color(s_lbl, lv_color_hex(0xE0801C), 0); lv_obj_align(s_lbl, LV_ALIGN_CENTER, 0, 28);
    n_lbl = lv_label_create(scr); lv_obj_set_style_text_font(n_lbl, &lv_font_montserrat_20, 0); lv_obj_set_style_text_color(n_lbl, lv_color_hex(0xB27C4A), 0); lv_obj_align(n_lbl, LV_ALIGN_CENTER, 0, 64);
    lv_timer_create(tick, 250, NULL);
    bsp_display_unlock();
    bsp_display_brightness_set(15);
}
