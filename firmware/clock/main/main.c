/* Product firmware (Phase 3). Engine, storage, clock chip, ring, alarm sound and stop gestures are real;
 * the screen graphics are temporary until step 3c. */
#include <time.h>
#include "nvs_flash.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "bsp/esp-bsp.h"
#include "hw.h"
#include "store.h"

_Static_assert(sizeof(time_t) == 8, "64-bit time is required (R39, 2038)");
static const char *TAG = "clock";

void app_main(void) {
    esp_err_t e = nvs_flash_init();
    if (e == ESP_ERR_NVS_NO_FREE_PAGES || e == ESP_ERR_NVS_NEW_VERSION_FOUND) { ESP_LOGW(TAG, "NVS erased"); nvs_flash_erase(); nvs_flash_init(); }
    g_bench_mute = store_get_i64("mute", 0) != 0;
    int v = (int)store_get_i64("avol", 70); g_alarm_vol = v < 30 ? 30 : v > 100 ? 100 : v;
    bsp_display_start(); bsp_display_brightness_set(0);          /* also brings up the I2C bus */
    rtc_load();
    alarm_engine_start();
    light_fx_start();
    audio_out_start();
    console_start();
    face_start();
    rtc_housekeeping_start();
    ESP_LOGI(TAG, "READY");
}
