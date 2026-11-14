/* The alarm state machine (FIRMWARE.md section 3, D-007, D-015, R18-R23).
 * Pure: it is given the time, the next scheduled occurrence and an event, and says what the
 * light and the sound should do. It owns no hardware, so it runs on a computer for testing and on
 * core 1 for real. The persisted part (alarm_sm_t) is small and plain so it can be restored after
 * a reset and the alarm resumes at the right stage (R20, R21). */
#pragma once
#include <stdint.h>
#include <stdbool.h>

typedef enum { SM_IDLE = 0, SM_SUNRISE, SM_RINGING, SM_SNOOZED, SM_MISSED } sm_state_t;
typedef enum { EV_NONE = 0, EV_SNOOZE, EV_DISMISS, EV_TIME_SET } sm_event_t;

typedef struct {
    int32_t sunrise_s;      /* 1800 */
    int32_t stage_s;        /* 60: sound steps up every minute */
    int32_t auto_stop_s;    /* 1800 */
    int32_t snooze_s;       /* 540 */
    int32_t max_snooze;     /* 3 */
    int32_t catchup_s;      /* 3600 */
    int32_t missed_hold_s;  /* 3600: the light stays on, then the alarm is closed */
} sm_cfg_t;

typedef struct {            /* persisted (RTC slow memory plus NVS on transitions) */
    uint8_t  state;
    uint8_t  snoozes;
    int64_t  armed;         /* the occurrence being waited for or served (UTC), 0 = none */
    int64_t  handled;       /* the last occurrence that was handled */
    int64_t  ring_t0;       /* when this ringing episode began */
    int64_t  snooze_until;
} alarm_sm_t;

typedef struct {
    sm_state_t state;
    bool    light_on;
    int32_t dawn_t_s;       /* seconds relative to the alarm time: -1800 .. +600 */
    int32_t sound_stage;    /* 0 = silent, 1..4 = escalation steps */
    bool    persist;        /* state changed: write it down */
    bool    missed;         /* the sound gave up: show "Alarm missed" */
    bool    ended;          /* an occurrence was just handled: the caller clears overrides */
    int32_t snoozes_left;
} sm_out_t;

#define ALARM_STAGES 4
void sm_default_cfg(sm_cfg_t *c);
void sm_init(alarm_sm_t *sm);
sm_out_t sm_step(alarm_sm_t *sm, const sm_cfg_t *c, int64_t now, int64_t next_occ_utc, sm_event_t ev);
