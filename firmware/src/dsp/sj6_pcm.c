/* SPDX-License-Identifier: GPL-3.0-only */
#include "sj6_pcm.h"
#include <stdlib.h>
#include <string.h>

enum { SJ6_SHIFT = 14, SJ6_BLOCK = 512, SJ6_QESC = 16 };

static uint32_t zig(int32_t r)
{
    return ((uint32_t)r << 1) ^ (uint32_t)(r >> 31);
}

static int32_t unzig(uint32_t u)
{
    return (int32_t)((u >> 1) ^ (0u - (u & 1u)));
}

typedef struct BitW {
    uint8_t *d;
    size_t n, cap;
    uint64_t acc;
    int bits;
    int overflow;
} BitW;

static void bw_put(BitW *b, uint32_t v, int k)
{
    if (k <= 0 || b->overflow) return;
    b->acc |= (uint64_t)v << b->bits;
    b->bits += k;
    while (b->bits >= 8) {
        if (b->n >= b->cap) { b->overflow = 1; return; }
        b->d[b->n++] = (uint8_t)b->acc;
        b->acc >>= 8;
        b->bits -= 8;
    }
}

static void bw_unary(BitW *b, uint32_t q)
{
    while (q >= 24) { bw_put(b, 0, 24); q -= 24; }
    bw_put(b, 1u << q, (int)q + 1);
}

static void bw_flush(BitW *b)
{
    if (b->bits && !b->overflow) {
        if (b->n >= b->cap) { b->overflow = 1; return; }
        b->d[b->n++] = (uint8_t)b->acc;
        b->acc = 0;
        b->bits = 0;
    }
}

typedef struct BitR {
    const uint8_t *p, *end;
    uint64_t acc;
    int bits;
    int bad;
    Sj6PcmStats *st;
} BitR;

static uint32_t br_get(BitR *b, int k)
{
    uint32_t mask, v;
    if (k <= 0 || b->bad) return 0;
    if (b->st) b->st->bit_reads += (uint64_t)k;
    while (b->bits < k) {
        if (b->p >= b->end) { b->bad = 1; return 0; }
        b->acc |= (uint64_t)(*b->p++) << b->bits;
        b->bits += 8;
        if (b->st) b->st->adds++;
    }
    mask = k == 32 ? 0xffffffffu : ((1u << k) - 1u);
    v = (uint32_t)b->acc & mask;
    b->acc >>= k;
    b->bits -= k;
    if (b->st) b->st->shifts += 2;
    return v;
}

static uint32_t br_unary(BitR *b)
{
    uint32_t q = 0;
    for (;;) {
        if (b->st) b->st->branches++;
        if (br_get(b, 1)) return q;
        if (b->bad || q == 0xffffffffu) { b->bad = 1; return 0; }
        q++;
        if (b->st) b->st->adds++;
    }
}

static int32_t predict(const int32_t *hist, const int16_t *c, int order, Sj6PcmStats *st)
{
    int64_t acc = (int64_t)1 << (SJ6_SHIFT - 1);
    int i;
    for (i = 0; i < order; i++) {
        acc += (int64_t)c[i] * (int64_t)hist[i];
        if (st) { st->muls++; st->adds++; }
    }
    acc >>= SJ6_SHIFT;
    if (st) st->shifts++;
    return (int32_t)acc;
}

static void push_hist(int32_t *hist, int order, int32_t x)
{
    int i;
    for (i = order - 1; i > 0; i--) hist[i] = hist[i - 1];
    if (order) hist[0] = x;
}

static void autocorr(const int32_t *x, int n, int order, double *r)
{
    int lag, i;
    for (lag = 0; lag <= order; lag++) {
        double s = 0;
        for (i = lag; i < n; i++) s += (double)x[i] * (double)x[i - lag];
        r[lag] = s;
    }
}

static int levinson(const double *r, int order, double *a)
{
    double aa[SJ6_PCM_ORDER_MAX + 1][SJ6_PCM_ORDER_MAX + 1];
    double e = r[0];
    int i, j;
    if (!(e > 0)) return -1;
    memset(aa, 0, sizeof aa);
    for (i = 1; i <= order; i++) {
        double acc = r[i];
        double k;
        for (j = 1; j < i; j++) acc += aa[i - 1][j] * r[i - j];
        k = -acc / e;
        if (k > 0.999) k = 0.999;
        if (k < -0.999) k = -0.999;
        aa[i][i] = k;
        for (j = 1; j < i; j++) aa[i][j] = aa[i - 1][j] + k * aa[i - 1][i - j];
        e *= 1.0 - k * k;
        if (!(e > 1e-18)) return -1;
    }
    for (j = 1; j <= order; j++) a[j] = aa[order][j];
    return 0;
}

static void quantize(const double *a, int order, int16_t *c)
{
    int j;
    for (j = 0; j < order; j++) {
        double q = a[j + 1] * (double)(1 << SJ6_SHIFT);
        if (q > 32767.0) q = 32767.0;
        if (q < -32768.0) q = -32768.0;
        c[j] = (int16_t)(q < 0 ? q - 0.5 : q + 0.5);
    }
}

