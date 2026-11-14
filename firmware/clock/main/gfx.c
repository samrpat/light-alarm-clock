#include "gfx.h"
#include "icons.h"
#include "fonts/fonts.h"
#include <math.h>
#include <stdio.h>
#include <string.h>

static lv_layer_t *L; static int OX, OY;
/* with no layer (the layout check) nothing is drawn, only the boxes are recorded */
#define lv_draw_rect(l, ...) do { if (l) lv_draw_rect(l, __VA_ARGS__); } while (0)
#define lv_draw_arc(l, ...) do { if (l) lv_draw_arc(l, __VA_ARGS__); } while (0)
#define lv_draw_line(l, ...) do { if (l) lv_draw_line(l, __VA_ARGS__); } while (0)
#define lv_draw_label(l, ...) do { if (l) lv_draw_label(l, __VA_ARGS__); } while (0)
#define lv_draw_triangle(l, ...) do { if (l) lv_draw_triangle(l, __VA_ARGS__); } while (0)
#define OPA(o) ((lv_opa_t)((o) <= 0 ? 0 : (o) >= 1 ? 255 : (int)((o) * 255 + .5f)))

/* ---- boxes for the layout check ---- */
typedef struct { float x0, y0, x1, y1; uint8_t kind; uint8_t size; } box_t;      /* kind 0 text, 1 icon, 2 dot */
static box_t boxes[96]; static int nbox;
static void add_box(float x0, float y0, float x1, float y1, int kind, int size) { if (nbox < 96) boxes[nbox++] = (box_t){x0, y0, x1, y1, (uint8_t)kind, (uint8_t)size}; }
void gfx_check_reset(void) { nbox = 0; }
void gfx_dot_box(float x, float y, float r) { add_box(x - r, y - r, x + r, y + r, 2, 0); }
int gfx_check(char *msg, int n, bool line_limit) {
    int bad = 0; if (n) msg[0] = 0;
#define SAY(...) do { if (!bad++ && n) snprintf(msg, n, __VA_ARGS__); } while (0)
    for (int i = 0; i < nbox; i++) {
        box_t *a = &boxes[i];
        float cx[4] = {a->x0, a->x1, a->x0, a->x1}, cy[4] = {a->y0, a->y0, a->y1, a->y1};
        if (a->kind != 2) for (int k = 0; k < 4; k++) if (hypotf(cx[k] - C, cy[k] - C) > 209.5f) { SAY("box %d outside the safe circle (%.0f,%.0f)", i, cx[k], cy[k]); break; }
        if (a->kind == 0 && a->size < 26) SAY("text of %d px (under 26)", a->size);
        for (int j = i + 1; j < nbox; j++) {
            box_t *b = &boxes[j]; if (a->kind == 2 && b->kind == 2) continue;
            float p = 3; if (a->x0 - p < b->x1 && b->x0 - p < a->x1 && a->y0 - p < b->y1 && b->y0 - p < a->y1) SAY("boxes %d and %d overlap (%.0f,%.0f)-(%.0f,%.0f) with (%.0f,%.0f)-(%.0f,%.0f)", i, j, a->x0, a->y0, a->x1, a->y1, b->x0, b->y0, b->x1, b->y1);
        }
    }
    if (line_limit) {                                        /* at most four text lines */
        float ys[96]; int m = 0;
        for (int i = 0; i < nbox; i++) if (boxes[i].kind == 0) ys[m++] = (boxes[i].y0 + boxes[i].y1) / 2;
        for (int i = 1; i < m; i++) { float v = ys[i]; int j = i - 1; while (j >= 0 && ys[j] > v) { ys[j + 1] = ys[j]; j--; } ys[j + 1] = v; }
        int lines = m ? 1 : 0; for (int i = 1; i < m; i++) if (ys[i] - ys[i - 1] > 14) lines++;
        if (lines > 4) SAY("%d text lines (max 4)", lines);
    }
    return bad;
}

