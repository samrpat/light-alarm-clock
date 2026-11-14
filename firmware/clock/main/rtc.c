/* PCF85063 clock chip: read at boot, written when the time is set. Not on the alarm path (R18): the engine
 * reads the system clock, which this restores. */
#include <sys/time.h>
#include "esp_log.h"
#include "driver/i2c_master.h"
#include "bsp/esp-bsp.h"
#include "storage.h"
#include "hw.h"

static const char *TAG = "rtc";
static i2c_master_dev_handle_t dev;
bool g_time_trusted;

static bool attach(void) {
    if (dev) return true;
    i2c_master_bus_handle_t bus = bsp_i2c_get_handle(); if (!bus) return false;
    i2c_device_config_t c = {.dev_addr_length = I2C_ADDR_BIT_LEN_7, .device_address = 0x51, .scl_speed_hz = 100000};
    return i2c_master_bus_add_device(bus, &c, &dev) == ESP_OK;
}
void rtc_load(void) {
    uint8_t reg = 0x04, r[7]; int64_t utc; bool osc;
    if (attach() && i2c_master_transmit_receive(dev, &reg, 1, r, 7, 100) == ESP_OK && pcf85063_decode(r, &utc, &osc) && !osc) {
        struct timeval tv = {.tv_sec = (time_t)utc}; settimeofday(&tv, NULL); g_time_trusted = true; ESP_LOGI(TAG, "time restored from the chip");
    } else { g_time_trusted = false; ESP_LOGW(TAG, "chip time not trusted: the UI will ask for the time"); }
}
bool rtc_set(int64_t utc) {
    struct timeval tv = {.tv_sec = (time_t)utc}; settimeofday(&tv, NULL);
    uint8_t w[8] = {0x04}; pcf85063_encode(utc, w + 1);                  /* writing seconds clears the oscillator-stop flag */
    bool ok = attach() && i2c_master_transmit(dev, w, 8, 100) == ESP_OK; g_time_trusted = ok;
    alarm_post(CMD_TIME_SET); return ok;
}
