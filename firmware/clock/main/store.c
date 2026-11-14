#include "store.h"
#include "storage.h"
#include "nvs.h"
#include "esp_log.h"
#include <string.h>
static const char *TAG = "store";
#define NS "alarm"

static bool read_slot(nvs_handle_t h, const char *k, rec_t *r) {
    size_t n = sizeof *r; memset(r, 0, sizeof *r);
    return nvs_get_blob(h, k, r, &n) == ESP_OK && n == sizeof *r;
}
bool store_load_sched(sched_t *s, bool *damaged) {
    sched_init(s); *damaged = false; nvs_handle_t h;
    if (nvs_open(NS, NVS_READONLY, &h) != ESP_OK) return false;          /* first boot: empty is correct */
    rec_t a, b; bool ha = read_slot(h, "sched_a", &a), hb = read_slot(h, "sched_b", &b); nvs_close(h);
    if (!ha && !hb) return false;                                        /* never written: a new clock */
    bool fell_back = false;
    if (!sched_load_ab(&a, &b, s, &fell_back)) { *damaged = true; ESP_LOGE(TAG, "no usable schedule copy: empty schedule, warning shown"); return false; }
    if (fell_back) ESP_LOGW(TAG, "newest schedule copy was bad: using the older good one");
    return true;
}
bool store_save_sched(const sched_t *s) {
    if (!sched_valid(s)) { ESP_LOGE(TAG, "refused to store an out-of-range schedule"); return false; }
    nvs_handle_t h;
    if (nvs_open(NS, NVS_READWRITE, &h) != ESP_OK) return false;
    rec_t a, b; read_slot(h, "sched_a", &a); read_slot(h, "sched_b", &b);
    int dst = sched_next_slot(&a, &b); uint32_t seq = sched_next_seq(&a, &b);
    uint8_t buf[SCHED_BYTES]; sched_serialize(s, buf); rec_t n; rec_pack(&n, SCHED_SCHEMA, seq, buf, SCHED_BYTES);
    const char *key = dst == 0 ? "sched_a" : "sched_b";
    bool ok = nvs_set_blob(h, key, &n, sizeof n) == ESP_OK && nvs_commit(h) == ESP_OK;
    if (ok) { rec_t v; ok = read_slot(h, key, &v) && rec_valid(&v) && v.seq == seq; }      /* read back, then it counts */
    nvs_close(h);
    if (!ok) ESP_LOGE(TAG, "schedule write failed");
    return ok;
}
void store_load_sm(alarm_sm_t *sm) {
    sm_init(sm); nvs_handle_t h;
    if (nvs_open(NS, NVS_READONLY, &h) != ESP_OK) return;
    size_t n = sizeof *sm; alarm_sm_t t;
    if (nvs_get_blob(h, "sm", &t, &n) == ESP_OK && n == sizeof t && t.state <= SM_MISSED) *sm = t;
    nvs_close(h);
}
void store_save_sm(const alarm_sm_t *sm) {
    nvs_handle_t h;
    if (nvs_open(NS, NVS_READWRITE, &h) != ESP_OK) return;
    nvs_set_blob(h, "sm", sm, sizeof *sm); nvs_commit(h); nvs_close(h);
}
void store_load_region(char *buf, int n) {
    strncpy(buf, "EST5EDT,M3.2.0/2,M11.1.0/2", n); buf[n - 1] = 0;
    nvs_handle_t h;
    if (nvs_open(NS, NVS_READONLY, &h) != ESP_OK) return;
    char t[64]; size_t tl = sizeof t;
    if (nvs_get_str(h, "tz", t, &tl) == ESP_OK) { strncpy(buf, t, n - 1); buf[n - 1] = 0; }
    nvs_close(h);
}
int64_t store_get_i64(const char *key, int64_t def) {
    nvs_handle_t h; int64_t v = def;
    if (nvs_open(NS, NVS_READONLY, &h) != ESP_OK) return def;
    if (nvs_get_i64(h, key, &v) != ESP_OK) v = def;
    nvs_close(h); return v;
}
void store_set_i64(const char *key, int64_t v) {
    nvs_handle_t h;
    if (nvs_open(NS, NVS_READWRITE, &h) != ESP_OK) return;
    nvs_set_i64(h, key, v); nvs_commit(h); nvs_close(h);
}
void store_dump(void) {
    nvs_handle_t h;
    if (nvs_open(NS, NVS_READONLY, &h) != ESP_OK) { ESP_LOGW(TAG, "dump: no namespace"); return; }
    const char *keys[2] = {"sched_a", "sched_b"};
    for (int i = 0; i < 2; i++) {
        rec_t r; bool ok = read_slot(h, keys[i], &r); sched_t t; bool des = ok && sched_deserialize(&t, r.payload, r.len);
        ESP_LOGW(TAG, "DUMP %s present=%d seq=%u crc_ok=%d usable=%d", keys[i], ok, (unsigned)r.seq, ok && rec_valid(&r), des && rec_valid(&r));
    }
    nvs_close(h);
}
