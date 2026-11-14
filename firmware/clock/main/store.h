#pragma once
#include <stdbool.h>
#include <stdint.h>
#include "sched.h"
#include "alarm_sm.h"
/* NVS A/B records with CRC (D-004, R17). */
bool store_load_sched(sched_t *s, bool *damaged);   /* false + *damaged: copies exist but none is usable (D-015 warning) */
bool store_save_sched(const sched_t *s);            /* refuses anything out of range; read back and checked */
void store_load_sm(alarm_sm_t *sm);
void store_save_sm(const alarm_sm_t *sm);
void store_load_region(char *buf, int n);
int64_t store_get_i64(const char *key, int64_t def);
void store_set_i64(const char *key, int64_t v);
void store_dump(void);
