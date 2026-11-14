#include "civil.h"

int64_t floor_div(int64_t a, int64_t b) { int64_t q = a / b; return (a % b != 0 && ((a < 0) != (b < 0))) ? q - 1 : q; }
bool is_leap(int y) { return (y % 4 == 0 && y % 100 != 0) || y % 400 == 0; }
int days_in_month(int y, int m) { static const int dm[] = {31,28,31,30,31,30,31,31,30,31,30,31}; return (m == 2 && is_leap(y)) ? 29 : dm[m - 1]; }

int64_t days_from_civil(int y, int m, int d) {
    y -= m <= 2;
    int64_t era = (y >= 0 ? y : y - 399) / 400;
    int64_t yoe = y - era * 400;
    int64_t doy = (153 * (m + (m > 2 ? -3 : 9)) + 2) / 5 + d - 1;
    int64_t doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;
    return era * 146097 + doe - 719468;
}
void civil_from_days(int64_t z, int *y, int *m, int *d) {
    z += 719468;
    int64_t era = (z >= 0 ? z : z - 146096) / 146097;
    int64_t doe = z - era * 146097;
    int64_t yoe = (doe - doe / 1460 + doe / 36524 - doe / 146096) / 365;
    int64_t yy = yoe + era * 400;
    int64_t doy = doe - (365 * yoe + yoe / 4 - yoe / 100);
    int64_t mp = (5 * doy + 2) / 153;
    *d = (int)(doy - (153 * mp + 2) / 5 + 1);
    *m = (int)(mp < 10 ? mp + 3 : mp - 9);
    *y = (int)(yy + (*m <= 2));
}
int weekday_from_days(int64_t days) { int64_t w = (days + 3) % 7; if (w < 0) w += 7; return (int)w; }
int64_t civil_to_secs(const civil_t *c) { return days_from_civil(c->y, c->m, c->d) * 86400 + c->hh * 3600 + c->mm * 60 + c->ss; }
void secs_to_civil(int64_t s, civil_t *c) {
    int64_t days = floor_div(s, 86400); int64_t r = s - days * 86400;
    civil_from_days(days, &c->y, &c->m, &c->d);
    c->hh = (int)(r / 3600); c->mm = (int)(r % 3600 / 60); c->ss = (int)(r % 60);
}
