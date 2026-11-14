#include "store.h"
#include "storage.h"
#include "nvs.h"
#include "esp_log.h"
#include <string.h>
static const char *TAG = "store";
#define NS "alarm"

static bool read_slot(nvs_handle_t h, const char *k, rec_t *r) { size_t n = sizeof *r; memset(r, 0, sizeof *r); return nvs_get_blob(h, k, r, &n) == ESP_OK && n == sizeof *r; }

bool store_load_sched(sched_t *s, bool *damaged) {
    sched_init(s); *damaged = false; nvs_handle_t h;
    if (nvs_open(NS, NVS_READONLY, &h) != ESP_OK) return false;     /* first boot: no namespace yet, empty is correct */
    rec_t a, b; bool ha = read_slot(h, "sched_a", &a), hb = read_slot(h, "sched_b", &b); nvs_close(h);
    if (!ha && !hb) return false;                                   /* never written: a new clock */
    if (!ha) memset(&a, 0, sizeof a);
    if (!hb) memset(&b, 0, sizeof b);
    int p = rec_pick(&a, &b); const rec_t *best = p == 0 ? &a : &b;
    if (p < 0 || !sched_deserialize(s, best->payload, best->len)) { sched_init(s); *damaged = true; ESP_LOGE(TAG, "both schedule copies are bad"); return false; }
    return true;
}
bool store_save_sched(const sched_t *s) {
    nvs_handle_t h; if (nvs_open(NS, NVS_READWRITE, &h) != ESP_OK) return false;
    rec_t a, b; bool ha = read_slot(h, "sched_a", &a), hb = read_slot(h, "sched_b", &b);
    if (!ha) memset(&a, 0, sizeof a);
    if (!hb) memset(&b, 0, sizeof b);
    int best = rec_pick(&a, &b), dst = rec_next_slot(&a, &b); uint32_t seq = best < 0 ? 1 : (best == 0 ? a.seq : b.seq) + 1;
    uint8_t buf[SCHED_BYTES]; sched_serialize(s, buf); rec_t n; rec_pack(&n, SCHED_SCHEMA, seq, buf, SCHED_BYTES);
    const char *key = dst == 0 ? "sched_a" : "sched_b"; bool ok = nvs_set_blob(h, key, &n, sizeof n) == ESP_OK && nvs_commit(h) == ESP_OK;
    if (ok) {                                                       /* read back and check, then it counts as written */
        rec_t v; ok = read_slot(h, key, &v) && rec_valid(&v) && v.seq == seq;
    }
    nvs_close(h); if (!ok) ESP_LOGE(TAG, "schedule write failed"); return ok;
}
void store_load_sm(alarm_sm_t *sm) {
    sm_init(sm); nvs_handle_t h;
    if (nvs_open(NS, NVS_READONLY, &h) != ESP_OK) return;
    size_t n = sizeof *sm; alarm_sm_t t; if (nvs_get_blob(h, "sm", &t, &n) == ESP_OK && n == sizeof t && t.state <= SM_MISSED) *sm = t; nvs_close(h); }
void store_save_sm(const alarm_sm_t *sm) {
    nvs_handle_t h;
    if (nvs_open(NS, NVS_READWRITE, &h) != ESP_OK) return;
    nvs_set_blob(h, "sm", sm, sizeof *sm); nvs_commit(h); nvs_close(h); }
void store_load_region(char *buf, int n) {
    strncpy(buf, "EST5EDT,M3.2.0/2,M11.1.0/2", n); buf[n - 1] = 0;
    nvs_handle_t h;
    if (nvs_open(NS, NVS_READONLY, &h) != ESP_OK) return;
    char t[64]; size_t tl = sizeof t;
    if (nvs_get_str(h, "tz", t, &tl) == ESP_OK) { strncpy(buf, t, n - 1); buf[n - 1] = 0; }
    nvs_close(h);
}
