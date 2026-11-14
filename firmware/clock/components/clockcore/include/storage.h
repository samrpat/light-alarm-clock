/* Integrity helpers: CRC-32, A/B records (D-004, R17), schedule serialisation, PCF85063 decode, drift. */
#pragma once
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "sched.h"

uint32_t crc32_calc(const void *data, size_t n);

#define REC_MAGIC 0xA1A4C10Cu
#define REC_MAX_PAYLOAD 64
typedef struct { uint32_t magic; uint16_t schema; uint16_t len; uint32_t seq; uint8_t payload[REC_MAX_PAYLOAD]; uint32_t crc; } rec_t;
void rec_pack(rec_t *r, uint16_t schema, uint32_t seq, const void *payload, uint16_t len);
bool rec_valid(const rec_t *r);
/* Choose the valid record with the highest seq. Returns 0 = A, 1 = B, -1 = both bad. */
int  rec_pick(const rec_t *a, const rec_t *b);
/* Slot to write next: the one that is not the current best (so one good copy always exists). */
int  rec_next_slot(const rec_t *a, const rec_t *b);

#define SCHED_SCHEMA 1
#define SCHED_BYTES 30
void sched_serialize(const sched_t *s, uint8_t out[SCHED_BYTES]);
bool sched_deserialize(sched_t *s, const uint8_t in[SCHED_BYTES], uint16_t len);
bool sched_valid(const sched_t *s);                 /* every field in range: nothing else may be written */
/* Load from the A/B pair. Tries the newest copy first; a copy counts only if its CRC is good AND its contents are a
 * valid schedule, otherwise the other copy is used (R17). Returns false if neither is usable (schedule left empty).
 * *fell_back is set when the newest copy had to be passed over. */
bool sched_load_ab(const rec_t *a, const rec_t *b, sched_t *out, bool *fell_back);
/* Where and with which number to write next, so the one usable copy is never the one overwritten. */
int  sched_next_slot(const rec_t *a, const rec_t *b);
uint32_t sched_next_seq(const rec_t *a, const rec_t *b);

/* PCF85063A registers 0x04..0x0A: seconds, minutes, hours, days, weekdays, months, years (BCD).
 * Bit 7 of seconds is the oscillator-stop flag: time is not trusted (D-003, R15). */
bool pcf85063_decode(const uint8_t r[7], int64_t *utc, bool *osc_stopped);
void pcf85063_encode(int64_t utc, uint8_t r[7]);

/* Drift learning (LONGEVITY.md section 2): after a correction of err_s seconds (device was behind if positive)
 * over elapsed_s, returns the drift in parts per million, or 0 if the interval is too short (under 7 days)
 * or the result is implausible (over 200 ppm). */
float drift_ppm(int64_t elapsed_s, int64_t err_s);
