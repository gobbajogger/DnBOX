/* SPDX-License-Identifier: GPL-3.0-only */
/* SuperJ6 row 5: every control on the shipping SuperJ6 editor pages
 * (ui_original_juno_rows: 4 pages x 4 = 16 controls) changes the part output
 * through the production runtime path (UI q127 byte -> original_sound_control_value
 * -> original_runtime_param -> original_runtime_render), each FX changes the output,
 * and FX bypass (chorus mix 0) is bit-exact dry. Prints controls=16/16 fx=1/1 bypass=bit-exact. */
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include "../../firmware/src/app/original_runtime.h"
#include "../../firmware/src/app/ui_original_controls.h"

#define BLOCK 256u
#define BLOCKS 160u                      /* 0.93 s at 44.1 kHz */
static original_runtime_part p;
static original_runtime_workspace w;
static float out_a[BLOCKS * BLOCK * 2], out_b[BLOCKS * BLOCK * 2];

typedef struct { unsigned id; uint8_t byte; } setting;
static void set_byte(unsigned id, uint8_t byte)
{
    float x;
    assert(original_sound_control_value(ORIGINAL_SOUND_JUNO, id, byte, &x));
    assert(original_runtime_param(&p, id, x) == ORIGINAL_RUNTIME_OK);
}
/* Two-note phrase: held 0.46 s, then released, so envelopes, filter and tails all count. */
static void render(const setting *ctx, unsigned nctx, unsigned id, uint8_t byte, float *out)
{
    uint8_t cell[32];
    float l[BLOCK], r[BLOCK];
    assert(original_sound_init(cell, ORIGINAL_SOUND_JUNO, 0));
    assert(original_runtime_init(&p, cell, 120, 0, 0) == ORIGINAL_RUNTIME_OK);
    for (unsigned i = 0; i < nctx; i++) set_byte(ctx[i].id, ctx[i].byte);
    set_byte(id, byte);
    assert(original_runtime_key(&p, ORIGINAL_INPUT_KEYS, 48, 100) == ORIGINAL_RUNTIME_OK);
    assert(original_runtime_key(&p, ORIGINAL_INPUT_KEYS, 55, 100) == ORIGINAL_RUNTIME_OK);
    for (unsigned b = 0; b < BLOCKS; b++) {
        if (b == BLOCKS / 2) {
            assert(original_runtime_key(&p, ORIGINAL_INPUT_KEYS, 48, 0) == ORIGINAL_RUNTIME_OK);
            assert(original_runtime_key(&p, ORIGINAL_INPUT_KEYS, 55, 0) == ORIGINAL_RUNTIME_OK);
        }
        assert(original_runtime_render(&p, &w, l, r, BLOCK) == ORIGINAL_RUNTIME_OK);
        for (unsigned i = 0; i < BLOCK; i++) {
            assert(isfinite(l[i]) && isfinite(r[i]));
            out[(b * BLOCK + i) * 2] = l[i]; out[(b * BLOCK + i) * 2 + 1] = r[i];
        }
    }
    assert(original_runtime_retire(&p) == ORIGINAL_RUNTIME_OK);
}
/* Difference energy relative to the louder render, in dB. */
static double diff_db(void)
{
    double d = 0, ea = 0, eb = 0;
    for (unsigned i = 0; i < BLOCKS * BLOCK * 2; i++) {
        double x = out_a[i], y = out_b[i];
        d += (x - y) * (x - y); ea += x * x; eb += y * y;
    }
    double e = ea > eb ? ea : eb;
    return e > 0 ? 10 * log10(d / e + 1e-30) : -300;
}