/* ---- primitives ---- */
void gfx_begin(lv_layer_t *layer, int ox, int oy) { L = layer; OX = ox; OY = oy; }
void gfx_pt(float a, float r, float *x, float *y) { float t = a * 0.01745329f; *x = C + r * sinf(t); *y = C - r * cosf(t); }
static void area(lv_area_t *a, float x, float y, float w, float h) { a->x1 = (int)lroundf(x) + OX; a->y1 = (int)lroundf(y) + OY; a->x2 = a->x1 + (int)lroundf(w) - 1; a->y2 = a->y1 + (int)lroundf(h) - 1; }

void gfx_fill_v(uint32_t top, uint32_t bot) {
    lv_draw_rect_dsc_t d; lv_draw_rect_dsc_init(&d); d.bg_opa = 255; d.bg_color = lv_color_hex(top);
    d.bg_grad.dir = LV_GRAD_DIR_VER; d.bg_grad.stops_count = 2;
    d.bg_grad.stops[0] = (lv_grad_stop_t){.color = lv_color_hex(top), .opa = 255, .frac = 0}; d.bg_grad.stops[1] = (lv_grad_stop_t){.color = lv_color_hex(bot), .opa = 255, .frac = 255};
    lv_area_t a = {0, 0, 465, 465}; lv_draw_rect(L, &d, &a);
}
void gfx_fill_v3(uint32_t top, uint32_t mid, uint32_t bot, float opa) {
    for (int k = 0; k < 2; k++) {
        lv_draw_rect_dsc_t d; lv_draw_rect_dsc_init(&d); d.bg_opa = OPA(opa); uint32_t c0 = k ? mid : top, c1 = k ? bot : mid; d.bg_color = lv_color_hex(c0);
        d.bg_grad.dir = LV_GRAD_DIR_VER; d.bg_grad.stops_count = 2;
        d.bg_grad.stops[0] = (lv_grad_stop_t){.color = lv_color_hex(c0), .opa = OPA(opa), .frac = 0}; d.bg_grad.stops[1] = (lv_grad_stop_t){.color = lv_color_hex(c1), .opa = OPA(opa), .frac = 255};
        lv_area_t a = {0, k ? 256 : 0, 465, k ? 465 : 255}; lv_draw_rect(L, &d, &a);
    }
}
void gfx_glow(col_t bg) {                                    /* centre (233,186), radius 289: bg in the middle, a fifth of it at the edge */
    gfx_fill_v(rgb(mulc(bg, .2f)), rgb(mulc(bg, .2f)));
    for (int i = 1; i <= 48; i++) { float k = i / 48.0f; gfx_circle(C, 186, 289 * (1 - k), rgb(mulc(bg, .2f + .8f * k)), 1); }
}
void gfx_circle(float cx, float cy, float r, uint32_t col, float opa) {
    if (r <= 0.3f) return;
    lv_draw_rect_dsc_t d; lv_draw_rect_dsc_init(&d); d.bg_opa = OPA(opa); d.bg_color = lv_color_hex(col); d.radius = LV_RADIUS_CIRCLE;
    lv_area_t a; area(&a, cx - r, cy - r, 2 * r, 2 * r); lv_draw_rect(L, &d, &a);
}
void gfx_circle_g(float cx, float cy, float r, uint32_t ca, uint32_t cb) {
    lv_draw_rect_dsc_t d; lv_draw_rect_dsc_init(&d); d.bg_opa = 255; d.bg_color = lv_color_hex(ca); d.radius = LV_RADIUS_CIRCLE;
    d.bg_grad.dir = LV_GRAD_DIR_VER; d.bg_grad.stops_count = 2;
    d.bg_grad.stops[0] = (lv_grad_stop_t){.color = lv_color_hex(ca), .opa = 255, .frac = 0}; d.bg_grad.stops[1] = (lv_grad_stop_t){.color = lv_color_hex(cb), .opa = 255, .frac = 255};
    lv_area_t a; area(&a, cx - r, cy - r, 2 * r, 2 * r); lv_draw_rect(L, &d, &a);
}
void gfx_arc(float cx, float cy, float r, float a0, float a1, uint32_t col, float w, float opa, bool round) {
    if (a1 - a0 <= 0.1f) return;
    lv_draw_arc_dsc_t d; lv_draw_arc_dsc_init(&d); d.color = lv_color_hex(col); d.width = (int)lroundf(w); d.opa = OPA(opa); d.rounded = round;
    d.center.x = (int)lroundf(cx) + OX; d.center.y = (int)lroundf(cy) + OY; d.radius = (uint16_t)lroundf(r + w / 2);
    if (a1 - a0 >= 359.9f) { d.start_angle = 0; d.end_angle = 360; d.rounded = 0; }
    else { float s = fmodf(a0 - 90, 360); if (s < 0) s += 360; d.start_angle = s; d.end_angle = s + (a1 - a0); }
    lv_draw_arc(L, &d);
}
void gfx_ring(float cx, float cy, float r, uint32_t col, float w, float opa) { gfx_arc(cx, cy, r, 0, 360, col, w, opa, false); }
void gfx_line(float x0, float y0, float x1, float y1, uint32_t col, float w, float opa) {
    lv_draw_line_dsc_t d; lv_draw_line_dsc_init(&d); d.color = lv_color_hex(col); d.width = (int)lroundf(w); if (d.width < 1) d.width = 1; d.opa = OPA(opa); d.round_start = d.round_end = 1;
    d.p1.x = x0 + OX; d.p1.y = y0 + OY; d.p2.x = x1 + OX; d.p2.y = y1 + OY; lv_draw_line(L, &d);
}
void gfx_pill(float x, float y, float w, float h, uint32_t col, float opa) {
    lv_draw_rect_dsc_t d; lv_draw_rect_dsc_init(&d); d.bg_opa = OPA(opa); d.bg_color = lv_color_hex(col); d.radius = (int)(h / 2);
    lv_area_t a; area(&a, x, y, w, h); lv_draw_rect(L, &d, &a);
}
void gfx_pill_g(float x, float y, float w, float h, uint32_t ca, uint32_t cb) {
    lv_draw_rect_dsc_t d; lv_draw_rect_dsc_init(&d); d.bg_opa = 255; d.bg_color = lv_color_hex(ca); d.radius = (int)(h / 2);
    d.bg_grad.dir = LV_GRAD_DIR_HOR; d.bg_grad.stops_count = 2;
    d.bg_grad.stops[0] = (lv_grad_stop_t){.color = lv_color_hex(ca), .opa = 255, .frac = 0}; d.bg_grad.stops[1] = (lv_grad_stop_t){.color = lv_color_hex(cb), .opa = 255, .frac = 255};
    lv_area_t a; area(&a, x, y, w, h); lv_draw_rect(L, &d, &a);
}
void gfx_tri(float x0, float y0, float x1, float y1, float x2, float y2, uint32_t col) {
    lv_draw_triangle_dsc_t d; lv_draw_triangle_dsc_init(&d); d.color = lv_color_hex(col); d.opa = 255;
    d.p[0].x = x0 + OX; d.p[0].y = y0 + OY; d.p[1].x = x1 + OX; d.p[1].y = y1 + OY; d.p[2].x = x2 + OX; d.p[2].y = y2 + OY; lv_draw_triangle(L, &d);
}
void gfx_icon(int id, float x, float y, uint32_t col, float sc, float opa, bool counted) {
    if (id < 0 || id >= ICO_COUNT) return;
    const ico_t *ic = &ICONS[id]; float w = 3 + 0.6f * sc, xmin = 1e9f, ymin = 1e9f, xmax = -1e9f, ymax = -1e9f;
    for (int s = 0; s < ic->n; s++) {
        const float *p = ic->s[s].xy;
        for (int i = 0; i < ic->s[s].n; i++) {
            float px = x + p[2 * i] * sc, py = y + p[2 * i + 1] * sc;
            xmin = px < xmin ? px : xmin; xmax = px > xmax ? px : xmax; ymin = py < ymin ? py : ymin; ymax = py > ymax ? py : ymax;
            if (i) gfx_line(x + p[2 * i - 2] * sc, y + p[2 * i - 1] * sc, px, py, col, w, opa);
        }
    }
    if (counted) add_box(xmin - w / 2, ymin - w / 2, xmax + w / 2, ymax + w / 2, 1, 0);
}
/* ---- text ---- */
static const struct { int size, weight; const lv_font_t *f; } FT[] = {
    {26, 400, &inter_r26}, {26, 500, &inter_m26}, {28, 400, &inter_r28}, {28, 500, &inter_m28}, {28, 300, &inter_l28}, {30, 500, &inter_m30},
    {34, 400, &inter_r34}, {34, 500, &inter_m34}, {38, 400, &inter_r38}, {38, 500, &inter_m38}, {38, 300, &inter_l38}, {40, 500, &inter_m40},
    {44, 500, &inter_m44}, {44, 300, &inter_l44}, {46, 300, &inter_l46}, {56, 300, &inter_l56}, {66, 300, &inter_l66}, {72, 300, &inter_l72},
    {76, 300, &inter_l76}, {80, 300, &inter_l80}, {96, 300, &inter_l96}, {110, 300, &inter_l110}, {124, 300, &inter_l124}};
