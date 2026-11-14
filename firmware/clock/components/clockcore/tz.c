#include "tz.h"
#include "civil.h"
#include <string.h>
#include <ctype.h>

const char *const TZ_REGION_STR[TZ_REGION_COUNT] = {
    "EST5EDT,M3.2.0/2,M11.1.0/2", "CST6CDT,M3.2.0/2,M11.1.0/2", "MST7MDT,M3.2.0/2,M11.1.0/2",
    "PST8PDT,M3.2.0/2,M11.1.0/2", "AKST9AKDT,M3.2.0/2,M11.1.0/2", "HST10",
    "GMT0BST,M3.5.0/1,M10.5.0/2", "CET-1CEST,M3.5.0/2,M10.5.0/3", "UTC0"};

static bool name(const char **p) {
    const char *s = *p;
    if (*s == '<') { s++; while (*s && *s != '>') s++; if (*s != '>') return false; *p = s + 1; return true; }
    int n = 0; while (isalpha((unsigned char)*s)) { s++; n++; }
    if (n < 3) return false;
    *p = s; return true;
}
static bool hms(const char **p, int32_t *out) {   /* [+-]hh[:mm[:ss]] */
    const char *s = *p; int sign = 1;
    if (*s == '+') s++; else if (*s == '-') { sign = -1; s++; }
    if (!isdigit((unsigned char)*s)) return false;
    int v[3] = {0, 0, 0}, i = 0;
    for (;;) {
        int n = 0, x = 0; while (isdigit((unsigned char)*s) && n < 3) { x = x * 10 + (*s++ - '0'); n++; }
        v[i++] = x; if (*s == ':' && i < 3) s++; else break;
    }
    *out = sign * (v[0] * 3600 + v[1] * 60 + v[2]); *p = s; return true;
}
static bool rule(const char **p, tz_rule_t *r) {
    const char *s = *p;
    if (*s++ != 'M') return false;
    int m = 0, w = 0, d = 0;
    if (!isdigit((unsigned char)*s)) return false;
    while (isdigit((unsigned char)*s)) m = m * 10 + (*s++ - '0');
    if (*s++ != '.') return false;
    if (!isdigit((unsigned char)*s)) return false;
    w = *s++ - '0';
    if (*s++ != '.') return false;
    if (!isdigit((unsigned char)*s)) return false;
    d = *s++ - '0';
    if (m < 1 || m > 12 || w < 1 || w > 5 || d > 6) return false;
    r->m = (uint8_t)m; r->w = (uint8_t)w; r->d = (uint8_t)d; r->t = 7200;
    if (*s == '/') { s++; if (!hms(&s, &r->t)) return false; }
    *p = s; return true;
}
bool tz_parse(const char *s, tz_t *tz) {
    memset(tz, 0, sizeof *tz);
    if (!s || !name(&s)) return false;
    int32_t off; if (!hms(&s, &off)) return false;
    tz->std_east = -off; tz->dst_east = tz->std_east;
    if (*s == 0) return true;
    if (!name(&s)) return false;
    tz->has_dst = true; tz->dst_east = tz->std_east + 3600;
    if (*s && *s != ',') { if (!hms(&s, &off)) return false; tz->dst_east = -off; }
    if (*s == ',') {
        s++; if (!rule(&s, &tz->start) || *s++ != ',' || !rule(&s, &tz->end)) return false;
    } else { tz->start = (tz_rule_t){3, 2, 0, 7200}; tz->end = (tz_rule_t){11, 1, 0, 7200}; }
    return *s == 0;
}
static int64_t rule_secs(const tz_rule_t *r, int y) {
    int64_t first = days_from_civil(y, r->m, 1);
    int wd_first_sun0 = (weekday_from_days(first) + 1) % 7;            /* 0=Sun */
    int day = 1 + ((r->d - wd_first_sun0) + 7) % 7 + (r->w - 1) * 7;
    while (day > days_in_month(y, r->m)) day -= 7;
    return (first + (day - 1)) * 86400 + r->t;
}
int64_t tz_transition_start_utc(const tz_t *tz, int y) { return rule_secs(&tz->start, y) - tz->std_east; }
int64_t tz_transition_end_utc(const tz_t *tz, int y)   { return rule_secs(&tz->end, y) - tz->dst_east; }
static int year_of(int64_t secs) { int y, m, d; civil_from_days(floor_div(secs, 86400), &y, &m, &d); return y; }
bool tz_is_dst(const tz_t *tz, int64_t utc) {
    if (!tz->has_dst) return false;
    int y = year_of(utc + tz->std_east);
    int64_t s = tz_transition_start_utc(tz, y), e = tz_transition_end_utc(tz, y);
    return s < e ? (utc >= s && utc < e) : (utc >= s || utc < e);
}
int32_t tz_offset_at_utc(const tz_t *tz, int64_t utc) { return tz_is_dst(tz, utc) ? tz->dst_east : tz->std_east; }
int64_t tz_utc_to_local(const tz_t *tz, int64_t utc) { return utc + tz_offset_at_utc(tz, utc); }
int64_t tz_local_to_utc(const tz_t *tz, int64_t local, local_kind_t *kind) {
    local_kind_t k = LOCAL_OK; int64_t u;
    if (!tz->has_dst) u = local - tz->std_east;
    else {
        int64_t us = local - tz->std_east, ud = local - tz->dst_east;
        bool vs = !tz_is_dst(tz, us), vd = tz_is_dst(tz, ud);
        if (vs && vd) { k = LOCAL_AMBIGUOUS; u = ud < us ? ud : us; }
        else if (vs) u = us; else if (vd) u = ud;
        else { k = LOCAL_GAP; u = tz_transition_start_utc(tz, year_of(local)); }
    }
    if (kind) *kind = k;
    return u;
}
