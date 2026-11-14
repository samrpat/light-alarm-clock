/* Sleep sounds, made from maths (SOUND.md, D-040, D-041). One sample at a time, 22 050 Hz, output about -1..1.
 * Order matches the Sound screen: 0 pink 1 white 2 brown 3 green 4 grey 5 blue 6 violet 7 fan
 * 8 rain 9 storm 10 ocean 11 stream 12 waterfall 13 wind 14 forest 15 fire. To be tuned by ear. */
#include <math.h>
#include <stdint.h>
#include "hw.h"
#define FS 22050.0f
static uint32_t rs = 0x1234abcd;
static inline uint32_t rnd(void) { rs ^= rs << 13; rs ^= rs >> 17; rs ^= rs << 5; return rs; }
static inline float wn(void) { return (int32_t)rnd() * (1.0f / 2147483648.0f); }
static inline float uni(void) { return (rnd() >> 8) * (1.0f / 16777216.0f); }
static float pb0, pb1, pb2, br, lp1, lp2, hp1, prevw, prevp, sv_l, sv_b, drop, crk, th_env, wander = 0.5f, hum_ph;
static uint32_t t, th_next = 22050 * 8;
static struct { float ph, f, df, env; } bub[6];
static inline float pink(float w) { pb0 = 0.99765f * pb0 + w * 0.0990460f; pb1 = 0.96300f * pb1 + w * 0.2965164f; pb2 = 0.57000f * pb2 + w * 1.0526913f; return (pb0 + pb1 + pb2 + w * 0.1848f) * 0.22f; }
static inline float brown(float w) { br = (br + 0.02f * w) / 1.02f; return br * 3.5f; }
static inline float svf(float x, float f, float q, float *band) { float k = 2 * sinf(3.14159f * f / FS); sv_l += k * sv_b; float h = x - sv_l - q * sv_b; sv_b += k * h; *band = sv_b; return sv_l; }
static inline float rainy(float w) { hp1 += 0.23f * (w - hp1); float h = w - hp1;                      /* above about 900 Hz */
    if (uni() < 0.012f) { float a = uni(); if (a * a > drop) drop = a * a; } drop *= 0.992f; return h * (0.22f + 0.5f * drop); }
float sleep_sample(int id) {
    float w = wn(), o = 0, b; t++;
    switch (id) {
    case 0: o = pink(w); break;
    case 1: o = w * 0.45f; break;
    case 2: o = brown(w); break;
    case 3: svf(w, 500, 1.2f, &b); o = b * 0.9f; break;
    case 4: svf(w, 3000, 1.0f, &b); o = (w - 0.75f * b) * 0.5f + brown(w) * 0.4f; break;
    case 5: { float p = pink(w); o = (p - prevp) * 2.2f; prevp = p; break; }
    case 6: o = (w - prevw) * 0.35f; prevw = w; break;
    case 7: lp1 += 0.06f * (brown(w) - lp1); hum_ph += 110.0f / FS; if (hum_ph > 1) hum_ph -= 1; o = lp1 * 1.6f + 0.05f * sinf(6.28318f * hum_ph) + pink(w) * 0.15f; break;
    case 8: o = rainy(w); break;
    case 9: o = rainy(w);
        if (t >= th_next) { th_env = 0.0001f; th_next = t + (uint32_t)((12 + 14 * uni()) * FS); }
        if (th_env > 0) { static float up = 1; if (up > 0) { th_env *= 1.0009f; if (th_env > 0.9f) up = 0; } else { th_env *= 0.99994f; if (th_env < 0.001f) { th_env = 0; up = 1; } }
            lp2 += 0.012f * (brown(w) - lp2); o += lp2 * 5.0f * th_env * (1 + 0.5f * sinf(t * (6.28318f * 7 / FS))); }
        break;
    case 10: { float s = sinf(t * (3.14159f / (9 * FS))); s *= s; o = brown(w) * (0.25f + 0.75f * s) + pink(w) * 0.5f * s * s; break; }
    case 11: o = pink(w) * 0.18f;
        if (uni() < 55.0f / FS) { int k = rnd() % 6; bub[k].ph = 0; bub[k].f = 400 + 1400 * uni(); bub[k].df = bub[k].f * 0.6f / (0.03f * FS); bub[k].env = 0.12f + 0.2f * uni(); }
        for (int k = 0; k < 6; k++) if (bub[k].env > 0.001f) { bub[k].ph += bub[k].f / FS; if (bub[k].ph > 1) bub[k].ph -= 1; bub[k].f += bub[k].df; bub[k].env *= 0.9975f; o += sinf(6.28318f * bub[k].ph) * bub[k].env; }
        break;
    case 12: o = pink(w) * 0.6f + brown(w) * 0.6f; break;
    case 13: wander += (uni() - 0.5f) * 0.0006f; wander += (0.5f - wander) * 0.00001f; if (wander < 0.05f) wander = 0.05f; if (wander > 0.95f) wander = 0.95f;
        svf(pink(w), 180 + 700 * wander, 0.8f, &b); o = b * (0.6f + 1.2f * wander); break;
    case 14: { hp1 += 0.5f * (w - hp1); float gust = 0.6f + 0.4f * sinf(t * (6.28318f / (11 * FS)));
        float burst = sinf(t * (6.28318f / (3.7f * FS))); burst = burst > 0.6f ? (burst - 0.6f) * 2.5f : 0;
        float fl = 0.5f + 0.5f * sinf(t * (6.28318f * 25 / FS));
        o = (w - hp1) * 0.12f * gust + pink(w) * 0.12f + burst * fl * 0.05f * (sinf(t * (6.28318f * 3900 / FS)) + sinf(t * (6.28318f * 4300 / FS))); break; }
    case 15: lp1 += 0.03f * (brown(w) - lp1); o = lp1 * 1.2f;
        if (uni() < (crk > 0.05f ? 40.0f : 6.0f) / FS) { float a = uni(); crk = 0.15f + 0.85f * a * a * a; } crk *= 0.985f; o += w * crk * 0.8f; break;
    }
    static float dc; dc += 0.042f * (o - dc); o -= dc;                                                 /* 150 Hz high-pass: protects the small speaker */
    return o > 1 ? 1 : o < -1 ? -1 : o;
}
