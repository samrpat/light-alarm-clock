/* The alarm engine: core 1, priority 20 (FIRMWARE.md). It owns the schedule and calls the tested core.
 * It reads the time only from the system clock, never from I2C, and never waits for the screen (R18).
 * Every change to the schedule comes in as a message, so there is one writer. */
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include "esp_task_wdt.h"
#include "driver/gpio.h"
#include "hw.h"
#include "store.h"
#include "storage.h"
#include "tz.h"
#include "sched.h"

static const char *TAG = "alarm";
typedef struct { uint8_t type; int32_t a, b; } msg_t;
static QueueHandle_t q;
static alarm_snapshot_t snap;
static portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;

void alarm_post(alarm_cmd_t c, int a, int b) { msg_t m = {(uint8_t)c, a, b}; if (q) xQueueSend(q, &m, 0); }
void alarm_snapshot_get(alarm_snapshot_t *out) { portENTER_CRITICAL(&mux); *out = snap; portEXIT_CRITICAL(&mux); }

static void engine(void *arg) {
    (void)arg; esp_task_wdt_add(NULL);
    sched_t s; bool damaged; store_load_sched(&s, &damaged);
    char tzs[64]; store_load_region(tzs, sizeof tzs); tz_t tz;
    if (!tz_parse(tzs, &tz)) tz_parse("EST5EDT,M3.2.0/2,M11.1.0/2", &tz);
    alarm_sm_t sm; store_load_sm(&sm); sm_cfg_t cfg; sm_default_cfg(&cfg);
    ESP_LOGI(TAG, "engine up%s", damaged ? " (SCHEDULE DAMAGED: empty, warning shown)" : "");
    bool first = true;
    for (;;) {
        struct timeval tv; gettimeofday(&tv, NULL);
        int wait_ms = 1000 - (int)(tv.tv_usec / 1000); if (wait_ms < 10) wait_ms = 10;
        msg_t m; sm_event_t ev = EV_NONE; int64_t now = (int64_t)time(NULL);
        /* the very first pass does not wait: the outputs know the state within milliseconds of power-up */
        if (!first && xQueueReceive(q, &m, pdMS_TO_TICKS(wait_ms)) == pdTRUE) {          /* a message is handled at once, not at the next second */
            now = (int64_t)time(NULL); sched_t before = s; bool edit = true;
            switch (m.type) {
            case CMD_SNOOZE: ev = EV_SNOOZE; edit = false; break;
            case CMD_DISMISS: ev = EV_DISMISS; edit = false; break;
            case CMD_TIME_SET: ev = EV_TIME_SET; edit = false; break;
            case CMD_SET_DAY: if (m.a >= 0 && m.a < 7) { s.day[m.a].on = m.b >= 0; if (m.b >= 0) s.day[m.a].min = (uint16_t)m.b; } break;
            case CMD_SET_TOMORROW: sched_set_tomorrow(&s, &tz, now, (uint16_t)m.b); break;
            case CMD_SKIP_NEXT: sched_skip_next(&s, &tz, now); break;
            case CMD_UNSKIP: sched_unskip(&s); break;
            case CMD_CLEAR_ALL: sched_init(&s); break;
            default: edit = false; break;
            }
            if (edit) {
                if (sched_valid(&s) && store_save_sched(&s)) { ev = EV_SCHED_CHANGED; damaged = false; }
                else { s = before; ESP_LOGE(TAG, "schedule change rejected"); }
            }
        }
        bool no_time = now < MIN_VALID_UTC;                                    /* never set: an alarm cannot be placed in time */
        occurrence_t o = {0}; if (!no_time) o = next_alarm(&s, &tz, now);
        sm_out_t out = sm_step(&sm, &cfg, now, o.valid ? o.utc : 0, ev);
        portENTER_CRITICAL(&mux);
        snap.state = out.state; snap.light_on = out.light_on; snap.dawn_t_s = out.dawn_t_s; snap.sound_stage = out.sound_stage;
        snap.ring_elapsed_s = out.ring_elapsed_s; snap.missed = out.missed; snap.snoozes_left = out.snoozes_left;
        snap.next_utc = o.valid ? o.utc : 0; snap.no_time = no_time; snap.sched_damaged = damaged;
        portEXIT_CRITICAL(&mux);
        /* storage after the outputs are told: a flash write never delays the light or the sound */
        if (out.ended) {
            sched_expire(&s, local_day_of(&tz, now));
            if (out.ended_occ > now) s.skip_day = local_day_of(&tz, out.ended_occ);       /* dismissed early: that day is done */
            store_save_sched(&s);
        }
        if (out.persist) store_save_sm(&sm);
        first = false;
        esp_task_wdt_reset();
    }
}
/* BOOT button (GPIO0): hold 3 s to stop the alarm even if the screen or touch is dead (R23, D-007). */
static void boot_btn(void *arg) {
    (void)arg; gpio_config_t c = {.pin_bit_mask = 1ULL << GPIO_NUM_0, .mode = GPIO_MODE_INPUT, .pull_up_en = GPIO_PULLUP_ENABLE}; gpio_config(&c);
    int held = 0;
    for (;;) {
        if (gpio_get_level(GPIO_NUM_0) == 0) { if (++held == 60) { ESP_LOGW(TAG, "BOOT held 3 s: dismiss"); alarm_post(CMD_DISMISS, 0, 0); } }
        else held = 0;
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}
void alarm_engine_start(void) {
    q = xQueueCreate(8, sizeof(msg_t));
    xTaskCreatePinnedToCore(engine, "alarm_engine", 6144, NULL, 20, NULL, 1);
    xTaskCreatePinnedToCore(boot_btn, "boot_btn", 2560, NULL, 18, NULL, 1);
}
