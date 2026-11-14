/* Product firmware, step 3b-1: the engine and its storage on real FreeRTOS. Screen, ring, speaker and
 * the PCF85063 task follow in 3b-2 (they reuse /firmware/bringup). */
#include <assert.h>
#include <time.h>
#include "nvs_flash.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "hw.h"

_Static_assert(sizeof(time_t) == 8, "64-bit time is required (R39, 2038)");
static const char *TAG = "clock";

void app_main(void) {
    esp_err_t e = nvs_flash_init();
    if (e == ESP_ERR_NVS_NO_FREE_PAGES || e == ESP_ERR_NVS_NEW_VERSION_FOUND) { ESP_LOGW(TAG, "NVS erased"); nvs_flash_erase(); nvs_flash_init(); }
    alarm_engine_start();
    for (;;) { vTaskDelay(pdMS_TO_TICKS(5000)); ESP_LOGI(TAG, "state=%d next=%lld", (int)g_alarm.state, (long long)g_alarm.next_utc); }
}
