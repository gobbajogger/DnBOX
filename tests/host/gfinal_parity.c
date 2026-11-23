/* SPDX-License-Identifier: GPL-3.0-only */
/* Host harness for gfinal_parity_test.py: coverage-exact compare + static-glyph check. */
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "dsp/fastmath.h"
#include "ui_gfinal.h"
#define N (GF_W*GF_H)
static uint16_t mask[N];
static uint32_t hash_case(int id, int value, int st, const gf_live *L)
{
    uint32_t h = 2166136261u;
    for (int r = 0; r < 4; ++r) { gf_render(id, value, st, L, r, mask); for (int i = 0; i < N; ++i) h = (h ^ gf_m[i]) * 16777619u; }
    return h;
}
int main(int argc, char **argv)
{
    if (argc != 3) return 2;
    FILE *t = fopen(argv[1], "r"), *b = fopen(argv[2], "rb");
    if (!t || !b) return 2;
    char name[32], cs[32]; int id, value, st, act, fl; double v[8]; int ncase = 0, bad = 0;
    uint32_t first_v[GF_COUNT] = {0}, first_l[GF_COUNT] = {0}; uint8_t has_v[GF_COUNT] = {0}, has_l[GF_COUNT] = {0}, moves_v[GF_COUNT] = {0}, moves_l[GF_COUNT] = {0};
    unsigned char exp[4*N];
    while (fscanf(t, "%d %31s %31s %d %d %d %d %lf %lf %lf %lf %lf %lf %lf %lf", &id, name, cs, &value, &st, &act, &fl,
                  v, v+1, v+2, v+3, v+4, v+5, v+6, v+7) == 15) {
        if (id < 0 || id >= GF_COUNT || fread(exp, 1, sizeof exp, b) != sizeof exp) return 2;
        ++ncase;
        gf_live L = {0}; L.active = act; L.tape_active = fl & 1; L.playing = fl >> 1 & 1;
        L.amp = v[0]; L.env = v[1]; L.filter_env = v[2]; L.phase = v[3]; L.progress = v[4]; L.mod = v[5]; L.tape_phase = v[6]; L.clock_phase = v[7];
        int diff = 0;
        for (int r = 0; r < 4; ++r) { gf_render(id, value, st, &L, r, mask); for (int i = 0; i < N; ++i) diff += __builtin_popcount(gf_m[i]) != exp[r*N + i]; }
        if (diff) { ++bad; printf("PIXEL MISMATCH %s %s: %d px\n", name, cs, diff); }
        uint32_t h = hash_case(id, value, st, &L);
        if (!strncmp(cs, "live", 4)) { if (!has_l[id]) { has_l[id] = 1; first_l[id] = h; } else if (h != first_l[id]) moves_l[id] = 1; }
        else { if (!has_v[id]) { has_v[id] = 1; first_v[id] = h; } else if (h != first_v[id]) moves_v[id] = 1; }
    }
    int stat = 0;
    for (int i = 0; i < GF_COUNT; ++i) {
        if (!has_v[i] || !moves_v[i]) { ++stat; printf("STATIC GLYPH %s: value/state sweep does not change it\n", gf_names[i]); }
        if (has_l[i] && !moves_l[i]) { ++stat; printf("STATIC GLYPH %s: live frames do not move it\n", gf_names[i]); }
    }
    printf("G-FINAL parity: %d cases, %d pixel mismatches, %d static glyphs, %d glyph types\n", ncase, bad, stat, GF_COUNT);
    return bad || stat || !ncase;
}
