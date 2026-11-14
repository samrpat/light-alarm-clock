/* Shared between the firmware tasks. The alarm engine never talks to a chip itself (D-002, R18): it publishes a
 * small snapshot that the light, sound and screen tasks read. */
#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "alarm_sm.h"
#include "sched.h"
#include "dawn.h"

#define MIN_VALID_UTC 1767225600LL   /* 2026-01-01: before this the clock has clearly never been set */

typedef struct {
    sm_state_t state; bool light_on; int32_t dawn_t_s; int32_t sound_stage; int32_t ring_elapsed_s; bool missed;
    int32_t snoozes_left; int64_t next_utc; bool no_time; bool sched_damaged;
} alarm_snapshot_t;
void alarm_snapshot_get(alarm_snapshot_t *out);     /* safe from any task */

typedef enum { CMD_SNOOZE = 1, CMD_DISMISS, CMD_TIME_SET, CMD_SET_DAY, CMD_SET_TOMORROW, CMD_SKIP_NEXT, CMD_UNSKIP, CMD_CLEAR_ALL,
               CMD_SET_ONCE /* a=local day, b=minute */, CMD_CLEAR_ONCE /* a=day */, CMD_SKIP_DAY /* a=day */, CMD_TZ_RELOAD }  alarm_cmd_t;
void alarm_sched_get(sched_t *out);                 /* a copy of the schedule, for the screen */
void alarm_engine_start(void);                      /* core 1 */
void alarm_post(alarm_cmd_t cmd, int a, int b);     /* from the screen, the BOOT button, the console: never blocks */

void light_fx_start(void);                          /* core 1 */
void audio_out_start(void);                         /* core 1 */
void console_start(void);                           /* core 0: bench console on the USB port */

void rtc_load(void);                                /* boot: restore the system clock */
bool rtc_set(int64_t utc, bool tell_engine);        /* set the clock: system time, chip, last-known time */
void rtc_housekeeping_start(void);                  /* core 0: remembers the time every 5 minutes */
extern volatile bool g_time_trusted;                /* restored from the clock chip or set by hand since power-up */
extern volatile bool g_time_estimated;              /* chip lost its time: running from the last remembered time */

extern volatile int g_alarm_vol;                    /* 30..100 %, the Alarm page setting (R22: never silent) */
extern volatile int64_t g_test_until_us;            /* alarm test (R26): light and sound until this esp_timer time */
extern volatile bool g_bench_mute;
extern rgbw_f g_lamp;                               /* the Lamp screen's light; shown whenever no alarm owns the ring */
extern volatile int g_chime;                        /* focus timer: 2 or 3 soft notes, cleared when played */
extern volatile bool g_snd_play, g_snd_ended;       /* sleep sound on; set by the sound task when the sleep timer ran out */
extern volatile int g_snd_id, g_snd_vol, g_snd_timer_min;
float sleep_sample(int id);                         /* sounds.c */                  /* bench only: self-checks run with light and sound off */