static const lv_font_t *font_of(int size, int weight) {
    const lv_font_t *best = FT[0].f; int bd = 1 << 30;
    for (unsigned i = 0; i < sizeof FT / sizeof FT[0]; i++) { int d = abs(FT[i].size - size) * 1000 + abs(FT[i].weight - weight); if (d < bd) { bd = d; best = FT[i].f; } }
    return best;
}
float gfx_text_w(int size, int weight, const char *t) { lv_point_t sz; lv_text_get_size(&sz, t, font_of(size, weight), 0, 0, LV_COORD_MAX, LV_TEXT_FLAG_NONE); return (float)sz.x; }
static void text_at(float left, float baseline, const lv_font_t *f, const char *t, uint32_t col, float opa, float w) {
    lv_draw_label_dsc_t d; lv_draw_label_dsc_init(&d); d.text = t; d.text_local = 1; d.font = f; d.color = lv_color_hex(col); d.opa = OPA(opa);
    lv_area_t a; a.x1 = (int)lroundf(left) + OX; a.y1 = (int)lroundf(baseline) - (f->line_height - f->base_line) + OY; a.x2 = a.x1 + (int)w + 4; a.y2 = a.y1 + f->line_height;
    lv_draw_label(L, &d, &a);
}
void gfx_text(float x, float y, int size, int weight, const char *t, uint32_t col, float opa) {
    const lv_font_t *f = font_of(size, weight); float w = gfx_text_w(size, weight, t), base = y + 0.35f * size;
    text_at(x - w / 2, base, f, t, col, opa, w);
    add_box(x - w / 2, base - 0.73f * size, x + w / 2, base + 0.02f * size, 0, size);
}
void gfx_time(float x, float y, int size, int m, uint32_t col, int ap) {
    m = ((m % 1440) + 1440) % 1440; int h = m / 60; char t[8]; snprintf(t, sizeof t, "%d:%02d", (h + 11) % 12 + 1, m % 60); const char *s = h < 12 ? "AM" : "PM";
    const lv_font_t *f = font_of(size, 300), *fa = font_of(ap, 300);
    float w = gfx_text_w(size, 300, t), gap = (float)(size / 14), wa = gfx_text_w(ap, 300, s), tot = w + gap + wa, base = y + 0.35f * size;
    text_at(x - tot / 2, base, f, t, col, 1, w); text_at(x - tot / 2 + w + gap, base, fa, s, col, 1, wa);
    add_box(x - tot / 2, base - 0.73f * size, x + tot / 2, base + 0.02f * size, 0, size);
}
