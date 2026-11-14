/* A bench console on the USB serial port, so the engine can be tested before the UI exists (3c).
 *   time 2026-11-14 06:29:30     set the clock (UTC)
 *   alarm 0 06:45 | alarm 0 off  weekday 0=Mon..6=Sun
 *   in 90                        one alarm 90 seconds from now
 *   snooze | dismiss | status                                                                                   */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "hw.h"
#include "store.h"
#include "civil.h"

static const char *TAG = "console";
static sched_t sched;

static void run(char *l) {
    int y, mo, d, h, mi, s, wd;
    if (sscanf(l, "time %d-%d-%d %d:%d:%d", &y, &mo, &d, &h, &mi, &s) == 6) { civil_t c = {y, mo, d, h, mi, s}; ESP_LOGI(TAG, "time set: %d", rtc_set(civil_to_secs(&c))); }
    else if (sscanf(l, "alarm %d %d:%d", &wd, &h, &mi) == 3 && wd >= 0 && wd < 7) { bool dummy; store_load_sched(&sched, &dummy); sched.day[wd].on = true; sched.day[wd].min = (uint16_t)(h * 60 + mi); store_save_sched(&sched); alarm_post(CMD_SCHED_CHANGED); ESP_LOGI(TAG, "alarm %d at %02d:%02d", wd, h, mi); }
    else if (sscanf(l, "alarm %d off", &wd) == 1 && wd >= 0 && wd < 7) { bool dummy; store_load_sched(&sched, &dummy); sched.day[wd].on = false; store_save_sched(&sched); alarm_post(CMD_SCHED_CHANGED); ESP_LOGI(TAG, "alarm %d off", wd); }
    else if (sscanf(l, "in %d", &s) == 1) { bool dummy; store_load_sched(&sched, &dummy); time_t t = time(NULL) + s; sched.one_day = (int32_t)(t / 86400); sched.one_min = (uint16_t)(t % 86400 / 60); store_save_sched(&sched); alarm_post(CMD_SCHED_CHANGED); ESP_LOGI(TAG, "one alarm (UTC clock, minute resolution)"); }
    else if (!strncmp(l, "snooze", 6)) alarm_post(CMD_SNOOZE);
    else if (!strncmp(l, "dismiss", 7)) alarm_post(CMD_DISMISS);
    else if (!strncmp(l, "status", 6)) ESP_LOGI(TAG, "state=%d light=%d dawn_t=%d stage=%d next=%lld trusted=%d", (int)g_alarm.state, g_alarm.light_on, (int)g_alarm.dawn_t_s, (int)g_alarm.sound_stage, (long long)g_alarm.next_utc, g_time_trusted);
}
static void task(void *a) {
    char line[80]; int n = 0;
    for (;;) { int c = getchar(); if (c == EOF) { vTaskDelay(pdMS_TO_TICKS(50)); continue; }
        if (c == '\n' || c == '\r') { line[n] = 0; if (n) run(line); n = 0; } else if (n < 79) line[n++] = (char)c; }
}
void console_start(void) { xTaskCreatePinnedToCore(task, "console", 4096, NULL, 4, NULL, 0); }
