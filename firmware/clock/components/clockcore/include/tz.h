/* Time zones as formulas, not tables (D-003, D-031). A POSIX TZ string such as
 * "EST5EDT,M3.2.0/2,M11.1.0/2" is parsed once. Works for any year, no network. */
#pragma once
#include <stdint.h>
#include <stdbool.h>

typedef struct { uint8_t m, w, d; int32_t t; } tz_rule_t;   /* month, week 1..5 (5=last), day 0=Sun, seconds after midnight */
typedef struct {
    int32_t std_east, dst_east;   /* seconds east of UTC */
    bool has_dst;
    tz_rule_t start, end;
} tz_t;

typedef enum { LOCAL_OK = 0, LOCAL_GAP = 1, LOCAL_AMBIGUOUS = 2 } local_kind_t;

bool    tz_parse(const char *s, tz_t *tz);
int64_t tz_transition_start_utc(const tz_t *tz, int year);   /* spring forward (UTC instant) */
int64_t tz_transition_end_utc(const tz_t *tz, int year);     /* fall back (UTC instant) */
bool    tz_is_dst(const tz_t *tz, int64_t utc);
int32_t tz_offset_at_utc(const tz_t *tz, int64_t utc);       /* seconds east */
int64_t tz_utc_to_local(const tz_t *tz, int64_t utc);        /* local wall clock as seconds-since-epoch */
/* Local wall clock to UTC. GAP: returns the first valid instant after the gap. AMBIGUOUS: first occurrence. */
int64_t tz_local_to_utc(const tz_t *tz, int64_t local, local_kind_t *kind);

/* The built-in regions of the Region page (UX.md). Index 8 = Manual (fixed offset, no DST). */
#define TZ_REGION_COUNT 9
extern const char *const TZ_REGION_STR[TZ_REGION_COUNT];
