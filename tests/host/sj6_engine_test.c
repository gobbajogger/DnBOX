/* SPDX-License-Identifier: GPL-3.0-only */
/* Host check of the live SuperJ6 voice (sj6_engine.c): R008 defaults load,
 * polyphony and voice stealing, finite output, release to silence, sizes,
 * and host time per sample at 1, 4 and SJ6_POLY voices. No private data. */
#define _POSIX_C_SOURCE 199309L
#include "sj6_engine.h"
#include <stdio.h>
#include <string.h>
#include <time.h>

static double now(void)
{
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    return t.tv_sec + t.tv_nsec * 1e-9;
}

static Sj6Engine e;
static float L[256], R[256];

static int finite_block(void)
{
    unsigned i;
    for (i = 0; i < 256; i++) {
        uint32_t u;
        memcpy(&u, &L[i], 4);
        if (((u >> 23) & 255u) == 255u) return 0;
    }
    return 1;
}

int main(void)
{
    unsigned pv, pool, poly, fails = 0, checks = 0, i, k;
    float peak = 0.f;
    sj6_engine_bytes(&pv, &pool, &poly);
    printf("sizes: per-voice %u B, engine (pool) %u B, poly %u, resynth state %u B\n",
           pv, pool, poly, (unsigned)sizeof(Sj6State));
    sj6_engine_init(&e);
    sj6_engine_select(&e, 1);
    {   /* out-of-range ids are counted and skipped */
        uint16_t ids[2] = {0, 999};
        float v[2] = {1.f, 1.f};
        checks++;
        if (sj6_engine_load(&e, ids, v, 2) != 1) { fails++; printf("FAIL load bad-id count\n"); }
    }
    for (k = 0; k < poly + 2; k++) sj6_engine_note_on(&e, (uint8_t)(48 + k), 100);
    checks++;
    if (sj6_engine_sounding(&e) != poly) { fails++; printf("FAIL poly %u\n", sj6_engine_sounding(&e)); }
    for (i = 0; i < 172; i++) {           /* about 1 s */
        memset(L, 0, sizeof L); memset(R, 0, sizeof R);
        sj6_engine_render(&e, L, R, 256, 120.f);
        if (!finite_block()) { fails++; printf("FAIL non-finite\n"); break; }
        for (k = 0; k < 256; k++) if (L[k] > peak) peak = L[k]; else if (-L[k] > peak) peak = -L[k];
    }
    checks += 2;
    if (!(peak > 1e-4f)) { fails++; printf("FAIL silent (peak %g)\n", peak); }
    printf("peak over 1 s with %u voices: %.4f\n", poly, peak);
    for (k = 0; k < poly + 2; k++) sj6_engine_note_off(&e, (uint8_t)(48 + k));
    for (i = 0; i < 2000 && sj6_engine_sounding(&e); i++) {
        memset(L, 0, sizeof L); memset(R, 0, sizeof R);
        sj6_engine_render(&e, L, R, 256, 120.f);
    }
    checks++;
    if (sj6_engine_sounding(&e)) { fails++; printf("FAIL release: %u still sounding\n", sj6_engine_sounding(&e)); }
    {
        unsigned counts[3] = {1, 4, SJ6_POLY}, c;
        for (c = 0; c < 3; c++) {
            double t0, t1;
            sj6_engine_init(&e);
            sj6_engine_select(&e, 1);
            for (k = 0; k < counts[c]; k++) sj6_engine_note_on(&e, (uint8_t)(40 + 5 * k), 100);
            t0 = now();
            for (i = 0; i < 1723; i++) {   /* 10 s of audio */
                sj6_engine_render(&e, L, R, 256, 120.f);
            }
            t1 = now();
            printf("host: %u voices %.1f ns/sample (%.2f%% of one host core at 44.1 kHz); source FPU-result count %u/sample\n",
                   counts[c], (t1 - t0) * 1e9 / (1723.0 * 256.0), (t1 - t0) / 10.0 * 100.0, sj6_fpu_estimate(counts[c]));
        }
    }
    printf("%s: %u checks, %u failed\n", fails ? "FAIL" : "PASS", checks, fails);
    return fails != 0;
}
