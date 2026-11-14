#include "storage.h"
#include "civil.h"
#include <string.h>

uint32_t crc32_calc(const void *data, size_t n) {
    const uint8_t *p = data; uint32_t c = 0xFFFFFFFFu;
    while (n--) { c ^= *p++; for (int k = 0; k < 8; k++) c = (c >> 1) ^ (0xEDB88320u & -(c & 1)); }
    return ~c;
}
static uint32_t rec_crc(const rec_t *r) { return crc32_calc(r, offsetof(rec_t, crc)); }
void rec_pack(rec_t *r, uint16_t schema, uint32_t seq, const void *payload, uint16_t len) {
    memset(r, 0, sizeof *r); r->magic = REC_MAGIC; r->schema = schema; r->seq = seq;
    if (len > REC_MAX_PAYLOAD) len = REC_MAX_PAYLOAD;
    r->len = len; memcpy(r->payload, payload, len); r->crc = rec_crc(r);
}
bool rec_valid(const rec_t *r) { return r->magic == REC_MAGIC && r->len <= REC_MAX_PAYLOAD && r->crc == rec_crc(r); }
int rec_pick(const rec_t *a, const rec_t *b) {
    bool va = rec_valid(a), vb = rec_valid(b);
    if (va && vb) return (int32_t)(b->seq - a->seq) > 0 ? 1 : 0;        /* wrap-safe */
    if (va) return 0;
    if (vb) return 1;
    return -1;
}
int rec_next_slot(const rec_t *a, const rec_t *b) { int best = rec_pick(a, b); return best < 0 ? 0 : 1 - best; }

static void put16(uint8_t *p, uint16_t v) { p[0] = (uint8_t)v; p[1] = (uint8_t)(v >> 8); }
static uint16_t get16(const uint8_t *p) { return (uint16_t)(p[0] | (p[1] << 8)); }
static void put32(uint8_t *p, int32_t v) { for (int i = 0; i < 4; i++) p[i] = (uint8_t)((uint32_t)v >> (8 * i)); }
static int32_t get32(const uint8_t *p) { uint32_t v = 0; for (int i = 0; i < 4; i++) v |= (uint32_t)p[i] << (8 * i); return (int32_t)v; }
/* layout: 7 x (u16 minutes with bit 15 = on) | i32 skip_day | i32 one_day | u16 one_min  = 14 + 4 + 4 + 2 = 24, padded to 30 */
void sched_serialize(const sched_t *s, uint8_t o[SCHED_BYTES]) {
    memset(o, 0, SCHED_BYTES);
    for (int i = 0; i < 7; i++) put16(o + 2 * i, (uint16_t)(s->day[i].min | (s->day[i].on ? 0x8000 : 0)));
    put32(o + 14, s->skip_day); put32(o + 18, s->one_day); put16(o + 22, s->one_min);
}
bool sched_deserialize(sched_t *s, const uint8_t in[SCHED_BYTES], uint16_t len) {
    if (len != SCHED_BYTES) return false;
    sched_t t;
    for (int i = 0; i < 7; i++) { uint16_t v = get16(in + 2 * i); t.day[i].on = v & 0x8000; t.day[i].min = v & 0x7FFF; if (t.day[i].min >= 1440) return false; }
    t.skip_day = get32(in + 14); t.one_day = get32(in + 18); t.one_min = get16(in + 22);
    if (t.one_min >= 1440) return false;
    *s = t; return true;
}
static int bcd(uint8_t v) { return (v >> 4) * 10 + (v & 15); }
static uint8_t tobcd(int v) { return (uint8_t)(((v / 10) << 4) | (v % 10)); }
bool pcf85063_decode(const uint8_t r[7], int64_t *utc, bool *osc) {
    *osc = (r[0] & 0x80) != 0;
    int ss = bcd(r[0] & 0x7F), mi = bcd(r[1] & 0x7F), hh = bcd(r[2] & 0x3F), dd = bcd(r[3] & 0x3F), mo = bcd(r[5] & 0x1F), yy = 2000 + bcd(r[6]);
    if (ss > 59 || mi > 59 || hh > 23 || mo < 1 || mo > 12 || dd < 1 || dd > days_in_month(yy, mo)) return false;
    civil_t c = {yy, mo, dd, hh, mi, ss}; *utc = civil_to_secs(&c); return true;
}
void pcf85063_encode(int64_t utc, uint8_t r[7]) {
    civil_t c; secs_to_civil(utc, &c);
    r[0] = tobcd(c.ss); r[1] = tobcd(c.mm); r[2] = tobcd(c.hh); r[3] = tobcd(c.d);
    r[4] = (uint8_t)((weekday_from_days(floor_div(utc, 86400)) + 1) % 7);       /* 0 = Sunday on the chip; never trusted on read */
    r[5] = tobcd(c.m); r[6] = tobcd(c.y - 2000);
}
float drift_ppm(int64_t elapsed, int64_t err) {
    if (elapsed < 7 * 86400) return 0;
    float ppm = (float)err * 1e6f / (float)elapsed;
    return (ppm > 200 || ppm < -200) ? 0 : ppm;
}
