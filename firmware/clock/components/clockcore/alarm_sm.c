#include "alarm_sm.h"

void sm_default_cfg(sm_cfg_t *c) { c->sunrise_s = 1800; c->stage_s = 60; c->auto_stop_s = 1800; c->snooze_s = 540; c->max_snooze = 3; c->catchup_s = 3600; c->missed_hold_s = 3600; }
void sm_init(alarm_sm_t *sm) { sm->state = SM_IDLE; sm->snoozes = 0; sm->armed = 0; sm->handled = 0; sm->ring_t0 = 0; sm->snooze_until = 0; }

static void end_occurrence(alarm_sm_t *sm, sm_out_t *o) { sm->handled = sm->armed; sm->armed = 0; sm->state = SM_IDLE; sm->snoozes = 0; o->ended = true; o->ended_occ = sm->handled; o->persist = true; }
/* after the alarm time the light goes on into the 10-minute daylight stage (LIGHT.md) and holds there */
static int32_t after(int64_t d) { return d < 0 ? 0 : d > 600 ? 600 : (int32_t)d; }
static int stage_of(int64_t el, int32_t step) { int s = 1 + (int)(el / step); return s > ALARM_STAGES ? ALARM_STAGES : s; }

sm_out_t sm_step(alarm_sm_t *sm, const sm_cfg_t *c, int64_t now, int64_t next, sm_event_t ev) {
    sm_out_t o = {0};
    if (ev == EV_TIME_SET) {            /* the clock was set by hand: nothing "missed" in the jump */
        if (sm->state == SM_IDLE) { sm->handled = now; sm->armed = 0; o.persist = true; }
    }
    /* The user edited the schedule: a future occurrence that was dismissed early may be asked for again. The engine
     * keeps it from ringing twice by marking that day skipped in the schedule itself. */
    if (ev == EV_SCHED_CHANGED && sm->handled > now) { sm->handled = 0; o.persist = true; }
    /* IDLE: follow the schedule, then decide whether the dawn has begun or the alarm was missed while off */
    if (sm->state == SM_IDLE) {
        if (sm->armed == 0 || sm->armed == sm->handled || (sm->armed > now && next != sm->armed)) {
            /* `handled` only stops the very same occurrence from coming back (dismissed early in the dawn). It is NOT a
             * high-water mark: a new alarm earlier than a cancelled one must still ring (found on the board). */
            int64_t n = next; if (n && n == sm->handled) n = 0;
            if (sm->armed != n) { sm->armed = n; o.persist = true; }
        }
        if (sm->armed && sm->armed != sm->handled) {
            if (now >= sm->armed) {                              /* due: ring, or catch up after a reset */
                if (now - sm->armed <= c->catchup_s) { sm->state = SM_RINGING; sm->ring_t0 = (now - sm->armed > 5) ? now : sm->armed; sm->snoozes = 0; o.persist = true; }   /* late: start soft, not at full volume */
                else { sm->handled = sm->armed; sm->armed = 0; o.persist = true; }
            } else if (now >= sm->armed - c->sunrise_s) { sm->state = SM_SUNRISE; o.persist = true; }
        }
    }
    if (sm->state == SM_SUNRISE) {
        if (ev == EV_DISMISS) { end_occurrence(sm, &o); }
        else if (now >= sm->armed) { sm->state = SM_RINGING; sm->ring_t0 = (now - sm->armed > 5) ? now : sm->armed; o.persist = true; }   /* power back late: start soft */
        else if (next != sm->armed) {
            /* The schedule changed during the dawn. An EARLIER alarm takes over (it must never be dropped);
             * anything else (switched off, skipped, moved later) cancels this dawn. */
            if (next != 0 && next < sm->armed) { sm->armed = next; o.persist = true; if (now >= sm->armed) { sm->state = SM_RINGING; sm->ring_t0 = (now - sm->armed > 5) ? now : sm->armed; } }
            else end_occurrence(sm, &o);
        }      /* alarm switched off or skipped during the dawn */
    }
    if (sm->state == SM_RINGING) {
        if (ev == EV_DISMISS) { end_occurrence(sm, &o); }
        else if (ev == EV_SNOOZE && sm->snoozes < c->max_snooze) { sm->snoozes++; sm->state = SM_SNOOZED; sm->snooze_until = now + c->snooze_s; o.persist = true; }
        else if (now - sm->ring_t0 >= c->auto_stop_s) { sm->state = SM_MISSED; o.persist = true; }
    } else if (sm->state == SM_SNOOZED) {
        if (ev == EV_DISMISS) end_occurrence(sm, &o);
        else if (now >= sm->snooze_until) { sm->state = SM_RINGING; sm->ring_t0 = now; o.persist = true; }
    }
    if (sm->state == SM_MISSED) {
        if (ev == EV_DISMISS || now - sm->armed >= c->auto_stop_s + c->missed_hold_s) end_occurrence(sm, &o);
    }
    o.state = (sm_state_t)sm->state; o.snoozes_left = c->max_snooze - sm->snoozes;
    switch (sm->state) {
    case SM_SUNRISE: o.light_on = true; o.dawn_t_s = (int32_t)(now - sm->armed); break;
    case SM_RINGING: o.light_on = true; o.dawn_t_s = after(now - sm->armed); o.sound_stage = stage_of(now - sm->ring_t0, c->stage_s); o.ring_elapsed_s = (int32_t)(now - sm->ring_t0); break;
    case SM_SNOOZED: o.light_on = true; o.dawn_t_s = after(now - sm->armed); break;
    case SM_MISSED:  o.light_on = true; o.dawn_t_s = after(now - sm->armed); o.missed = true; break;
    default: break;
    }
    return o;
}
