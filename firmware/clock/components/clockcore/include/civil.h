/* Calendar maths. Pure C, 64-bit, no libc time functions (D-003, LONGEVITY.md section 2).
 * Days are counted from 1970-01-01. Weekday 0 = Monday ... 6 = Sunday. */
#pragma once
#include <stdint.h>
#include <stdbool.h>

int64_t days_from_civil(int y, int m, int d);
void    civil_from_days(int64_t days, int *y, int *m, int *d);
int     weekday_from_days(int64_t days);          /* 0=Mon..6=Sun */
bool    is_leap(int y);
int     days_in_month(int y, int m);

typedef struct { int y, m, d, hh, mm, ss; } civil_t;
int64_t civil_to_secs(const civil_t *c);           /* as if the wall clock were UTC */
void    secs_to_civil(int64_t secs, civil_t *c);
int64_t floor_div(int64_t a, int64_t b);
