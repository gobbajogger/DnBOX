/* SPDX-License-Identifier: GPL-3.0-only */
/* Drum voice pans (X0X): every voice centred, drum909_render_st / drum808_render_st give both sides
 * exactly the mono render's dry (and the same sends); a voice panned past the clip (raw 0) plays exactly
 * as L50 (raw 14), the right side at the L50 law's far gain. */
#include <stdio.h>
#include <string.h>
#include "drum909.h"
#include "drum808.h"
#include "x0x_param.h"

#define N 256
static int fails;
static void check(int ok, const char *what)
{
    printf("  %s %s\n", ok ? "ok  " : "FAIL", what);
    fails += !ok;
}
static drum909_t a9, b9;
static drum808_t a8, b8;

int main(void)
{
    static float d[N], r[N], y[N], l[N], rr[N], r2[N], y2[N];
    int b, i, v, same = 1, quiet = 1, loud = 0;
    drum909_init(&a9);
    drum909_init(&b9);
    for (b = 0; b < 300; b++) {
        if (b % 25 == 0)
            for (v = 0; v < DR_NUM; v++) {
                drum909_trigger(&a9, v, 1.0f);
                drum909_trigger(&b9, v, 1.0f);
            }
        memset(d, 0, sizeof d); memset(r, 0, sizeof r); memset(y, 0, sizeof y);
        memset(l, 0, sizeof l); memset(rr, 0, sizeof rr); memset(r2, 0, sizeof r2); memset(y2, 0, sizeof y2);
        drum909_render(&a9, d, r, y, N);
        drum909_render_st(&b9, l, rr, r2, y2, N);
        same &= !memcmp(d, l, sizeof d) && !memcmp(d, rr, sizeof d) && !memcmp(r, r2, sizeof r) && !memcmp(y, y2, sizeof y);
    }
    check(same, "909: every voice centred: stereo == mono, bit for bit");
    drum808_init(&a8);
    drum808_init(&b8);
    same = 1;
    for (b = 0; b < 300; b++) {
        if (b % 25 == 0)
            for (v = 0; v < D8_NUM; v++) {
                drum808_trigger(&a8, v, 1.0f);
                drum808_trigger(&b8, v, 1.0f);
            }
        memset(d, 0, sizeof d); memset(r, 0, sizeof r); memset(y, 0, sizeof y);
        memset(l, 0, sizeof l); memset(rr, 0, sizeof rr); memset(r2, 0, sizeof r2); memset(y2, 0, sizeof y2);
        drum808_render(&a8, d, r, y, N);
        drum808_render_st(&b8, l, rr, r2, y2, N);
        same &= !memcmp(d, l, sizeof d) && !memcmp(d, rr, sizeof d) && !memcmp(r, r2, sizeof r) && !memcmp(y, y2, sizeof y);
    }
    check(same, "808: every track centred: stereo == mono, bit for bit");
    /* hard left past the clip: the snare (909) and the clap (808) at raw 0 play bit for bit as L50 (raw 14) */
    {
        static float l2[N], rr2[N], r3[N], y3[N];
        double el = 0.0, er = 0.0;
        float gl, gr;
        int clip = 1;
        drum909_init(&b9); drum909_init(&a9);
        drum909_set(&b9, DR_SD, drum909_nparams(DR_SD) - 1, 0);
        drum909_set(&a9, DR_SD, drum909_nparams(DR_SD) - 1, X0X_PAN_MIN);
        drum808_init(&b8); drum808_init(&a8);
        drum808_set(&b8, D8_CP, drum808_nparams(D8_CP) - 1, 0);
        drum808_set(&a8, D8_CP, drum808_nparams(D8_CP) - 1, X0X_PAN_MIN);
        drum909_pan_settle(&b9); drum909_pan_settle(&a9);   /* as the engine does while the kit is silent */
        drum808_pan_settle(&b8); drum808_pan_settle(&a8);
        x0x_pan_gains(X0X_PAN_MIN, &gl, &gr);
        for (b = 0; b < 40; b++) {
            if (b == 0) {
                drum909_trigger(&b9, DR_SD, 1.0f); drum909_trigger(&a9, DR_SD, 1.0f);
                drum808_trigger(&b8, D8_CP, 1.0f); drum808_trigger(&a8, D8_CP, 1.0f);
            }
            memset(l, 0, sizeof l); memset(rr, 0, sizeof rr); memset(r2, 0, sizeof r2); memset(y2, 0, sizeof y2);
            memset(l2, 0, sizeof l2); memset(rr2, 0, sizeof rr2); memset(r3, 0, sizeof r3); memset(y3, 0, sizeof y3);
            drum909_render_st(&b9, l, rr, r2, y2, N);
            drum808_render_st(&b8, l, rr, r2, y2, N);
            drum909_render_st(&a9, l2, rr2, r3, y3, N);
            drum808_render_st(&a8, l2, rr2, r3, y3, N);
            clip &= !memcmp(l, l2, sizeof l) && !memcmp(rr, rr2, sizeof rr) && !memcmp(r2, r3, sizeof r2) && !memcmp(y2, y3, sizeof y2);
            for (i = 0; i < N; i++) {
                el += (double)l[i] * l[i];
                er += (double)rr[i] * rr[i];
                loud |= l[i] > 0.01f || l[i] < -0.01f;
            }
        }
        quiet = el > 0.0 && er > 0.0 && er < el && gl == 1.0f && gr > 0.0f && gr < 1.0f;
        check(clip && loud, "pan past the clip (raw 0) == L50 (raw 14), bit for bit");
        check(quiet, "pan L50: the left leads, the right at the L50 law's far gain (not past it)");
    }
    printf(fails ? "drum pan: %d FAILED\n" : "drum pan: ok\n", fails);
    return fails != 0;
}
