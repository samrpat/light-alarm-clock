/*
 * Hardware bring-up test for the sunrise alarm clock.
 * NOT the product firmware (that starts in Phase 3). It only proves the parts work:
 *   screen + touch, the RGBW ring on GPIO17, the speaker, and the chips on the I2C bus.
 *
 * Tap the screen to step through the tests:
 *   Idle -> Sunrise (20 s loop) -> White light -> White noise -> Tone 440 Hz -> Idle
 *
 * Safety: the ring is held to RING_CAP_MA (300 mA) because this test rig has no fuse.
 */
#include <math.h>
#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_random.h"
#include "esp_timer.h"
#include "driver/i2c_master.h"
#include "bsp/esp-bsp.h"
#include "bsp/display.h"
#include "esp_codec_dev.h"
#include "led_strip.h"
#include "lvgl.h"

static const char *TAG = "bringup";

#define RING_GPIO     17      /* header pin 6, see docs/HARDWARE.md */
#define RING_LEDS     24
#define RING_CAP_MA   300     /* firmware current cap for this un-fused test */
#define MA_PER_CH     18      /* Adafruit: ~18 mA per colour channel at full */
#define RING_IDLE_MA  25
#define SAMPLE_RATE   22050

enum { MODE_IDLE, MODE_SUNRISE, MODE_WHITE, MODE_NOISE, MODE_TONE, MODE_COUNT };
static const char *MODE_NAME[MODE_COUNT] = {"Idle", "Sunrise", "White light", "White noise", "Tone 440 Hz"};
static volatile int s_mode = MODE_IDLE;
static volatile int64_t s_mode_t0;

static led_strip_handle_t s_ring;
static esp_codec_dev_handle_t s_spk;
static lv_obj_t *s_time_lbl, *s_mode_lbl, *s_info_lbl;
static volatile int s_ring_ma;

/* ---------- ring ---------- */
typedef struct { uint8_t r, g, b, w; } rgbw_t;

/* Fill the whole ring with one colour, scaled down if it would exceed the current cap. */
static void ring_fill(rgbw_t c)
{
    if (!s_ring) return;
    int sum = (c.r + c.g + c.b + c.w) * RING_LEDS;
    int ma = RING_IDLE_MA + sum * MA_PER_CH / 255;
    if (ma > RING_CAP_MA) {
        float k = (float)(RING_CAP_MA - RING_IDLE_MA) / (float)(ma - RING_IDLE_MA);
        c.r *= k; c.g *= k; c.b *= k; c.w *= k;
        ma = RING_CAP_MA;
    }
    s_ring_ma = ma;
    for (int i = 0; i < RING_LEDS; i++) led_strip_set_pixel_rgbw(s_ring, i, c.r, c.g, c.b, c.w);
    led_strip_refresh(s_ring);
}

/* p = 0..1 : dark -> deep red -> amber -> warm white (the W LED). */
static rgbw_t sunrise_colour(float p)
{
    float r = fminf(1.0f, p * 2.5f);
    float g = fmaxf(0.0f, p - 0.2f) * 0.5f;
    float w = fmaxf(0.0f, (p - 0.35f) / 0.65f);
    float bright = p * p;                       /* slow start, like a real dawn */
    rgbw_t c = { .r = 255 * r * fmaxf(bright, 0.02f), .g = 255 * g * bright, .b = 0, .w = 255 * w * w };
    return c;
}

static void ring_task(void *arg)
{
    while (1) {
        float t = (esp_timer_get_time() - s_mode_t0) / 1e6f;
        switch (s_mode) {
        case MODE_SUNRISE: ring_fill(sunrise_colour(fmodf(t, 20.0f) / 20.0f)); break;
        case MODE_WHITE:   ring_fill((rgbw_t){0, 0, 0, 255}); break;
        default:           ring_fill((rgbw_t){0, 0, 0, 0}); break;
        }
        vTaskDelay(pdMS_TO_TICKS(20));
    }
}

static void ring_init(void)
{
    led_strip_config_t cfg = {
        .strip_gpio_num = RING_GPIO,
        .max_leds = RING_LEDS,
        .led_model = LED_MODEL_SK6812,
        .color_component_format = LED_STRIP_COLOR_COMPONENT_FMT_GRBW,
    };
    led_strip_rmt_config_t rmt = { .clk_src = RMT_CLK_SRC_DEFAULT, .resolution_hz = 10 * 1000 * 1000 };
    esp_err_t err = led_strip_new_rmt_device(&cfg, &rmt, &s_ring);
    ESP_LOGI(TAG, "ring on GPIO%d: %s", RING_GPIO, esp_err_to_name(err));
    if (err == ESP_OK) led_strip_clear(s_ring);
}

/* ---------- audio ---------- */
static void audio_task(void *arg)
{
    static int16_t buf[512];
    float phase = 0;
    while (1) {
        int mode = s_mode;
        for (int i = 0; i < 512; i++) {
            if (mode == MODE_NOISE) buf[i] = (int16_t)((int32_t)(esp_random() & 0xFFFF) - 32768) / 8;
            else if (mode == MODE_TONE) { buf[i] = (int16_t)(sinf(phase) * 6000); phase += 2 * M_PI * 440 / SAMPLE_RATE; if (phase > 2 * M_PI) phase -= 2 * M_PI; }
            else buf[i] = 0;
        }
        if (s_spk) esp_codec_dev_write(s_spk, buf, sizeof(buf));
        else vTaskDelay(pdMS_TO_TICKS(20));
    }
}

