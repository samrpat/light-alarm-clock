/* The dawn light curve (LIGHT.md section 3, D-033) and the pre-dawn sleep sound (D-044). Pure maths. */
#pragma once
#include <stdint.h>
#include <stdbool.h>

typedef struct { float r, g, b, w; } rgbw_f;            /* 0..1 of the per-channel maximum */
float dawn_F(int32_t t_s);                              /* overall fraction, t relative to the alarm (-1800..+600) */
rgbw_f dawn_rgbw(int32_t t_s);

/* Sleep sound before the dawn: timer of n_min minutes (0 = all night) played at bedtime plays again for
 * n_min minutes ending at the dawn start. Returns a gain 0..1. t_s relative to the alarm time. */
float presound_gain(int32_t t_s, int n_min);
