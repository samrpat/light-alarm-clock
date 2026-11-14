/* audio_out: core 1, priority 19. The alarm sound "Rise" (D-049), made by the clock itself:
 * bell-like notes of a G major pentatonic scale rising through four phrases, each a little fuller and quicker,
 * while the loudness climbs smoothly for 3 minutes (alarm_gain, tested on the host). Notes sit between 780 Hz and
 * 1570 Hz with overtones, where the small 2 W speaker is efficient. Sleep sounds are Phase 5. */
#include <math.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "esp_task_wdt.h"
#include "bsp/esp-bsp.h"
#include "esp_codec_dev.h"
#include "hw.h"
#include "dawn.h"

static const char *TAG = "audio";
#define FS 22050
#define BLOCK 441                                   /* 20 ms */
#define NV 8
volatile int g_alarm_vol = 70;
volatile int64_t g_test_until_us;
volatile bool g_bench_mute;
static esp_codec_dev_handle_t spk;
static float lut[1024];

static const float NOTE[6] = {783.99f, 880.00f, 987.77f, 1174.66f, 1318.51f, 1567.98f};      /* G5 A5 B5 D6 E6 G6 */
typedef struct { int n; float period; float t[6]; uint8_t note[6]; } phrase_t;
static const phrase_t PH[5] = {
    {0, 1.0f, {0}, {0}},
    {3, 4.0f, {0, .50f, 1.00f}, {0, 2, 3}},                             /* stage 1: three slow notes, long rest */
    {4, 3.0f, {0, .40f, .80f, 1.20f}, {0, 2, 3, 4}},                    /* stage 2: four notes */
    {5, 2.2f, {0, .30f, .60f, .90f, 1.20f}, {0, 2, 3, 4, 5}},           /* stage 3: the full rise */
    {6, 1.4f, {0, .18f, .36f, .54f, .72f, .90f}, {0, 2, 3, 5, 3, 5}},   /* stage 4: quick and insistent */
};
typedef struct { bool on; uint32_t ph[3], inc[3]; float env; int age; } voice_t;
static voice_t v[NV];
static const float PART[3] = {0.57f, 0.29f, 0.14f};                      /* fundamental and two overtones: bell-like, and louder on a small speaker */

static void note_on(float f) {
    int k = 0; for (int i = 0; i < NV; i++) { if (!v[i].on) { k = i; break; } if (v[i].age > v[k].age) k = i; }
    v[k].on = true; v[k].age = 0; v[k].env = 0;
    for (int p = 0; p < 3; p++) { v[k].ph[p] = 0; v[k].inc[p] = (uint32_t)(f * (p + 1) / FS * 4294967296.0f); }
}
static void task(void *arg) {
    (void)arg; esp_task_wdt_add(NULL);
    static int16_t buf[BLOCK]; float g = 0; int stage = 0, pos = 0, next_note = 0;
    const float decay = expf(-1.0f / (0.35f * FS)); const int attack = FS * 6 / 1000;
    for (;;) {
        alarm_snapshot_t s; alarm_snapshot_get(&s);
        bool test = esp_timer_get_time() < g_test_until_us;
        int want = s.sound_stage > 4 ? 4 : s.sound_stage; int el = s.ring_elapsed_s;
        if (test && want == 0) { want = 3; el = 180; }                    /* alarm test (R26): the full sound at the set volume */
        float target = (want > 0 && !g_bench_mute) ? alarm_gain(g_alarm_vol, el) : 0.0f;
        float g1 = g + (target - g) * (target > g ? 0.10f : 0.45f);       /* up in about 0.2 s, down in about 50 ms, never a click */
        if (want != stage && (pos == 0 || stage == 0 || want == 0)) { stage = want; pos = 0; next_note = 0; }
        const phrase_t *ph = &PH[stage];
        for (int i = 0; i < BLOCK; i++) {
            if (stage > 0) {
                while (next_note < ph->n && pos >= (int)(ph->t[next_note] * FS)) note_on(NOTE[ph->note[next_note++]]);
                if (++pos >= (int)(ph->period * FS)) { pos = 0; next_note = 0; if (want != stage) { stage = want; ph = &PH[stage]; } }
            }
            float x = 0;
            for (int k = 0; k < NV; k++) {
                if (!v[k].on) continue;
                voice_t *o = &v[k]; float sm = 0;
                for (int p = 0; p < 3; p++) { sm += PART[p] * lut[o->ph[p] >> 22]; o->ph[p] += o->inc[p]; }
                if (o->age < attack) o->env = (float)o->age / attack; else o->env *= decay;
                x += sm * o->env; if (++o->age > FS * 2) o->on = false;
            }
            x *= 0.80f; if (x > 1) x = 1; else if (x < -1) x = -1;
            x = 1.5f * x - 0.5f * x * x * x;                              /* gentle limiter: overlapping notes never clip harshly */
            float gg = g + (g1 - g) * (float)i / BLOCK;
            buf[i] = (int16_t)(x * gg * 32000.0f);
        }
        g = g1;
        if (spk) esp_codec_dev_write(spk, buf, sizeof buf); else vTaskDelay(pdMS_TO_TICKS(20));
        esp_task_wdt_reset();
    }
}
void audio_out_start(void) {
    for (int i = 0; i < 1024; i++) lut[i] = sinf(2 * (float)M_PI * i / 1024);
    spk = bsp_audio_codec_speaker_init();
    if (spk) {
        esp_codec_dev_sample_info_t fs = {.sample_rate = FS, .channel = 1, .bits_per_sample = 16};
        if (esp_codec_dev_open(spk, &fs) != ESP_OK) spk = NULL; else esp_codec_dev_set_out_vol(spk, 100);    /* fixed; loudness is done in samples (D-008) */
    }
    ESP_LOGI(TAG, "speaker: %s (the alarm still lights and shows without it)", spk ? "ok" : "missing");
    xTaskCreatePinnedToCore(task, "audio_out", 8192, NULL, 19, NULL, 1);
}
