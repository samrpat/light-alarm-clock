/* Time keeping around the PCF85063 clock chip. Not on the alarm path (R18): the engine reads the system clock.
 * If the chip lost its time (no battery fitted and the power went), the clock carries on from the last time it
 * remembered, so the alarm rings a little late instead of not at all (D-003); the screen says "Check time". */
#include <sys/time.h>
#include <time.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/i2c_master.h"
#include "bsp/esp-bsp.h"
#include "storage.h"
#include "store.h"
#include "hw.h"

static const char *TAG = "rtc";
static i2c_master_dev_handle_t dev;
volatile bool g_time_trusted, g_time_estimated;

static bool attach(void) {
    if (dev) return true;
    i2c_master_bus_handle_t bus = bsp_i2c_get_handle();
    if (!bus) return false;
    i2c_device_config_t c = {.dev_addr_length = I2C_ADDR_BIT_LEN_7, .device_address = 0x51, .scl_speed_hz = 100000};
    return i2c_master_bus_add_device(bus, &c, &dev) == ESP_OK;
}
static void sys_set(int64_t utc) { struct timeval tv = {.tv_sec = (time_t)utc}; settimeofday(&tv, NULL); }
void rtc_load(void) {
    uint8_t reg = 0x04, r[7]; int64_t utc = 0; bool osc = true;
    bool ok = attach() && i2c_master_transmit_receive(dev, &reg, 1, r, 7, 100) == ESP_OK && pcf85063_decode(r, &utc, &osc) && !osc && utc >= MIN_VALID_UTC;
    if (ok) { sys_set(utc); g_time_trusted = true; ESP_LOGI(TAG, "time restored from the chip"); return; }
    int64_t lkt = store_get_i64("lkt", 0);
    if (lkt >= MIN_VALID_UTC) { sys_set(lkt); g_time_estimated = true; ESP_LOGW(TAG, "chip lost its time: carrying on from the last remembered time (Check time)"); }
    else ESP_LOGW(TAG, "no time at all: Set time; alarms wait");
}
bool rtc_set(int64_t utc, bool tell_engine) {
    sys_set(utc);
    uint8_t w[8] = {0x04}; pcf85063_encode(utc, w + 1);                 /* writing seconds clears the oscillator-stop flag */
    bool ok = attach() && i2c_master_transmit(dev, w, 8, 100) == ESP_OK;
    if (!ok) ESP_LOGE(TAG, "could not write the clock chip (time is right until the power goes)");
    g_time_trusted = utc >= MIN_VALID_UTC; g_time_estimated = false;
    if (utc >= MIN_VALID_UTC) store_set_i64("lkt", utc);
    if (tell_engine) alarm_post(CMD_TIME_SET, 0, 0);
    return ok;
}
static void keeper(void *a) {
    (void)a;
    for (;;) {
        vTaskDelay(pdMS_TO_TICKS(300000));                              /* every 5 minutes; flash wear is negligible (LONGEVITY.md) */
        int64_t now = (int64_t)time(NULL);
        if (now >= MIN_VALID_UTC) store_set_i64("lkt", now);
    }
}
void rtc_housekeeping_start(void) { xTaskCreatePinnedToCore(keeper, "timekeeper", 3072, NULL, 3, NULL, 0); }
