/* SPDX-License-Identifier: GPL-3.0-only */
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "dsp/drum808.h"
/* The reference wrapper owns the historic engine layout. */
void baseline_init(void);
void baseline_trigger(int track, float velocity);
void baseline_render(float *left, float *right, float *reverb, float *delay, int n);
static drum808_t b;
int main(void)
{
    for (int track = 0; track < D8_NUM; ++track) {
        baseline_init();
        drum808_init(&b);
        int saw_meter = 0;
        for (int block = 0; block < 1200; ++block) {
            float l[256] = {0}, r[256] = {0}, rev[256] = {0}, dly[256] = {0};
            float cl[256] = {0}, cr[256] = {0}, cv[256] = {0}, cd[256] = {0};
            int sizes[] = {0, 1, 31, 32, 33, 63, 64, 127, 256};
            int n = sizes[block % 9];
            /* Initial silence, full tail to idle, then a lower velocity restart. */
            if (block == 4 || block == 1100) {
                float velocity = block == 4 ? 1.0f : 0.3f;
                baseline_trigger(track, velocity);
                drum808_trigger(&b, track, velocity);
            }
            baseline_render(l, r, rev, dly, n);
            drum808_render_st(&b, cl, cr, cv, cd, n);
            float level = drum808_env(&b, track);
            assert(level >= 0.0f && level <= 1.0f);
            if (level > 0.0f) saw_meter = 1;
            if (block < 4) assert(level == 0.0f);
            assert(!memcmp(l, cl, sizeof l) && !memcmp(r, cr, sizeof r));
            assert(!memcmp(rev, cv, sizeof rev) && !memcmp(dly, cd, sizeof dly));
        }
        assert(saw_meter);
    }
    puts("808 natural release and retrigger stereo/send equivalence: PASS");
}
