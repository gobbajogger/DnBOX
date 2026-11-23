/* SPDX-License-Identifier: GPL-3.0-only */
/* Synth One compact delay/reverb level regression (X0X_S1C). Irresistible Forces Pad (062), JEC Atmostrings 2 and Red Minor (435)
 * play the factory stimulus (six notes, 0.75 s each, rising velocity, then 1.5 s release)
 * through the device part. The body RMS (1.0-4.5 s) and the last-second tail RMS must stay
 * within a few dB of the original Synth One (AudioKit, 48 kHz render of the same stimulus;
 * reference levels below, no audio in Git). 062's long high-feedback delay tail is the
 * compact line's known gap (decimated storage), so it has a wider tail bound.
 * Usage: test [pack]; SKIP without the factory pack (S1_PACK). */
#include "app/s1c_part.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static s1_dry_part part;
static _Alignas(8) uint8_t arena[S1C_ARENA_BYTES];

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

/* program = pack index; the ear-preview files number by the reference dump (Atmostrings 2 is 289 there). */
static const struct { unsigned program; const char *name; float body_db, tail_db, body_tol, tail_tol; } ref[] = {
    { 62, "Irresistible Forces Pad", -14.51f,  -6.49f, 3.0f, 8.0f },
    {278, "JEC Atmostrings 2",       -24.62f, -27.65f, 3.0f, 4.0f },
    {435, "Red Minor",               -12.00f, -24.08f, 3.0f, 4.0f },
};

int main(int argc, char **argv)
{
    uint32_t n = 0;
    uint8_t *pack = slurp(argc > 1 ? argv[1] : getenv("S1_PACK"), &n);
    if (!pack) { printf("SKIP s1c fx level: no pack (set S1_PACK)\n"); return 0; }
    s1_part_pack = pack; s1_part_pack_bytes = n;
    const unsigned sr = 44100, gate = sr * 3 / 4, play = 6 * gate, total = play + sr * 3 / 2;
    static const unsigned note[6] = {48, 55, 60, 64, 67, 72};
    int fail = 0;
    for (unsigned i = 0; i < sizeof ref / sizeof ref[0]; ++i) {
        assert(strcmp(s1_part_program_name(ref[i].program), ref[i].name) == 0);
        s1_part_init(&part);
        s1_part_bind(&part, arena);
        assert(s1_part_program(&part, ref[i].program));
        double body = 0, tail = 0; unsigned nb = 0, nt = 0, cur = 0;
        float l[128], r[128];
        for (unsigned at = 0; at < total;) {
            if (at % gate == 0 && at <= play) {
                if (at) s1_part_note(&part, note[cur], 0);
                if (at < play) { cur = (at / gate) % 6; s1_part_note(&part, note[cur], 64 + at / gate * 11); }
            }
            unsigned k = 128, left = gate - at % gate;
            if (at < play + gate && k > left) k = left;
            if (k > total - at) k = total - at;
            s1_part_render(&part, l, r, k);
            for (unsigned j = 0; j < k; ++j, ++at) {
                double e = 0.5 * ((double)l[j] * l[j] + (double)r[j] * r[j]);
                if (at >= sr && at < sr * 9 / 2) { body += e; nb++; }
                if (at >= total - sr) { tail += e; nt++; }
            }
        }
        float bd = 10.0f * log10f((float)(body / nb) + 1e-20f), td = 10.0f * log10f((float)(tail / nt) + 1e-20f);
        int ok = fabsf(bd - ref[i].body_db) <= ref[i].body_tol && fabsf(td - ref[i].tail_db) <= ref[i].tail_tol;
        printf("s1c fx level %-24s: body %.1f dB (orig %.1f) tail %.1f dB (orig %.1f) %s\n", ref[i].name,
               bd, ref[i].body_db, td, ref[i].tail_db, ok ? "ok" : "FAIL");
        fail |= !ok;
    }
    return fail;
}
