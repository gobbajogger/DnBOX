/* SPDX-License-Identifier: GPL-3.0-only */
/* The 128-word blocks are immutable. Each header gives the first word,
 * first delta, minimum second difference, and residual width in bits. */
#include "chicago_dense_private.h"

static uint32_t dense_u24(const uint8_t *p)
{
    return (uint32_t)p[0] | (uint32_t)p[1] << 8 | (uint32_t)p[2] << 16;
}

static uint32_t dense_s24(const uint8_t *p)
{
    return (dense_u24(p) ^ 0x800000u) - 0x800000u;
}

static uint32_t dense_word(const uint8_t *data, const uint16_t *offset, uint32_t index)
{
    const uint8_t *p = data + offset[index >> 7];
    unsigned count = index & 127u;
    uint32_t value = dense_u24(p) | (uint32_t)p[3] << 24;
    if (!count) return value;
    uint32_t delta = dense_s24(p + 4);
    uint32_t minimum = dense_s24(p + 7);
    unsigned width = p[10];
    /* Add the base quadratic in modular integer arithmetic. */
    value += count * delta + (count * (count - 1u) / 2u) * minimum;
    if (!width || count == 1) return value;
    const uint32_t mask = (1u << width) - 1u;
    uint32_t reservoir = 0, slope = 0;
    unsigned available = 0;
    p += 11;
    for (unsigned n = 1; n < count; ++n) {
        while (available < width) {
            reservoir |= (uint32_t)*p++ << available;
            available += 8;
        }
        slope += reservoir & mask;
        value += slope;
        reservoir >>= width;
        available -= width;
    }
    return value;
}
