#pragma once
#include <stdbool.h>
#include "sched.h"
#include "alarm_sm.h"
/* NVS A/B records with CRC (D-004, R17). Returns false if both copies are bad: the schedule is then
 * empty and *damaged is set so the UI shows a persistent warning (D-015). */
bool store_load_sched(sched_t *s, bool *damaged);
bool store_save_sched(const sched_t *s);
void store_load_sm(alarm_sm_t *sm);
void store_save_sm(const alarm_sm_t *sm);
void store_load_region(char *buf, int n);
