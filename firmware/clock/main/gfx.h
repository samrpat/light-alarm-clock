/* A small drawing layer over LVGL's draw calls, shaped like the concept page's SVG helpers so each screen can be
 * carried over line by line (UX.md). Angles: 0 = 12 o'clock, clockwise. Colours: 0xRRGGBB. Opacity 0..1.
 * Every text and icon box is recorded so the layout rules can be checked on the device itself (R37). */
#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "lvgl.h"

#define C 233
typedef struct { float r, g, b; } col_t;
static inline uint32_t rgb(col_t c) {
    int r = (int)(c.r + .5f), g = (int)(c.g + .5f), b = (int)(c.b + .5f);
    r = r < 0 ? 0 : r > 255 ? 255 : r; g = g < 0 ? 0 : g > 255 ? 255 : g; b = b < 0 ? 0 : b > 255 ? 255 : b;
    return (uint32_t)(r << 16 | g << 8 | b);
}
static inline col_t mixc(col_t a, col_t b, float k) { col_t o = {a.r + (b.r - a.r) * k, a.g + (b.g - a.g) * k, a.b + (b.b - a.b) * k}; return o; }
static inline col_t mulc(col_t a, float k) { col_t o = {a.r * k, a.g * k, a.b * k}; return o; }
static inline col_t hexc(uint32_t h) { col_t o = {(float)(h >> 16 & 255), (float)(h >> 8 & 255), (float)(h & 255)}; return o; }

#define GFX_STRIP_H 26     /* must be even: the panel takes only even row starts and counts */
#define GFX_STRIPS 18
void gfx_record_begin(int ox, int oy);          /* ui_draw() now only records and hashes */
bool gfx_strip_dirty(int s);
void gfx_strip_sent(int s);
void gfx_strips_forget(void);
void gfx_replay_old(lv_layer_t *layer, int y0, int y1);
void gfx_hold_old(void);
void gfx_replay(lv_layer_t *layer, int y0, int y1);   /* y0 < 0: everything */
void gfx_begin(lv_layer_t *layer, int ox, int oy);         /* ox, oy: the small burn-in shift */
void gfx_pt(float ang, float r, float *x, float *y);        /* point on a circle round the centre */
void gfx_fill_v(uint32_t top, uint32_t bottom);             /* whole screen, vertical gradient */
void gfx_fill_v3(uint32_t top, uint32_t mid, uint32_t bottom, float opa);
void gfx_glow(col_t bg);                                    /* the soft radial background of a themed screen */
void gfx_circle(float cx, float cy, float r, uint32_t col, float opa);
void gfx_circle_g(float cx, float cy, float r, uint32_t a, uint32_t b);   /* glossy two-colour fill */
void gfx_ring(float cx, float cy, float r, uint32_t col, float w, float opa);
void gfx_arc(float cx, float cy, float r, float a0, float a1, uint32_t col, float w, float opa, bool round);
void gfx_line(float x0, float y0, float x1, float y1, uint32_t col, float w, float opa);
void gfx_pill(float x, float y, float w, float h, uint32_t col, float opa);
void gfx_pill_g(float x, float y, float w, float h, uint32_t a, uint32_t b);
void gfx_tri(float x0, float y0, float x1, float y1, float x2, float y2, uint32_t col);
void gfx_icon(int id, float x, float y, uint32_t col, float scale, float opa, bool counted);
float gfx_text_w(int size, int weight, const char *t);
void gfx_text(float x, float y, int size, int weight, const char *t, uint32_t col, float opa);      /* centred on x, optical middle on y */
void gfx_time(float x, float y, int size, int minutes, uint32_t col, int ap_size);                   /* "6:45" + small AM/PM */
void gfx_dot_box(float x, float y, float r);                /* a draggable dot, for the layout check */

/* layout check (R37, UX.md section 11) */
void gfx_check_reset(void);
int  gfx_check(char *msg, int n, bool line_limit);          /* number of problems; the first one described in msg */