static uint32_t rice_u(int32_t residual, int k, int *escaped)
{
    uint32_t u = zig(residual);
    uint32_t q = u >> k;
    *escaped = q >= SJ6_QESC;
    return u;
}

static size_t rice_bits(int32_t residual, int k)
{
    int escaped = 0;
    uint32_t u = rice_u(residual, k, &escaped);
    if (escaped) return (size_t)SJ6_QESC + 1 + 32;
    return (size_t)(u >> k) + 1 + (size_t)k;
}

static void write_u16(uint8_t *p, unsigned v) { p[0] = (uint8_t)v; p[1] = (uint8_t)(v >> 8); }
static void write_u32(uint8_t *p, uint32_t v)
{
    p[0] = (uint8_t)v; p[1] = (uint8_t)(v >> 8); p[2] = (uint8_t)(v >> 16); p[3] = (uint8_t)(v >> 24);
}
static unsigned read_u16(const uint8_t *p) { return (unsigned)p[0] | ((unsigned)p[1] << 8); }
static uint32_t read_u32(const uint8_t *p)
{
    return (uint32_t)p[0] | ((uint32_t)p[1] << 8) | ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24);
}

static size_t encode_order(const int32_t *x, int n, int bits, int rate, int order,
                           const int16_t *c, uint8_t *out, size_t cap)
{
    BitW bw;
    int32_t hist[SJ6_PCM_ORDER_MAX];
    size_t head;
    int i, pos;
    (void)bits;
    if (order < 0 || order > SJ6_PCM_ORDER_MAX) return 0;
    head = 4 + 1 + 1 + 4 + 4 + 1 + 1 + (size_t)order * 2 + 2;
    if (cap < head) return 0;
    memcpy(out, "J6R1", 4);
    out[4] = 1;
    out[5] = (uint8_t)bits;
    write_u32(out + 6, (uint32_t)rate);
    write_u32(out + 10, (uint32_t)n);
    out[14] = (uint8_t)order;
    out[15] = SJ6_SHIFT;
    for (i = 0; i < order; i++) write_u16(out + 16 + i * 2, (unsigned)(uint16_t)c[i]);
    write_u16(out + 16 + order * 2, SJ6_BLOCK);
    memset(hist, 0, sizeof hist);
    memset(&bw, 0, sizeof bw);
    bw.d = out + head;
    bw.cap = cap - head;
    for (pos = 0; pos < n; ) {
        int len = n - pos;
        int k, bestk = 0;
        size_t best = (size_t)-1;
        int j;
        if (len > SJ6_BLOCK) len = SJ6_BLOCK;
        for (k = 0; k <= 12; k++) {
            size_t bitsum = 8;
            int32_t h[SJ6_PCM_ORDER_MAX];
            memcpy(h, hist, sizeof h);
            for (j = 0; j < len; j++) {
                int32_t pred = order ? predict(h, c, order, 0) : 0;
                bitsum += rice_bits(x[pos + j] - pred, k);
                push_hist(h, order, x[pos + j]);
            }
            if (bitsum < best) { best = bitsum; bestk = k; }
        }
        bw_put(&bw, (uint32_t)bestk, 4);
        {
            int32_t h[SJ6_PCM_ORDER_MAX];
            memcpy(h, hist, sizeof h);
            for (j = 0; j < len; j++) {
                int escaped = 0;
                int32_t pred = order ? predict(h, c, order, 0) : 0;
                uint32_t u = rice_u(x[pos + j] - pred, bestk, &escaped);
                if (escaped) {
                    bw_unary(&bw, SJ6_QESC);
                    bw_put(&bw, u, 32);
                } else {
                    bw_unary(&bw, u >> bestk);
                    bw_put(&bw, u & ((1u << bestk) - 1u), bestk);
                }
                push_hist(h, order, x[pos + j]);
            }
            memcpy(hist, h, sizeof hist);
        }
        if (bw.overflow) return 0;
        pos += len;
    }
    bw_flush(&bw);
    if (bw.overflow) return 0;
    return head + bw.n;
}

