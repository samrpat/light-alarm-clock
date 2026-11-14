/* The weekly schedule and the "next alarm that will really fire" (R1, R3, R4, D-003, D-015).
 * Alarms are wall-clock times. next_alarm() is a pure function. */
#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "tz.h"

#define NO_DAY INT32_MIN
typedef struct { bool on; uint16_t min; } sched_day_t;      /* minutes after local midnight */
typedef struct {
    sched_day_t day[7];        /* 0 = Monday */
    int32_t skip_day;          /* local day number (days since 1970) whose alarm is skipped, or NO_DAY */
    int32_t one_day;           /* local day number with a one-off time, or NO_DAY */
    uint16_t one_min;
} sched_t;

typedef struct { bool valid; int64_t utc; int32_t local_day; uint16_t min; bool from_override; local_kind_t kind; } occurrence_t;

void sched_init(sched_t *s);                                  /* empty: no alarms (D-015) */
int32_t local_day_of(const tz_t *tz, int64_t utc);
/* First alarm strictly after now_utc, looking up to 14 days ahead. */
occurrence_t next_alarm(const sched_t *s, const tz_t *tz, int64_t now_utc);
/* Drop overrides for days that are over. Call when an occurrence is handled and at midnight. */
void sched_expire(sched_t *s, int32_t today);
void sched_skip_next(sched_t *s, const tz_t *tz, int64_t now_utc);   /* hold: skip the next alarm */
void sched_unskip(sched_t *s);
void sched_set_tomorrow(sched_t *s, const tz_t *tz, int64_t now_utc, uint16_t min);
