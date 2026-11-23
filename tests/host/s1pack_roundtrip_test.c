/* SPDX-License-Identifier: GPL-3.0-only */
/* Packed Synth One factory programs: every program's name and ordered physical
 * writes decode bit for bit from the pack, read from an arbitrary base. Input:
 * the pack and a flat expectation file written by s1pack_roundtrip_test.py. */
#include "../../firmware/src/dsp/s1pack/s1pack.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../firmware/src/crc32_nibble.h"

struct expect { unsigned n; unsigned ids[256]; uint32_t bits[256]; unsigned bad; };
static void emit(void *ctx, unsigned id, float v)
{
    struct expect *e = ctx;
    uint32_t b;
    memcpy(&b, &v, 4);
    if (e->n >= 256 || e->ids[e->n] != id || e->bits[e->n] != b) e->bad++;
    e->n++;
}
static unsigned char *slurp(const char *path, long *n)
{
    FILE *f = fopen(path, "rb");
    unsigned char *p;
    if (!f) { perror(path); exit(2); }
    fseek(f, 0, SEEK_END); *n = ftell(f); fseek(f, 0, SEEK_SET);
    p = malloc(*n + 64);
    if (fread(p + 4 * 7, 1, *n, f) != (size_t)*n) exit(2);   /* odd-but-aligned base */
    fclose(f);
    return p;
}
int main(int argc, char **argv)
{
    long n, en;
    unsigned char *raw, *pack;
    FILE *ef;
    unsigned programs, p, words = 0, nonfinite = 0, bad = 0, b, total_bank = 0, ncmd;
    setvbuf(stdout, NULL, _IONBF, 0);
    if (argc != 3) return 2;
    raw = slurp(argv[1], &n);
    pack = raw + 28;
    programs = s1pack_programs(pack, (uint32_t)n);
    if (programs != 694 || s1pack_total(pack) != (uint32_t)n ||
        x0x_crc32(pack + 32, (uint32_t)(n - 32)) != s1pack_crc(pack)) {
        printf("FAIL header programs=%u\n", programs);
        free(raw);
        return 1;
    }
    if (s1pack_programs(pack, (uint32_t)n - 1u)) { printf("FAIL truncated accepted\n"); free(raw); return 1; }
    for (b = 0;; b++) {
        const char *name; unsigned first, count;
        if (!s1pack_bank(pack, b, &name, &first, &count)) break;
        if (first != total_bank) { printf("FAIL bank %u\n", b); free(raw); return 1; }
        total_bank += count;
    }
    ef = fopen(argv[2], "rb");
    if (!ef) return 2;
    fseek(ef, 0, SEEK_END); en = ftell(ef); fseek(ef, 0, SEEK_SET); (void)en;
    for (p = 0; p < programs; p++) {
        struct expect e = {0};
        char want[64], got[S1PACK_NAME_MAX];
        unsigned char len;
        unsigned i;
        if (fread(&len, 1, 1, ef) != 1 || fread(want, 1, len, ef) != len) return 2;
        want[len] = 0;
        if (fread(&ncmd, 4, 1, ef) != 1 || ncmd > 256) return 2;
        for (i = 0; i < ncmd; i++)
            if (fread(&e.ids[i], 4, 1, ef) != 1 || fread(&e.bits[i], 4, 1, ef) != 1) return 2;
        if (!s1pack_name(pack, p, got) || strcmp(got, want)) { printf("FAIL name %u '%s' '%s'\n", p, got, want); bad++; }
        if (s1pack_load(pack, p, emit, &e) != (int)ncmd || e.n != ncmd || e.bad) { printf("FAIL program %u (%u bad)\n", p, e.bad); bad++; }
        for (i = 0; i < ncmd; i++) if ((e.bits[i] & 0x7F800000u) == 0x7F800000u) nonfinite++;
        words += e.n;
    }
    if (s1pack_load(pack, programs, emit, &(struct expect){0}) != -1) { printf("FAIL out of range\n"); bad++; }
    printf("s1pack: programs=%u banks=%u words=%u nonfinite=%u bytes=%ld mismatches=%u\n",
           programs, b, words, nonfinite, n, bad);
    printf("%s\n", bad || total_bank != programs || words != programs * 144u ? "FAIL" : "PASS");
    free(raw);
    fclose(ef);
    return bad || total_bank != programs ? 1 : 0;
}
