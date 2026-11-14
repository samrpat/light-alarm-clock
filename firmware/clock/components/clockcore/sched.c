#include "sched.h"
#include "civil.h"

void sched_init(sched_t *s) { for (int i = 0; i < 7; i++) { s->day[i].on = false; s->day[i].min = 420; } s->skip_day = NO_DAY; s->one_day = NO_DAY; s->one_min = 0; }
int32_t local_day_of(const tz_t *tz, int64_t utc) { return (int32_t)floor_div(tz_utc_to_local(tz, utc), 86400); }

occurrence_t next_alarm(const sched_t *s, const tz_t *tz, int64_t now) {
    occurrence_t o = {0};
    int32_t today = local_day_of(tz, now);
    for (int32_t d = today - 1; d <= today + 14; d++) {     /* -1: a gap can push a late alarm into the next day */
        bool has = false; uint16_t min = 0; bool ov = false;
        if (s->one_day == d) { has = true; min = s->one_min; ov = true; }
        else { const sched_day_t *sd = &s->day[weekday_from_days(d)]; if (sd->on) { has = true; min = sd->min; } }
        if (!has || s->skip_day == d) continue;
        local_kind_t k; int64_t u = tz_local_to_utc(tz, (int64_t)d * 86400 + min * 60, &k);
        if (u > now) { o.valid = true; o.utc = u; o.local_day = d; o.min = min; o.from_override = ov; o.kind = k; return o; }
    }
    return o;
}
void sched_expire(sched_t *s, int32_t today) {
    if (s->skip_day != NO_DAY && s->skip_day < today) s->skip_day = NO_DAY;
    if (s->one_day != NO_DAY && s->one_day < today) s->one_day = NO_DAY;
}
void sched_skip_next(sched_t *s, const tz_t *tz, int64_t now) {
    occurrence_t o = next_alarm(s, tz, now);
    if (o.valid) s->skip_day = o.local_day;
}
void sched_unskip(sched_t *s) { s->skip_day = NO_DAY; }
void sched_set_tomorrow(sched_t *s, const tz_t *tz, int64_t now, uint16_t min) { s->one_day = local_day_of(tz, now) + 1; s->one_min = min; s->skip_day = (s->skip_day == s->one_day) ? NO_DAY : s->skip_day; }