int main(void)
{
    /* Context so each control is reachable: pulse wave with PWM for the LFO/width
     * controls, chorus wet for the chorus rate, a mid cutoff for resonance/env depth,
     * low sustain for decay. Every page control is listed explicitly. */
    static const setting pulse[] = {{0, 127}, {11, 100}}, chorus[] = {{14, 100}};
    static const setting mid[] = {{2, 50}}, lowsus[] = {{7, 10}}, hp[] = {{2, 127}};
    static const struct { unsigned id; uint8_t lo, hi; const setting *ctx; unsigned n; } c[16] = {
        {0, 0, 127, 0, 0}, {1, 0, 127, 0, 0}, {2, 20, 110, 0, 0}, {3, 0, 120, mid, 1},
        {4, 0, 127, mid, 1}, {5, 0, 110, 0, 0}, {6, 5, 120, lowsus, 1}, {7, 0, 127, 0, 0},
        {8, 0, 127, 0, 0}, {9, 0, 110, hp, 1}, {10, 5, 127, pulse, 2}, {11, 0, 127, pulse + 0, 1},
        {12, 10, 120, pulse + 0, 1}, {13, 0, 127, chorus, 1}, {14, 0, 127, 0, 0}, {15, 20, 127, 0, 0},
    };
    /* The page map must be exactly the list above. */
    unsigned seen = 0, pages = 0;
    for (unsigned pg = 0; pg < UI_ORIGINAL_JUNO_PAGES; pg++)
        for (unsigned k = 0; k < 4; k++) {
            int id = ui_original_juno_rows[pg][k];
            if (id < 0) continue;
            assert(id < 16 && !(seen & (1u << id)) && *ui_original_name(ORIGINAL_SOUND_JUNO, (unsigned)id));
            seen |= 1u << id; pages++;
        }
    assert(pages == 16 && seen == 0xFFFFu);
    unsigned pass = 0;
    for (unsigned i = 0; i < 16; i++) {
        render(c[i].ctx, c[i].n, c[i].id, c[i].lo, out_a);
        render(c[i].ctx, c[i].n, c[i].id, c[i].hi, out_b);
        double db = diff_db();
        printf("  %-12s id %2u  diff %7.1f dB\n", ui_original_name(ORIGINAL_SOUND_JUNO, c[i].id), c[i].id, db);
        pass += db > -40.0;
    }
    /* FX: the instrument chorus. Wet vs bypass must differ; bypass must equal dry bit for bit. */
    render(0, 0, 14, 0, out_a);
    render(0, 0, 14, 127, out_b);
    unsigned fx = diff_db() > -40.0;
    {   /* bypass: runtime output with chorus mix 0 vs the synth rendered without the chorus stage */
        uint8_t cell[32]; float l[BLOCK], r[BLOCK];
        static juno6_t dry; unsigned exact = 1;
        assert(original_sound_init(cell, ORIGINAL_SOUND_JUNO, 0));
        assert(original_runtime_init(&p, cell, 120, 0, 0) == ORIGINAL_RUNTIME_OK);
        set_byte(14, 0);
        assert(original_runtime_key(&p, ORIGINAL_INPUT_KEYS, 48, 100) == ORIGINAL_RUNTIME_OK);
        dry = p.dsp.juno.synth;
        for (unsigned b = 0; b < 40; b++) {
            float dl[BLOCK], dr[BLOCK];
            assert(original_runtime_render(&p, &w, l, r, BLOCK) == ORIGINAL_RUNTIME_OK);
#if X0X_SUPERJ6_FACTORY
            assert(juno6_render_stereo(&dry, dl, dr, BLOCK));
            if (dry.master_gain != 1.f) for (unsigned i = 0; i < BLOCK; i++) { dl[i] *= dry.master_gain; dr[i] *= dry.master_gain; }
#else
            juno6_render(&dry, dl, BLOCK); memcpy(dr, dl, sizeof dr);
#endif
            exact &= !memcmp(l, dl, sizeof l) && !memcmp(r, dr, sizeof r);
        }
        assert(original_runtime_retire(&p) == ORIGINAL_RUNTIME_OK);
        printf("superj6 controls=%u/16 fx=%u/1 bypass=%s\n", pass, fx, exact ? "bit-exact" : "DIFFERS");
        return pass == 16 && fx && exact ? 0 : 1;
    }
}
