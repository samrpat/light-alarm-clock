/* Bench console on the USB serial port (used by tools/bench.py). Every schedule change goes through the engine,
 * exactly as the screen will do it.
 *   time @<utc seconds>        set the clock (as the Time page does)
 *   rawtime @<utc seconds>     set the clock without telling the engine (to act out "the power was off")
 *   day <0-6> <HH:MM|off>      weekday alarm, 0 = Monday (local time)
 *   tomorrow <HH:MM>  skip  unskip  clear
 *   snooze  dismiss  test  vol <30-100>  mute <on|off>  reboot  dump  status                                    */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "esp_system.h"
#include "hw.h"
#include "store.h"

static void status(void) {
    alarm_snapshot_t s; alarm_snapshot_get(&s);
    printf("STATUS state=%d light=%d dawn_t=%d stage=%d el=%d snz=%d next=%lld now=%lld trusted=%d est=%d notime=%d damaged=%d mute=%d vol=%d\n",
           (int)s.state, s.light_on, (int)s.dawn_t_s, (int)s.sound_stage, (int)s.ring_elapsed_s, (int)s.snoozes_left, (long long)s.next_utc, (long long)time(NULL),
           g_time_trusted, g_time_estimated, s.no_time, s.sched_damaged, g_bench_mute, g_alarm_vol);
}
static void run(char *l) {
    long long t; int wd, h, mi, v;
    if (sscanf(l, "time @%lld", &t) == 1) printf("OK time chip=%d\n", rtc_set(t, true));
    else if (sscanf(l, "rawtime @%lld", &t) == 1) printf("OK rawtime chip=%d\n", rtc_set(t, false));
    else if (sscanf(l, "day %d %d:%d", &wd, &h, &mi) == 3) { alarm_post(CMD_SET_DAY, wd, h * 60 + mi); printf("OK day\n"); }
    else if (sscanf(l, "day %d off", &wd) == 1) { alarm_post(CMD_SET_DAY, wd, -1); printf("OK day off\n"); }
    else if (sscanf(l, "tomorrow %d:%d", &h, &mi) == 2) { alarm_post(CMD_SET_TOMORROW, 0, h * 60 + mi); printf("OK tomorrow\n"); }
    else if (!strncmp(l, "skip", 4)) { alarm_post(CMD_SKIP_NEXT, 0, 0); printf("OK skip\n"); }
    else if (!strncmp(l, "unskip", 6)) { alarm_post(CMD_UNSKIP, 0, 0); printf("OK unskip\n"); }
    else if (!strncmp(l, "clear", 5)) { alarm_post(CMD_CLEAR_ALL, 0, 0); printf("OK clear\n"); }
    else if (!strncmp(l, "snooze", 6)) { alarm_post(CMD_SNOOZE, 0, 0); printf("OK snooze\n"); }
    else if (!strncmp(l, "dismiss", 7)) { alarm_post(CMD_DISMISS, 0, 0); printf("OK dismiss\n"); }
    else if (!strncmp(l, "test", 4)) { g_test_until_us = esp_timer_get_time() + 6000000; printf("OK test 6 s\n"); }
    else if (sscanf(l, "vol %d", &v) == 1) { g_alarm_vol = v < 30 ? 30 : v > 100 ? 100 : v; store_set_i64("avol", g_alarm_vol); printf("OK vol %d\n", g_alarm_vol); }
    else if (!strncmp(l, "mute on", 7)) { g_bench_mute = true; store_set_i64("mute", 1); printf("OK mute on\n"); }
    else if (!strncmp(l, "mute off", 8)) { g_bench_mute = false; store_set_i64("mute", 0); printf("OK mute off\n"); }
    else if (!strncmp(l, "reboot", 6)) { printf("OK reboot\n"); fflush(stdout); vTaskDelay(pdMS_TO_TICKS(100)); esp_restart(); }
    else if (!strncmp(l, "dump", 4)) store_dump();
    else if (!strncmp(l, "status", 6)) status();
    else printf("ERR unknown: %s\n", l);
}
static void task(void *a) {
    (void)a; char line[80]; int n = 0;
    for (;;) {
        int c = getchar();
        if (c == EOF) { vTaskDelay(pdMS_TO_TICKS(20)); continue; }
        if (c == '\n' || c == '\r') { line[n] = 0; if (n) run(line); n = 0; } else if (n < 79) line[n++] = (char)c;
    }
}
void console_start(void) { xTaskCreatePinnedToCore(task, "console", 4096, NULL, 4, NULL, 0); }