size_t sj6_pcm_encode(const int32_t *x, int n, int bits, int rate,
                      uint8_t *out, size_t cap, int *order_used)
{
    static const int orders[] = {0, 1, 4, 8, 12, 16, 255};
    size_t best = 0;
    int best_order = -1;
    int oi;
    uint8_t *scratch;
    if (!x || n <= 0 || (bits != 16 && bits != 24) || !out || cap < 32) return 0;
    scratch = (uint8_t *)malloc(cap);
    if (!scratch) return 0;
    for (oi = 0; oi < (int)(sizeof orders / sizeof orders[0]); oi++) {
        int order = orders[oi];
        int16_t c[SJ6_PCM_ORDER_MAX];
        double r[SJ6_PCM_ORDER_MAX + 1], a[SJ6_PCM_ORDER_MAX + 1];
        size_t got;
        memset(c, 0, sizeof c);
        if (order == 255) {
            size_t packed = 18u + (size_t)n * (size_t)(bits / 8);
            size_t i;
            if (packed > cap || (best && packed >= best)) continue;
            memcpy(out, "J6R1", 4);
            out[4] = 1;
            out[5] = (uint8_t)bits;
            write_u32(out + 6, (uint32_t)rate);
            write_u32(out + 10, (uint32_t)n);
            out[14] = 255;
            out[15] = 0;
            write_u16(out + 16, 0);
            for (i = 0; i < (size_t)n; i++) {
                uint32_t u = (uint32_t)x[i];
                size_t o = 18 + i * (size_t)(bits / 8);
                out[o] = (uint8_t)u;
                out[o + 1] = (uint8_t)(u >> 8);
                if (bits == 24) out[o + 2] = (uint8_t)(u >> 16);
            }
            if (!best || packed < best) { best = packed; best_order = 255; }
            continue;
        }
        if (order == 1) c[0] = (int16_t)(1 << SJ6_SHIFT);
        else if (order > 1) {
            autocorr(x, n, order, r);
            if (levinson(r, order, a) != 0) continue;
            quantize(a, order, c);
        }
        got = encode_order(x, n, bits, rate, order, c, scratch, cap);
        if (!got) continue;
        if (!best || got < best) {
            best = got;
            best_order = order;
            memcpy(out, scratch, got);
        }
    }
    free(scratch);
    if (order_used) *order_used = best_order;
    return best;
}

int sj6_pcm_decode(const uint8_t *in, size_t len, int32_t *x, int maxn,
                   int *bits_out, int *rate_out, Sj6PcmStats *stats)
{
    int order, bits, rate, n, shift, pos;
    int16_t c[SJ6_PCM_ORDER_MAX];
    int32_t hist[SJ6_PCM_ORDER_MAX];
    BitR br;
    size_t head;
    int i;
    if (stats) memset(stats, 0, sizeof *stats);
    if (!in || len < 18 || !x || memcmp(in, "J6R1", 4) || in[4] != 1) return -1;
    bits = in[5];
    rate = (int)read_u32(in + 6);
    n = (int)read_u32(in + 10);
    order = in[14];
    shift = in[15];
    if ((bits != 16 && bits != 24) || n <= 0 || n > maxn) return -1;
    if (order == 255) {
        size_t packed = 18u + (size_t)n * (size_t)(bits / 8);
        int i;
        if (len < packed || shift != 0) return -1;
        for (i = 0; i < n; i++) {
            const uint8_t *p = in + 18 + (size_t)i * (size_t)(bits / 8);
            int32_t s = p[0] | (p[1] << 8);
            if (bits == 24) {
                s |= p[2] << 16;
                if (s & 0x800000) s |= ~0xffffff;
            } else {
                s = (int16_t)s;
            }
            x[i] = s;
        }
        if (stats) {
            stats->samples = (uint64_t)n;
            stats->order = 255;
            stats->stored_bytes = packed;
            stats->state_bytes = 16;
            stats->adds = (uint64_t)n;
        }
        if (bits_out) *bits_out = bits;
        if (rate_out) *rate_out = rate;
        return n;
    }
    if (order < 0 || order > SJ6_PCM_ORDER_MAX || shift != SJ6_SHIFT)
        return -1;
    head = 16 + (size_t)order * 2 + 2;
    if (len < head || read_u16(in + 16 + order * 2) != SJ6_BLOCK) return -1;
    for (i = 0; i < order; i++) c[i] = (int16_t)read_u16(in + 16 + i * 2);
    memset(hist, 0, sizeof hist);
    memset(&br, 0, sizeof br);
    br.p = in + head;
    br.end = in + len;
    br.st = stats;
    if (stats) {
        stats->order = order;
        stats->shift = shift;
        stats->stored_bytes = len;
        stats->state_bytes = (size_t)order * (sizeof(int32_t) + sizeof(int16_t)) + 16;
        stats->samples = (uint64_t)n;
    }
    for (pos = 0; pos < n; ) {
        int lenb = n - pos;
        int k, j;
        if (lenb > SJ6_BLOCK) lenb = SJ6_BLOCK;
        k = (int)br_get(&br, 4);
        if (br.bad) return -1;
        for (j = 0; j < lenb; j++) {
            uint32_t q = br_unary(&br);
            uint32_t u;
            int32_t pred, sample;
            if (br.bad) return -1;
            if (q < SJ6_QESC) {
                uint32_t rem = k ? br_get(&br, k) : 0;
                u = (q << k) | rem;
            } else {
                u = br_get(&br, 32);
            }
            if (br.bad) return -1;
            pred = order ? predict(hist, c, order, stats) : 0;
            sample = pred + unzig(u);
            if (stats) stats->adds++;
            x[pos + j] = sample;
            push_hist(hist, order, sample);
        }
        pos += lenb;
    }
    if (bits_out) *bits_out = bits;
    if (rate_out) *rate_out = rate;
    return n;
}
