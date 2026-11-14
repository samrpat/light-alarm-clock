/* What the alarm engine may command. The engine never talks to a chip itself (D-002, R18): it fills a small
 * snapshot that the light and audio tasks read. Today the outputs only log; the ring and speaker tasks
 * from /firmware/bringup move in next (Phase 3 step 3b-2). */
#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "alarm_sm.h"

typedef struct {
    volatile uint32_t seq;
    sm_state_t state; bool light_on; int32_t dawn_t_s; int32_t sound_stage; bool missed;
    int64_t next_utc;
} alarm_snapshot_t;
extern alarm_snapshot_t g_alarm;

typedef enum { CMD_SNOOZE = 1, CMD_DISMISS, CMD_TIME_SET, CMD_SCHED_CHANGED } alarm_cmd_t;
void alarm_engine_start(void);                 /* core 1 */
void alarm_post(alarm_cmd_t cmd);              /* from UI, input and the BOOT button: never blocks */

void light_fx_start(void);                     /* core 1 */
void audio_out_start(void);                    /* core 1 */
void rtc_load(void);                           /* boot: restore the system clock from the PCF85063 */
bool rtc_set(int64_t utc);                     /* set by hand: system clock, chip, and tell the engine */
extern bool g_time_trusted;
void console_start(void);                      /* bench console on the USB port (temporary until the UI) */
