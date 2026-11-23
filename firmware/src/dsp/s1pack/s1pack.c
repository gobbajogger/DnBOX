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
uint32_t s1pack_total(const uint8_t *pack) { return s1p_le(pack + 12, 4); }
uint32_t s1pack_crc(const uint8_t *pack) { return s1p_le(pack + 16, 4); }

unsigned s1pack_programs(const uint8_t *pack, uint32_t available)
{
    uint32_t total;
    if (!pack || available < S1PACK_HEADER || s1p_le(pack, 4) != 0x4B503153u || H16(4) != 1u) return 0;
    total = s1pack_total(pack);
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

/* Walk to program p from its checkpoint. Column codes are re-read per skipped
 * program from the column section; every value is skipped, names kept in out. */
static int s1p_seek(const uint8_t *pack, unsigned program, s1p_bits *b, char *out)
{
    s1p_names n;
    unsigned pstep = H16(30), ncol = H16(8), gbits = pack[11], skip, c;
    uint16_t cnt[15];
    if (program >= H16(6)) return 0;
    s1p_name_tables(pack, &n);
    b->p = pack + H16(24);
    b->at = s1p_table(pack, 3u + (H16(26) + H16(28) - 1u) / H16(28) + program / pstep);
    for (skip = program % pstep;; skip--) {
        if (!s1p_read_name(b, &n, out)) return 0;
        if (!skip) return 1;
        s1p_bits col = {pack + H16(24), s1p_table(pack, 0)};
        for (c = 0; c < ncol; c++) {
            uint32_t esc, s;
            unsigned L;
            s1p_get(&col, 8);
            esc = s1p_eg(&col, 0);
            L = s1p_counts(&col, cnt);
            s = s1p_sym(b, cnt, L);
            if (s == 0xFFFFu) return 0;
            if (esc && s == esc - 1u) s1p_get(b, gbits);
        }
    }
}

int s1pack_name(const uint8_t *pack, unsigned program, char *out)
{
    s1p_bits b;
    return s1p_seek(pack, program, &b, out);
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

int s1pack_load(const uint8_t *pack, unsigned program, s1pack_emit_fn emit, void *context)
{
    char name[S1PACK_NAME_MAX];
    s1p_bits b, col, dict;
    unsigned ncol = H16(8), gbits = pack[11], c;
    uint16_t cnt[15];
    if (!s1p_seek(pack, program, &b, name)) return -1;
    col.p = dict.p = pack + H16(24);
    col.at = s1p_table(pack, 0);
    dict.at = s1p_table(pack, 1);
    for (c = 0; c < ncol; c++) {
        unsigned id = s1p_get(&col, 8), L, k, i, entries = 0;
        uint32_t esc = s1p_eg(&col, 0), s, g = 0, at, key;
        L = s1p_counts(&col, cnt);
        for (i = 0; i < L; i++) entries += cnt[i];
        if (esc) entries--;
        s = s1p_sym(&b, cnt, L);
        if (s == 0xFFFFu) return -1;
        k = s1p_get(&dict, 4);
        if (esc && s == esc - 1u) {
            g = s1p_get(&b, gbits);
            s = entries;                 /* walk past the whole dictionary */
        } else if (esc && s > esc - 1u) {
            s--;
        }
        for (i = 0, at = 0; i < entries; i++) {
            uint32_t z = s1p_eg(&dict, k);
            at += (z & 1u) ? ~(z >> 1) : z >> 1;
            if (i == s) g = at;
        }
        if (g >= H16(26)) return -1;
        key = s1p_global(pack, g);
        {
            union { uint32_t u; float f; } bits = {(key & 0x80000000u) ? key & 0x7FFFFFFFu : ~key};
            emit(context, id, bits.f);
        }
    }
    return (int)ncol;
}