static void audio_init(void)
{
    s_spk = bsp_audio_codec_speaker_init();
    if (!s_spk) { ESP_LOGE(TAG, "speaker init failed"); return; }
    esp_codec_dev_sample_info_t fs = { .sample_rate = SAMPLE_RATE, .channel = 1, .bits_per_sample = 16 };
    esp_err_t err = esp_codec_dev_open(s_spk, &fs);
    esp_codec_dev_set_out_vol(s_spk, 60);
    ESP_LOGI(TAG, "speaker: %s", err == ESP_OK ? "ok" : "open failed");
}

/* ---------- I2C scan + power chip ---------- */
static i2c_master_dev_handle_t s_axp;

static int axp_read16(uint8_t reg)
{
    uint8_t d[2];
    if (!s_axp || i2c_master_transmit_receive(s_axp, &reg, 1, d, 2, 100) != ESP_OK) return -1;
    return ((d[0] & 0x3F) << 8) | d[1];         /* 14-bit result, 1 mV per step */
}

static void i2c_report(void)
{
    static const struct { uint8_t addr; const char *name; } dev[] = {
        {0x18, "ES8311 codec"}, {0x20, "TCA9554 expander"}, {0x34, "AXP2101 power"}, {0x40, "ES7210 mic ADC"},
        {0x51, "PCF85063 RTC"}, {0x5A, "CST9217 touch"}, {0x6B, "QMI8658 IMU"},
    };
    i2c_master_bus_handle_t bus = bsp_i2c_get_handle();
    for (int i = 0; i < 7; i++)
        ESP_LOGI(TAG, "I2C 0x%02X %-17s %s", dev[i].addr, dev[i].name,
                 i2c_master_probe(bus, dev[i].addr, 100) == ESP_OK ? "found" : "MISSING");
    i2c_device_config_t c = { .dev_addr_length = I2C_ADDR_BIT_LEN_7, .device_address = 0x34, .scl_speed_hz = 100000 };
    if (i2c_master_bus_add_device(bus, &c, &s_axp) == ESP_OK) {
        uint8_t en[2] = {0x30, 0xFF};            /* switch on the chip's voltage measurements (unverified register) */
        i2c_master_transmit(s_axp, en, 2, 100);
    }
#ifdef BSP_I2S_MCLK
    ESP_LOGI(TAG, "BSP pins: I2S MCLK=%d BCLK=%d LRCK=%d DOUT=%d, amp enable=%d",
             BSP_I2S_MCLK, BSP_I2S_SCLK, BSP_I2S_LCLK, BSP_I2S_DOUT, BSP_POWER_AMP_IO);
#endif
}

/* ---------- screen ---------- */
static void on_tap(lv_event_t *e)
{
    s_mode = (s_mode + 1) % MODE_COUNT;
    s_mode_t0 = esp_timer_get_time();
    ESP_LOGI(TAG, "mode -> %s", MODE_NAME[s_mode]);
}

static void ui_tick(lv_timer_t *t)
{
    int s = esp_timer_get_time() / 1000000;
    lv_label_set_text_fmt(s_time_lbl, "%02d:%02d", s / 60 % 60, s % 60);
    lv_label_set_text(s_mode_lbl, MODE_NAME[s_mode]);
    lv_label_set_text_fmt(s_info_lbl, "ring %d mA  USB %d mV\ntap to change", s_ring_ma, axp_read16(0x38));
}

static void ui_init(void)
{
    lv_obj_t *scr = lv_screen_active();
    lv_obj_set_style_bg_color(scr, lv_color_black(), 0);
    lv_obj_add_flag(scr, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(scr, on_tap, LV_EVENT_CLICKED, NULL);

    s_time_lbl = lv_label_create(scr);
    lv_obj_set_style_text_font(s_time_lbl, &lv_font_montserrat_48, 0);
    lv_obj_set_style_text_color(s_time_lbl, lv_color_hex(0xFFB869), 0);
    lv_obj_align(s_time_lbl, LV_ALIGN_CENTER, 0, -40);

    s_mode_lbl = lv_label_create(scr);
    lv_obj_set_style_text_font(s_mode_lbl, &lv_font_montserrat_20, 0);
    lv_obj_set_style_text_color(s_mode_lbl, lv_color_hex(0xFFB869), 0);
    lv_obj_align(s_mode_lbl, LV_ALIGN_CENTER, 0, 20);

    s_info_lbl = lv_label_create(scr);
    lv_obj_set_style_text_color(s_info_lbl, lv_color_hex(0xB27C4A), 0);
    lv_obj_set_style_text_align(s_info_lbl, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(s_info_lbl, LV_ALIGN_CENTER, 0, 80);

    lv_timer_create(ui_tick, 250, NULL);
}

void app_main(void)
{
    ESP_LOGI(TAG, "bring-up test starting");
    bsp_display_start();
    bsp_display_brightness_set(60);
    i2c_report();
    ring_init();
    audio_init();

    bsp_display_lock(-1);
    ui_init();
    bsp_display_unlock();

    s_mode_t0 = esp_timer_get_time();
    xTaskCreatePinnedToCore(ring_task, "ring", 4096, NULL, 6, NULL, 1);
    xTaskCreatePinnedToCore(audio_task, "audio", 4096, NULL, 7, NULL, 1);
}
