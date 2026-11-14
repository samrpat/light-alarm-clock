/* The touch interface of UX.md, carried over from the concept page (mockups/ux-concept.html).
 * ui_draw.c draws a screen from a plain state struct, so the same code draws real screens, test states for the
 * layout check, and screenshots. ui.c holds the gestures and the state. Nothing here is on the alarm path (R18). */
#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "sched.h"

enum { SCR_CLOCK, SCR_TMR, SCR_WEEK, SCR_DAY, SCR_SOUND, SCR_FOCUS, SCR_LIGHT, SCR_RING, SCR_SET, SCR_COUNT };
enum { PG_TIME, PG_DATE, PG_REGION, PG_PHONE, PG_BEDTIME, PG_BRIGHT, PG_ALARM, PG_ABOUT, PG_COUNT };
enum { DRAG_NONE, DRAG_MIN, DRAG_HR, DRAG_LIN, DRAG_STEP, DRAG_SEC, DRAG_BED };
enum { PICK_NONE, PICK_SND, PICK_TMR };
enum { FPICK_NONE, FPICK_LEN, FPICK_BRK, FPICK_RDS };
#define REG_COUNT 9
#define SND_COUNT 16

typedef struct {
    int scr;
    /* the world: filled in before every draw */
    int now, secs;                      /* local minute of the day, and seconds */
    int wd, y, mo, d;                   /* local date; wd 0 = Monday */
    sched_day_t days[7];
    bool has_next, next_far; int next_wd, next_min;       /* the alarm that will really ring next (from the engine) */
    int eng_state, dawn_t, snz_left;
    char status[40];                    /* the one status line of the clock, already chosen by priority */
    bool status_warn;
    /* Tomorrow screen: the day it edits */
    bool tgt_today; int tgt_usual, tgt_one; bool tgt_skip;  /* minutes or -1 */
    /* interface */
    float aw, hold; int drag, sel; bool done;
    int vol, ti, snd, pick, pp; bool play;
    struct { bool on; int m; int l[2]; } lt;
    int pg, df, clk, sec; bool sec_mode; struct { int m, d, y; } dat; int rg, dayB, ns, ld; bool test;
    struct { int len, brk, rds; bool run; int ph, left, rd; bool menu; int pick; } fo;
    struct { bool present; int pct, mv; } bat;
} ui_t;

void ui_fresh(ui_t *s);                 /* defaults (UX.md) */
void ui_draw(const ui_t *s);            /* draw into the layer given to gfx_begin */
const char *ui_region_name(int i);
const char *ui_sound_name(int i);

void ui_start(void);                    /* core 0 */
/* bench: jump to a screen, run the layout check over many states, print one line per problem */
void ui_bench_goto(const char *args);
int  ui_bench_layoutcheck(void);
