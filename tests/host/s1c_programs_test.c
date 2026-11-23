/* SPDX-License-Identifier: GPL-3.0-only */
/* Row 4 / #6: every Synth One factory program (X0X_S1C) loads through the runtime, plays
 * six voices with its own delay and reverb, and stays finite. Also checks the RAM rules:
 * the part does not grow the backend union and the effects fit the Chicago FX2 arena,
 * which is re-initialised when the part retires.
 * Usage: test [pack]. Without a pack (it is built by tools/pack_synthone_factory.py,
 * not committed) the program walk prints SKIP and only INIT is checked.
 * Deadline overruns: host wall time per 256-frame block against its 5.8 ms period; this is a
 * coarse host check, the device figure comes from fm1-live. */
#define _POSIX_C_SOURCE 199309L
#include "app/original_runtime.h"
#include "chicago_fx2_compact.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static original_runtime_part part;
static original_runtime_workspace workspace;
static original_chicago_fx2_compact fx;
static original_chicago_fx2_cold_owner owner;

static uint8_t *slurp(const char *path, uint32_t *n)
{
    FILE *f = path ? fopen(path, "rb") : NULL;
    if (!f) return NULL;
    fseek(f, 0, SEEK_END);
    long len = ftell(f);
    fseek(f, 0, SEEK_SET);
    uint8_t *b = malloc((size_t)len);
    assert(b && fread(b, 1, (size_t)len, f) == (size_t)len);
    fclose(f);
    *n = (uint32_t)len;
    return b;
}

static double now(void)
{
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    return t.tv_sec + t.tv_nsec * 1e-9;
}

static unsigned nonfinite, overruns, silent;
static double worst_ms;

static void play(unsigned factory)
{
    uint8_t cell[32];
    assert(original_sound_init(cell, ORIGINAL_SOUND_SYNTHONE, factory));
    assert(original_sound_valid(cell));
    assert(original_sound_s1_program(cell) == factory);
    assert(original_runtime_retire(&part) == ORIGINAL_RUNTIME_OK);
    assert(original_runtime_init(&part, cell, 120, NULL, &owner) == ORIGINAL_RUNTIME_OK);
    static const unsigned notes[6] = {48, 55, 60, 64, 67, 72};
    float l[256], r[256], peak = 0.0f;
    for (unsigned b = 0; b < 300; ++b) {          /* ~1.5 s chord (arp patterns may start on a rest), then release tail */
        if (b == 0) for (unsigned i = 0; i < 6; ++i)
            assert(original_runtime_key(&part, ORIGINAL_INPUT_KEYS, notes[i], 100) == ORIGINAL_RUNTIME_OK);
        if (b == 260) for (unsigned i = 0; i < 6; ++i)
            assert(original_runtime_key(&part, ORIGINAL_INPUT_KEYS, notes[i], 0) == ORIGINAL_RUNTIME_OK);
        double t0 = now();
        assert(original_runtime_render(&part, &workspace, l, r, 256) == ORIGINAL_RUNTIME_OK);
        double ms = (now() - t0) * 1e3;
        if (ms > worst_ms) worst_ms = ms;
        if (ms > 256.0 * 1e3 / 44100.0) overruns++;
        for (unsigned i = 0; i < 256; ++i) {
            if (!isfinite(l[i]) || !isfinite(r[i])) { nonfinite++; break; }
            if (fabsf(l[i]) > peak) peak = fabsf(l[i]);
        }
    }
    if (peak == 0.0f) { silent++; if (getenv("S1_LIST")) printf("silent %u\n", factory); }
}

int main(int argc, char **argv)
{
    uint32_t n = 0;
    uint8_t *pack = slurp(argc > 1 ? argv[1] : getenv("S1_PACK"), &n);
    _Static_assert(sizeof(s1_dry_part) <= sizeof(original_miami_backend), "union");
    _Static_assert(sizeof(original_chicago_fx2_compact) >= S1C_ARENA_BYTES, "arena");
    printf("s1c part %u B (union %u B), arena use %u of %u B (reverb %u, delay pool %u)\n",
           (unsigned)sizeof(s1_dry_part), (unsigned)sizeof(original_runtime_backend),
           (unsigned)S1C_ARENA_BYTES, (unsigned)sizeof(original_chicago_fx2_compact),
           (unsigned)S1C_RVB_BYTES, (unsigned)(S1C_ARENA_BYTES - S1C_RVB_BYTES));
    assert(original_chicago_fx2_compact_init(&fx));
    owner = original_chicago_fx2_compact_owner(&fx);
    s1_part_pack = pack; s1_part_pack_bytes = n;
    play(0);
    unsigned programs = s1_part_programs();
    if (!pack || !programs) {
        printf("SKIP s1c programs: no pack (set S1_PACK); INIT nonfinite=%u\n", nonfinite);
        return nonfinite != 0;
    }
    assert(programs == ORIGINAL_SYNTHONE_PROGRAMS);
    for (unsigned k = 1; k <= programs; ++k) play(k);
    /* Retire hands the arena back to Chicago in its initial state. */
    assert(original_runtime_retire(&part) == ORIGINAL_RUNTIME_OK);
    original_chicago_fx2_compact clean;
    assert(original_chicago_fx2_compact_init(&clean));
    assert(fx.ready == clean.ready && fx.selected == clean.selected);
    printf("programs=%u nonfinite=%u silent=%u host_overruns=%u worst_block_ms=%.3f \n",
           programs, nonfinite, silent, overruns, worst_ms);
    return nonfinite != 0 || silent != 0;
}
