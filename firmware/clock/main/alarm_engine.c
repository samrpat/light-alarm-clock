/* The alarm engine: core 1, priority 20 (FIRMWARE.md). It owns the schedule in RAM and calls the pure core.
 * It reads the time only from the system clock (kept up to date by the RTC task), never from I2C, and
 * never waits for the UI (R18). */
#include <string.h>
#include <time.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include "esp_task_wdt.h"
#include "hw.h"
#include "store.h"
#include "tz.h"
#include "sched.h"

static const char *TAG = "alarm";
alarm_snapshot_t g_alarm;
static QueueHandle_t q;

void alarm_post(alarm_cmd_t c) { if (q) xQueueSend(q, &c, 0); }

static void engine(void *arg) {
    (void)arg; esp_task_wdt_add(NULL);
    sched_t s; bool damaged; store_load_sched(&s, &damaged);
    char tzs[64]; store_load_region(tzs, sizeof tzs); tz_t tz; if (!tz_parse(tzs, &tz)) tz_parse("EST5EDT,M3.2.0/2,M11.1.0/2", &tz);
    alarm_sm_t sm; store_load_sm(&sm); sm_cfg_t cfg; sm_default_cfg(&cfg);
    ESP_LOGI(TAG, "engine up%s", damaged ? " (SCHEDULE DAMAGED: empty)" : "");
    for (;;) {
        sm_event_t ev = EV_NONE; alarm_cmd_t c;
        while (xQueueReceive(q, &c, 0) == pdTRUE) {
            if (c == CMD_SNOOZE) ev = EV_SNOOZE; else if (c == CMD_DISMISS) ev = EV_DISMISS; else if (c == CMD_TIME_SET) ev = EV_TIME_SET;
            else if (c == CMD_SCHED_CHANGED) store_load_sched(&s, &damaged);
        }
        int64_t now = (int64_t)time(NULL);                          /* UTC; 64-bit (checked in main) */
        occurrence_t o = next_alarm(&s, &tz, now);
        sm_out_t out = sm_step(&sm, &cfg, now, o.valid ? o.utc : 0, ev);
        if (out.persist) store_save_sm(&sm);
        if (out.ended) { sched_expire(&s, local_day_of(&tz, now)); }
        g_alarm.seq++; g_alarm.state = out.state; g_alarm.light_on = out.light_on; g_alarm.dawn_t_s = out.dawn_t_s;
        g_alarm.sound_stage = out.sound_stage; g_alarm.missed = out.missed; g_alarm.next_utc = o.valid ? o.utc : 0; g_alarm.seq++;
        esp_task_wdt_reset();
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
void alarm_engine_start(void) {
    q = xQueueCreate(8, sizeof(alarm_cmd_t));
    xTaskCreatePinnedToCore(engine, "alarm_engine", 6144, NULL, 20, NULL, 1);
}
