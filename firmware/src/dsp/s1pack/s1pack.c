/* SPDX-License-Identifier: GPL-3.0-only */
/* Reader for the packed Synth One factory programs; format in
 * tools/pack_synthone_factory.py. Bit fields are MSB first. */
#include "s1pack.h"

typedef struct { const uint8_t *p; uint32_t at; } s1p_bits;

static uint32_t s1p_le(const uint8_t *p, unsigned n)
{
    uint32_t v = 0;
    while (n--) v = v << 8 | p[n];
    return v;
}
static uint32_t s1p_get(s1p_bits *b, unsigned n)
{
    uint32_t v = 0;
    while (n--) {
        v = v << 1 | (uint32_t)(b->p[b->at >> 3] >> (7u - (b->at & 7u)) & 1u);
        b->at++;
    }
    return v;
}
static uint32_t s1p_eg(s1p_bits *b, unsigned k)
{
    unsigned z = 0;
    while (!s1p_get(b, 1)) if (++z > 31u) return 0;
    z += k;
    return ((z < 32u ? 1u << z : 0u) | s1p_get(b, z)) - (1u << k);
}
/* Canonical Huffman table: L(4) then counts. Returns L. */
static unsigned s1p_counts(s1p_bits *b, uint16_t *cnt)
{
    unsigned i, L = s1p_get(b, 4);
    for (i = 0; i < L; i++) cnt[i] = (uint16_t)s1p_eg(b, 0);
    return L;
}
static uint32_t s1p_sym(s1p_bits *b, const uint16_t *cnt, unsigned L)
{
    uint32_t code = 0, first = 0, index = 0;
    unsigned i;
    for (i = 0; i < L; i++) {
        code |= s1p_get(b, 1);
        if (code - first < cnt[i]) return index + code - first;
        index += cnt[i];
        first = (first + cnt[i]) << 1;
        code <<= 1;
    }
    return 0xFFFFu;
}

#define H16(o) s1p_le(pack + (o), 2)

unsigned s1pack_programs(const uint8_t *pack, uint32_t available)
{
    uint32_t total;
    if (!pack || available < S1PACK_HEADER || s1p_le(pack, 4) != 0x4B503153u || H16(4) != 1u) return 0;
    total = s1p_le(pack + 12, 4);
    if (total > available || H16(20) != S1PACK_HEADER || H16(22) >= total || H16(24) >= total ||
        !H16(30) || !H16(28) || !H16(6))
        return 0;
    return H16(6);
}

int s1pack_bank(const uint8_t *pack, unsigned bank, const char **name, unsigned *first, unsigned *count)
{
    const uint8_t *p = pack + H16(20);
    if (bank >= pack[10]) return 0;
    while (bank--) {
        p += 5;
        while (*p++) {}
    }
    *first = s1p_le(p, 2);
    *count = s1p_le(p + 2, 2);
    *name = (const char *)p + 5;
    return 1;
}

/* Stream layout: the stream area starts at H16(24). The three section
 * starts, then the global and program checkpoints, are u24 at H16(22). */
static uint32_t s1p_table(const uint8_t *pack, unsigned i) { return s1p_le(pack + H16(22) + 3u * i, 3); }

typedef struct {
    s1p_bits prefix_syms, char_syms;    /* symbol lists of the two name codes */
    uint16_t pcnt[15], ccnt[15];
    unsigned pL, cL;
} s1p_names;

static void s1p_name_tables(const uint8_t *pack, s1p_names *n)
{
    s1p_bits b = {pack + H16(24), 0};
    n->pL = s1p_counts(&b, n->pcnt);
    n->prefix_syms = b;
    for (unsigned i = 0; i < n->pL; i++) b.at += 5u * n->pcnt[i];
    n->cL = s1p_counts(&b, n->ccnt);
    n->char_syms = b;
}
static uint32_t s1p_nth(s1p_bits list, uint32_t i, unsigned width)
{
    list.at += i * width;
    return s1p_get(&list, width);
}
/* Reads one program's name into o, keeping the front-coded prefix already there. */
static int s1p_read_name(s1p_bits *b, const s1p_names *n, char *o)
{
    unsigned i = s1p_nth(n->prefix_syms, s1p_sym(b, n->pcnt, n->pL), 5);
    for (;; i++) {
        uint32_t s = s1p_sym(b, n->ccnt, n->cL);
        if (s == 0xFFFFu || i >= S1PACK_NAME_MAX) return 0;
        o[i] = (char)s1p_nth(n->char_syms, s, 8);
        if (!o[i]) return 1;
    }
}

static uint32_t s1p_global(const uint8_t *pack, uint32_t index)
{
    unsigned gstep = H16(28);
    s1p_bits g = {pack + H16(24), s1p_table(pack, 2)};
    unsigned k = s1p_get(&g, 5);
    uint32_t v;
    g.at = s1p_table(pack, 3u + index / gstep);
    v = s1p_get(&g, 32);
    for (index %= gstep; index; index--) v += s1p_eg(&g, k);
    return v;
}

/* Walk from program p's checkpoint to p, reading each name into out (front
 * coding keeps the shared prefix) and each column symbol. At p, emit is NULL
 * for a name lookup; otherwise each column resolves its dictionary entry and
 * global value. Returns the column count, or -1 on a malformed pack. */
int s1pack_read(const uint8_t *pack, unsigned program, char *out, s1pack_emit_fn emit, void *context)
{
    s1p_names n;
    s1p_bits b, col, dict;
    unsigned pstep = H16(30), ncol = H16(8), gbits = pack[11], skip, c;
    uint16_t cnt[15];
    if (program >= H16(6)) return -1;
    s1p_name_tables(pack, &n);
    b.p = col.p = dict.p = pack + H16(24);
    b.at = s1p_table(pack, 3u + (H16(26) + H16(28) - 1u) / H16(28) + program / pstep);
    for (skip = program % pstep;; skip--) {
        if (!s1p_read_name(&b, &n, out)) return -1;
        if (!skip && !emit) return 0;
        col.at = s1p_table(pack, 0);
        dict.at = s1p_table(pack, 1);
        for (c = 0; c < ncol; c++) {
            unsigned id = s1p_get(&col, 8), L, k, i, entries = 0;
            uint32_t esc = s1p_eg(&col, 0), s, g = 0, at;
            L = s1p_counts(&col, cnt);
            s = s1p_sym(&b, cnt, L);
            if (s == 0xFFFFu) return -1;
            if (esc && s == esc - 1u) g = s1p_get(&b, gbits);
            if (skip) continue;
            for (i = 0; i < L; i++) entries += cnt[i];
            k = s1p_get(&dict, 4);
            if (esc) {
                entries--;
                if (s == esc - 1u) s = entries;      /* walk past the whole dictionary */
                else if (s > esc - 1u) s--;
            }
            for (i = 0, at = 0; i < entries; i++) {
                uint32_t z = s1p_eg(&dict, k);
                at += (z & 1u) ? ~(z >> 1) : z >> 1;
                if (i == s) g = at;
            }
            if (g >= H16(26)) return -1;
            {
                uint32_t key = s1p_global(pack, g);
                union { uint32_t u; float f; } bits = {(key & 0x80000000u) ? key & 0x7FFFFFFFu : ~key};
                emit(context, id, bits.f);
            }
        }
        if (!skip) return (int)ncol;
    }
}

