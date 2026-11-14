/* audio_out: core 1, priority 19. Plays the alarm in four steps from the snapshot (R22). A soft three-note
 * chime that gets louder; never silent while the alarm rings (floor). The sleep sounds come in Phase 5. */
#include <math.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_task_wdt.h"
#include "bsp/esp-bsp.h"
#include "esp_codec_dev.h"
#include "hw.h"

static const char *TAG = "audio";
#define FS 22050
static esp_codec_dev_handle_t spk;
static const float LEVEL[5] = {0, 0.06f, 0.12f, 0.25f, 0.45f};      /* of full scale per stage; stage 1 is already audible */
static const float NOTE[3] = {659.25f, 783.99f, 987.77f};

static void task(void *arg) {
    (void)arg; esp_task_wdt_add(NULL);
    static int16_t buf[441]; float ph = 0; uint32_t n = 0;                     /* 20 ms blocks */
    for (;;) {
        alarm_snapshot_t s; do { s.seq = g_alarm.seq; s = g_alarm; } while ((s.seq & 1) || s.seq != g_alarm.seq);
        int st = s.sound_stage > 4 ? 4 : s.sound_stage; float amp = LEVEL[st] * 32767.0f;
        for (int i = 0; i < 441; i++, n++) {
            float t = (n % (FS * 2)) / (float)FS;                              /* 2 s phrase: three notes then rest */
            int k = (int)(t / 0.3f); float env = 0, f = 0;
            if (k < 3) { float u = t - k * 0.3f; env = expf(-5.0f * u) * (u < 0.01f ? u / 0.01f : 1.0f); f = NOTE[k]; }
            ph += 2 * (float)M_PI * f / FS; if (ph > 2 * (float)M_PI) ph -= 2 * (float)M_PI;
            buf[i] = (int16_t)(sinf(ph) * env * amp);
        }
        if (spk) esp_codec_dev_write(spk, buf, sizeof buf); else vTaskDelay(pdMS_TO_TICKS(20));
        esp_task_wdt_reset();
    }
}
void audio_out_start(void) {
    spk = bsp_audio_codec_speaker_init();
    if (spk) { esp_codec_dev_sample_info_t fs = {.sample_rate = FS, .channel = 1, .bits_per_sample = 16}; if (esp_codec_dev_open(spk, &fs) != ESP_OK) spk = NULL; else esp_codec_dev_set_out_vol(spk, 70); }
    ESP_LOGI(TAG, "speaker: %s (the alarm still lights and shows without it)", spk ? "ok" : "missing");
    xTaskCreatePinnedToCore(task, "audio_out", 8192, NULL, 19, NULL, 1);
}
