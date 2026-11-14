#include "gfx.h"
#include "icons.h"
#include "fonts/fonts.h"
#include <math.h>
#include <stdio.h>
#include <string.h>
#include "esp_heap_caps.h"

static lv_layer_t *L; static int OX, OY;
/* ---- drawing goes through a recorded list ----
 * ui_draw() runs once per change in REC mode: every primitive is stored with its bounding box and hashed into the 26-row bands it touches.
 * Only bands whose hash changed are drawn (replayed) into a small internal-RAM buffer and sent to the panel.
 * FULL mode draws straight to a layer (the screenshot canvas); PLAIN mode (no layer) only records the layout-check boxes. */
enum { M_PLAIN, M_FULL, M_REC };
enum { T_RECT, T_ARC, T_LINE, T_LABEL, T_TRI, T_IMG };
typedef struct {
    uint8_t type; lv_area_t bb, area; char text[40];
    union { lv_draw_rect_dsc_t r; lv_draw_arc_dsc_t a; lv_draw_line_dsc_t l; lv_draw_label_dsc_t t; lv_draw_triangle_dsc_t g; lv_draw_image_dsc_t i; } d;
} rec_t;
#define MAXREC 700
static int mode; static rec_t *recs; static int nrec; static uint32_t hs[GFX_STRIPS], sent[GFX_STRIPS], extra_h;
static const size_t DSZ[] = {sizeof(lv_draw_rect_dsc_t), sizeof(lv_draw_arc_dsc_t), sizeof(lv_draw_line_dsc_t), sizeof(lv_draw_label_dsc_t), sizeof(lv_draw_triangle_dsc_t), sizeof(lv_draw_image_dsc_t)};
static uint32_t fnv(uint32_t h, const void *p, size_t n) { const uint8_t *b = p; while (n--) h = (h ^ *b++) * 16777619u; return h; }
static bool take(int type, const void *dsc, const lv_area_t *bb, const lv_area_t *area, const char *text) {
    if (mode == M_FULL) return true;
    if (mode != M_REC) return false;
    int s0 = bb->y1 < 0 ? 0 : bb->y1 / GFX_STRIP_H, s1 = (bb->y2 > 465 ? 465 : bb->y2) / GFX_STRIP_H;
    if (bb->y2 < 0 || bb->y1 > 465 || bb->x2 < 0 || bb->x1 > 465 || nrec >= MAXREC) return false;
    if (!recs) recs = heap_caps_malloc(sizeof(rec_t) * MAXREC, MALLOC_CAP_SPIRAM);
    if (!recs) return false;
    rec_t *r = &recs[nrec++]; r->type = type; r->bb = *bb; r->area = area ? *area : *bb; memcpy(&r->d, dsc, DSZ[type]); r->text[0] = 0;
    if (text) strlcpy(r->text, text, sizeof r->text);
    if (type == T_LABEL) r->d.t.text = NULL;
    uint32_t h = fnv(2166136261u, &r->d, DSZ[type]); h = fnv(h, &r->area, sizeof r->area); h = fnv(h, r->text, strlen(r->text)); h ^= type * 7919u + (type == T_IMG ? extra_h : 0);
    if (type == T_LABEL) r->d.t.text = r->text;
    for (int i = s0; i <= s1; i++) hs[i] = (hs[i] ^ h) * 16777619u;
    return false;
}
static lv_area_t bb_arc(const lv_draw_arc_dsc_t *d) { int r = d->radius + 2; return (lv_area_t){d->center.x - r, d->center.y - r, d->center.x + r, d->center.y + r}; }
static lv_area_t bb_line(const lv_draw_line_dsc_t *d) { int w = d->width / 2 + 2, x0 = (int)fminf(d->p1.x, d->p2.x), x1 = (int)fmaxf(d->p1.x, d->p2.x), y0 = (int)fminf(d->p1.y, d->p2.y), y1 = (int)fmaxf(d->p1.y, d->p2.y); return (lv_area_t){x0 - w, y0 - w, x1 + w, y1 + w}; }
static lv_area_t bb_tri(const lv_draw_triangle_dsc_t *d) { int x0 = 9999, y0 = 9999, x1 = -9999, y1 = -9999; for (int i = 0; i < 3; i++) { int x = (int)d->p[i].x, y = (int)d->p[i].y; if (x < x0) x0 = x; if (x > x1) x1 = x; if (y < y0) y0 = y; if (y > y1) y1 = y; } return (lv_area_t){x0 - 2, y0 - 2, x1 + 2, y1 + 2}; }
#define lv_draw_rect(l, d, a) do { if (take(T_RECT, d, a, a, NULL)) lv_draw_rect(l, d, a); } while (0)
#define lv_draw_arc(l, d) do { lv_area_t bb_ = bb_arc(d); if (take(T_ARC, d, &bb_, NULL, NULL)) lv_draw_arc(l, d); } while (0)
#define lv_draw_line(l, d) do { lv_area_t bb_ = bb_line(d); if (take(T_LINE, d, &bb_, NULL, NULL)) lv_draw_line(l, d); } while (0)
#define lv_draw_label(l, d, a) do { if (take(T_LABEL, d, a, a, (d)->text)) lv_draw_label(l, d, a); } while (0)
#define lv_draw_triangle(l, d) do { lv_area_t bb_ = bb_tri(d); if (take(T_TRI, d, &bb_, NULL, NULL)) lv_draw_triangle(l, d); } while (0)
#define lv_draw_image(l, d, a) do { if (take(T_IMG, d, a, a, NULL)) lv_draw_image(l, d, a); } while (0)
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
void gfx_begin(lv_layer_t *layer, int ox, int oy) { L = layer; OX = ox; OY = oy; mode = layer ? M_FULL : M_PLAIN; }
void gfx_record_begin(int ox, int oy) { L = NULL; OX = ox; OY = oy; mode = M_REC; nrec = 0; for (int i = 0; i < GFX_STRIPS; i++) hs[i] = 2166136261u; }
bool gfx_strip_dirty(int s) { return hs[s] != sent[s]; }
void gfx_strip_sent(int s) { sent[s] = hs[s]; }
void gfx_strips_forget(void) { memset(sent, 0, sizeof sent); }
/* replay a recorded list: the rows y0..y1 only (y0 < 0: everything) */
static rec_t *old; static int nold;
static void replay_list(rec_t *l, int n, lv_layer_t *layer, int y0, int y1) {
    int sm = mode; mode = M_FULL;
    for (int i = 0; i < n; i++) {
        rec_t *r = &l[i]; if (y0 >= 0 && (r->bb.y2 < y0 || r->bb.y1 > y1)) continue;
        switch (r->type) {
        case T_RECT: lv_draw_rect(layer, &r->d.r, &r->area); break;
        case T_ARC: lv_draw_arc(layer, &r->d.a); break;
        case T_LINE: lv_draw_line(layer, &r->d.l); break;
        case T_LABEL: lv_draw_label(layer, &r->d.t, &r->area); break;
        case T_TRI: lv_draw_triangle(layer, &r->d.g); break;
        case T_IMG: lv_draw_image(layer, &r->d.i, &r->area); break;
        }
    }
    mode = sm;
}
void gfx_replay(lv_layer_t *layer, int y0, int y1) { replay_list(recs, nrec, layer, y0, y1); }
void gfx_replay_old(lv_layer_t *layer, int y0, int y1) { replay_list(old, nold, layer, y0, y1); }
/* keep the picture on the glass as the "old" scene of a slide transition */
void gfx_hold_old(void) {
    if (!recs || !nrec) { nold = 0; return; }
    if (!old) old = heap_caps_malloc(sizeof(rec_t) * MAXREC, MALLOC_CAP_SPIRAM);
    if (!old) { nold = 0; return; }
    memcpy(old, recs, sizeof(rec_t) * nrec); nold = nrec;
    for (int i = 0; i < nold; i++) if (old[i].type == T_LABEL) old[i].d.t.text = old[i].text;
}
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
/* the glow is 48 big overlapping circles: far too slow to redraw each frame, so it is drawn once per colour into a cached image */
static void glow_draw(col_t bg) {
    gfx_fill_v(rgb(mulc(bg, .2f)), rgb(mulc(bg, .2f)));
    for (int i = 1; i <= 48; i++) { float k = i / 48.0f; gfx_circle(C, 186, 289 * (1 - k), rgb(mulc(bg, .2f + .8f * k)), 1); }
}
void gfx_glow(col_t bg) {                                    /* centre (233,186), radius 289: bg in the middle, a fifth of it at the edge */
    if (mode == M_PLAIN) return;
    static lv_obj_t *gc; static lv_draw_buf_t *gb; static uint32_t key = 0xFFFFFFFF; uint32_t k = rgb(bg);
    if (!gc) { gb = lv_draw_buf_create(466, 466, LV_COLOR_FORMAT_RGB565, 0); gc = lv_canvas_create(lv_screen_active()); lv_canvas_set_draw_buf(gc, gb); lv_obj_add_flag(gc, LV_OBJ_FLAG_HIDDEN); }
    if (k != key) {
        lv_layer_t *sl = L; int sx = OX, sy = OY, sm = mode; lv_layer_t gl; lv_canvas_init_layer(gc, &gl); L = &gl; OX = OY = 0; mode = M_FULL;
        glow_draw(bg); lv_canvas_finish_layer(gc, &gl); L = sl; OX = sx; OY = sy; mode = sm; key = k;
    }
    extra_h = k; lv_draw_image_dsc_t d; lv_draw_image_dsc_init(&d); d.src = gb; lv_area_t a = {0, 0, 465, 465}; lv_draw_image(L, &d, &a);
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
