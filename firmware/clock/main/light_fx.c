/* light_fx: core 1, priority 17, 50 Hz. Reads the alarm snapshot and drives the ring (D-012, D-014).
 * The colour maths is the tested core (dawn_rgbw); this file only dithers and caps the current. */
#include <math.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
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

/* spread the fraction across the 24 LEDs and across 4 frames (about 96 levels between whole steps) */
static void fill(rgbw_f c) {
    static uint32_t frame; float ch[4] = {c.r * 255.f, c.g * 255.f, c.b * 255.f, c.w * 255.f};
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
static void task(void *arg) {
    (void)arg; esp_task_wdt_add(NULL);
    for (;;) {
        alarm_snapshot_t s; do { s.seq = g_alarm.seq; s = g_alarm; } while ((s.seq & 1) || s.seq != g_alarm.seq);   /* lock-free read */
        if (ring) fill(s.light_on ? dawn_rgbw(s.dawn_t_s) : (rgbw_f){0, 0, 0, 0});
        esp_task_wdt_reset(); vTaskDelay(pdMS_TO_TICKS(20));
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
