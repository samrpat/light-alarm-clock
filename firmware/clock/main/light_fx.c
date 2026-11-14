/* light_fx: core 1, priority 17, 250 frames a second. Reads the alarm snapshot and drives the ring (D-012, D-014).
 * The colour maths is the tested core (dawn_rgbw); this file dithers, eases, fades and caps the current. */
#include <math.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "esp_task_wdt.h"
#include "led_strip.h"
#include "hw.h"
#include "dawn.h"

static const char *TAG = "light";
#define RING_GPIO 17
#define RING_LEDS 24
#define MA_PER_CH 18
#define IDLE_MA 25
/* Hard current cap (R14). 900 mA only with the fuse and capacitor of POWER.md fitted; the bare test rig
 * must stay at 300 mA. Change here, nowhere else. */
#ifndef RING_CAP_MA
#define RING_CAP_MA 300
#endif
static led_strip_handle_t ring;
rgbw_f g_lamp;

/* spread the fraction across the 24 LEDs and across 4 frames (about 96 levels between whole steps) */
static void fill(rgbw_f c) {
    static uint32_t frame; float ch[4] = {c.r * 255.f, c.g * 255.f, c.b * 255.f, c.w * 255.f};
    /* Floor: while red is wanted at all it never drops under one whole step, so every LED is lit from the first
     * moment (the curve's 0.2 % start is half a step, which would light only half the ring). */
    if (c.r > 0.0015f && ch[0] < 1.0f) ch[0] = 1.0f;
    float ma = IDLE_MA + (ch[0] + ch[1] + ch[2] + ch[3]) * RING_LEDS * MA_PER_CH / 255.0f;
    if (ma > RING_CAP_MA) { float k = (RING_CAP_MA - IDLE_MA) / (ma - IDLE_MA); for (int i = 0; i < 4; i++) ch[i] *= k; }
    float tphase = (frame++ & 3) * 0.25f / RING_LEDS;
    for (int i = 0; i < RING_LEDS; i++) {
        float thr = fmodf(i * 0.6180340f, 1.0f) + tphase; if (thr >= 1.0f) thr -= 1.0f;
        uint8_t o[4];
        for (int k = 0; k < 4; k++) { float t = thr + k * 0.25f; if (t >= 1.0f) t -= 1.0f; float v = ch[k] + t; o[k] = v >= 255.0f ? 255 : (uint8_t)v; }
        led_strip_set_pixel_rgbw(ring, i, o[0], o[1], o[2], o[3]);
    }
    led_strip_refresh(ring);
}
static rgbw_f mixc(rgbw_f a, rgbw_f b, float k) { rgbw_f o = {a.r + (b.r - a.r) * k, a.g + (b.g - a.g) * k, a.b + (b.b - a.b) * k, a.w + (b.w - a.w) * k}; return o; }
static rgbw_f scale(rgbw_f a, float k) { rgbw_f o = {a.r * k, a.g * k, a.b * k, a.w * k}; return o; }

static void task(void *arg) {
    (void)arg; esp_task_wdt_add(NULL);
    rgbw_f cur = {0}, from = {0}; bool was_on = false; int64_t off_t0 = 0; const rgbw_f zero = {0};
    for (;;) {
        alarm_snapshot_t s; alarm_snapshot_get(&s); int64_t now = esp_timer_get_time();
        bool test = now < g_test_until_us;
        if (s.light_on) { cur = mixc(cur, dawn_rgbw(s.dawn_t_s), 0.02f); was_on = true; off_t0 = 0; }            /* eased: no step is ever seen */
        else if (test) { cur = mixc(cur, scale(dawn_rgbw(0), 0.5f), 0.03f); was_on = false; off_t0 = 0; }
        else {
            if (was_on) { was_on = false; from = cur; off_t0 = now; }                               /* alarm ended: fade over 60 s (LIGHT.md) */
            if (off_t0) { float u = (now - off_t0) / 60e6f; if (u >= 1) { cur = zero; off_t0 = 0; } else cur = scale(from, (1 - u) * (1 - u)); }
            else cur = mixc(cur, g_lamp, 0.02f);                       /* no alarm: the Lamp screen owns the ring */
        }
        if (ring) fill(g_bench_mute ? zero : cur);
        esp_task_wdt_reset(); vTaskDelay(pdMS_TO_TICKS(4));
    }
}
void light_fx_start(void) {
    led_strip_config_t cfg = {.strip_gpio_num = RING_GPIO, .max_leds = RING_LEDS, .led_model = LED_MODEL_SK6812, .color_component_format = LED_STRIP_COLOR_COMPONENT_FMT_GRBW};
    led_strip_rmt_config_t rmt = {.clk_src = RMT_CLK_SRC_DEFAULT, .resolution_hz = 10 * 1000 * 1000};
    esp_err_t e = led_strip_new_rmt_device(&cfg, &rmt, &ring);
    ESP_LOGI(TAG, "ring: %s (the clock runs without it, R36)", esp_err_to_name(e));
    if (e != ESP_OK) ring = NULL; else led_strip_clear(ring);
    xTaskCreatePinnedToCore(task, "light_fx", 4096, NULL, 17, NULL, 1);
}
