/* Product firmware, steps 3b-1 and 3b-2: the engine, storage, clock chip, ring and speaker on FreeRTOS,
 * with a bench console. The LVGL UI is step 3c. */
#include <assert.h>
#include <time.h>
#include "nvs_flash.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "hw.h"
#include "bsp/esp-bsp.h"

_Static_assert(sizeof(time_t) == 8, "64-bit time is required (R39, 2038)");
static const char *TAG = "clock";

void app_main(void) {
    esp_err_t e = nvs_flash_init();
    if (e == ESP_ERR_NVS_NO_FREE_PAGES || e == ESP_ERR_NVS_NEW_VERSION_FOUND) { ESP_LOGW(TAG, "NVS erased"); nvs_flash_erase(); nvs_flash_init(); }
    bsp_display_start(); bsp_display_brightness_set(0);   /* screen on but dark until the UI (3c); also brings up the I2C bus */
    rtc_load();
    alarm_engine_start();
    light_fx_start();
    audio_out_start();
    console_start();
    for (;;) { vTaskDelay(pdMS_TO_TICKS(5000)); ESP_LOGI(TAG, "state=%d next=%lld", (int)g_alarm.state, (long long)g_alarm.next_utc); }
}
