/* SPDX-License-Identifier: GPL-3.0-only */
/* Exact recording codec for SuperJ6-style PCM.
 *
 * Generated voices (saw, triangle, sine, pulse, noise) are formulas and are
 * not stored here. A sampled zone is a recording: an integer linear-prediction
 * model plus a Rice-coded integer residual. Decode adds the residual back, so
 * the original integer samples return unchanged. No float sample is rebuilt
 * by rounding a model.
 *
 * The private 12-record SuperJ6 asset is not in this tree. This codec is the
 * container for that asset and for any other integer PCM of 16 or 24 bits.
 */
#ifndef SJ6_PCM_H
#define SJ6_PCM_H
#include <stddef.h>
#include <stdint.h>

enum { SJ6_PCM_ORDER_MAX = 16 };

typedef struct Sj6PcmStats {
    uint64_t samples;
    uint64_t muls;
    uint64_t adds;
    uint64_t shifts;
    uint64_t bit_reads;
    uint64_t branches;
    int order;
    int shift;
    size_t stored_bytes;
    size_t state_bytes;
} Sj6PcmStats;

/* Encode n integer samples (the original codes, already sign-extended).
 * bits is 16 or 24. Returns the stored size, or 0 if out is too small.
 * The decoder reproduces x[] exactly. */
size_t sj6_pcm_encode(const int32_t *x, int n, int bits, int rate,
                      uint8_t *out, size_t cap, int *order_used);

/* Decode into x[0..maxn). Returns the sample count, or -1 on a bad blob.
 * stats may be null. On success every sample matches the encoder input. */
int sj6_pcm_decode(const uint8_t *in, size_t len, int32_t *x, int maxn,
                   int *bits, int *rate, Sj6PcmStats *stats);

#endif
