/* SPDX-License-Identifier: GPL-3.0-only */
/* The REVERB bank's two knobs past Decay/Tone (Guidelines §3): Rev Low Cut filters the wet
 * INPUT, Rev Return scales the wet OUTPUT only: never the dry, never the stored tail. */
#include <math.h>
#include <stdio.h>
#include <string.h>
#include "../../firmware/src/dsp/fxbus.h"
static fxbus_t a, b;
static int16_t da[8192], db[8192];
static int fails;
#define CHECK(c, ...) do { if (!(c)) { fails++; printf("FAIL: " __VA_ARGS__); printf("\n"); } } while (0)
static void pair(void) { fxbus_init(&a, da, 8192); fxbus_init(&b, db, 8192); }
static float sine(int i, float hz) { return 0.4f * sinf(6.2831853f * hz * (float)i / D9_SR); }
/* run both buses over the same signals; returns the energy of each out (after warm-up) */
static void run(float dry_hz, float rev_hz, int burst, double *ea, double *eb, double *maxdiff, int *tail_same)
{
    float dry[256], rev[256], z[256], oa[256], ob[256];
    memset(z, 0, sizeof z);
    *ea = *eb = *maxdiff = 0; *tail_same = 1;
    for (int blk = 0; blk < 64; blk++) {
        for (int i = 0; i < 256; i++) {
            int t = blk * 256 + i;
            dry[i] = dry_hz > 0 ? sine(t, dry_hz) : 0.0f;
            rev[i] = rev_hz > 0 && (!burst || t < burst) ? sine(t, rev_hz) : 0.0f;
        }
        fxbus_process(&a, dry, rev, z, oa, 256);
        fxbus_process(&b, dry, rev, z, ob, 256);
        for (int i = 0; i < 256; i++) {
            if (blk >= 8) { *ea += (double)oa[i] * oa[i]; *eb += (double)ob[i] * ob[i]; }
            double d = fabs((double)oa[i] - ob[i]); if (d > *maxdiff) *maxdiff = d;
        }
        if (memcmp(a.comb, b.comb, sizeof a.comb) || memcmp(a.ap, b.ap, sizeof a.ap)) *tail_same = 0;
    }
}
int main(void)
{
    double ea, eb, md; int same;
    /* 1. dry is untouched by either knob: (dry + wet) - (wet alone) == dry * volume at 0 and 127 */
    for (int k = 0; k < 2; k++) for (int pot = 0; pot <= 127; pot += 127) {
        int prm = k ? FX_RV_HPF : FX_RV_LEVEL;
        float dry[256], rev[256], z[256], oa[256], ob[256]; double worst = 0;
        memset(z, 0, sizeof z);
        pair(); fxbus_set(&a, prm, pot); fxbus_set(&b, prm, pot);
        for (int blk = 0; blk < 32; blk++) {
            for (int i = 0; i < 256; i++) { dry[i] = sine(blk * 256 + i, 200.0f); rev[i] = sine(blk * 256 + i, 1000.0f); }
            fxbus_process(&a, dry, rev, z, oa, 256);
            fxbus_process(&b, z, rev, z, ob, 256);
            for (int i = 0; i < 256; i++) { double d = fabs((double)oa[i] - ob[i] - (double)dry[i] * a.volume); if (d > worst) worst = d; }
        }
        CHECK(worst < 1e-5, "%s at %d changes the dry signal (max error %g)", k ? "Rev Low Cut" : "Rev Return", pot, worst);
    }
    /* 2. Return scales the wet output in proportion and leaves the stored tail alone */
    pair(); fxbus_set(&a, FX_RV_LEVEL, 127); fxbus_set(&b, FX_RV_LEVEL, 32);
    {
        float ga = a.rv_level, gb = b.rv_level;
        run(0, 1000.0f, 4096, &ea, &eb, &md, &same);
        double want = (double)(gb / ga) * (gb / ga);
        CHECK(ea > 1e-6 && fabs(eb / ea - want) < 0.02 * want, "Rev Return: wet energy ratio %g, want %g", ea > 0 ? eb / ea : 0, want);
        CHECK(same, "Rev Return changed the reverb's stored tail (comb/allpass lines differ)");
    }
    /* 2b. Return 0 silences the wet but keeps the tail ringing for when it comes back */
    pair(); fxbus_set(&b, FX_RV_LEVEL, 0);
    run(0, 1000.0f, 4096, &ea, &eb, &md, &same);
    CHECK(eb == 0.0 && ea > 1e-6, "Rev Return 0: wet energy %g (want 0)", eb);
    CHECK(same, "Rev Return 0 changed the stored tail");
    /* 3. Low Cut filters the wet input: lows drop, highs pass */
    pair(); fxbus_set(&a, FX_RV_HPF, 0); fxbus_set(&b, FX_RV_HPF, 127);
    run(0, 60.0f, 0, &ea, &eb, &md, &same);
    CHECK(ea > 1e-6 && eb < 0.1 * ea, "Rev Low Cut max keeps %.1f%% of a 60 Hz wet (want <10%%)", ea > 0 ? 100 * eb / ea : 0);
    CHECK(!same, "Rev Low Cut did not change what enters the reverb");
    pair(); fxbus_set(&a, FX_RV_HPF, 0); fxbus_set(&b, FX_RV_HPF, 40);
    run(0, 6000.0f, 0, &ea, &eb, &md, &same);
    CHECK(ea > 1e-6 && eb > 0.5 * ea, "Rev Low Cut at 40 cuts a 6 kHz wet to %.1f%% (want >50%%)", ea > 0 ? 100 * eb / ea : 0);
    if (fails) { printf("reverb-bank-mapping: %d failure(s)\n", fails); return 1; }
    printf("reverb-bank-mapping: PASS\n");
    return 0;
}
