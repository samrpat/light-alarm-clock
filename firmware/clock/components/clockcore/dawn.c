#include "dawn.h"
#include <math.h>

static float lerp(float a, float b, float k) { return a + (b - a) * k; }
float dawn_F(int32_t t) {
    if (t <= -1800) return 0.0f;
    if (t >= 0) return 1.0f;
    float x = (t + 1800) / 1800.0f;
    return powf(0.002f, 1.0f - x);
}
rgbw_f dawn_rgbw(int32_t t) {
    float F = dawn_F(t), r, g, w, b = 0;
    if (t <= -900) { r = 1; g = 0; w = 0; }
    else if (t <= -300) { float k = (t + 900) / 600.0f; r = 1; g = .40f * k; w = .15f * k; }
    else if (t <= 0)    { float k = (t + 300) / 300.0f; r = lerp(1, .30f, k); g = lerp(.40f, .15f, k); w = lerp(.15f, 1, k); }
    else { float k = (t > 600 ? 600 : t) / 600.0f; r = .30f * (1 - k); g = lerp(.15f, .10f, k); w = 1; b = .25f * k; }
    rgbw_f o = {r * F, g * F, b * F, w * F};
    return o;
}
static float smooth(float x) { x = x < 0 ? 0 : x > 1 ? 1 : x; return x * x * (3 - 2 * x); }
float presound_gain(int32_t t, int n) {
    if (n <= 0) return 0.0f;                                   /* all night: no pre-window */
    float tm = t / 60.0f, start = -30.0f - n;
    if (tm < start) return 0.0f;
    float fin = smooth((tm - start) / (n < 10 ? (float)n : 10.0f));
    float fout = 1.0f - smooth((tm + 30.0f) / 3.0f);
    return fin < fout ? fin : fout;
}
