/* SPDX-License-Identifier: GPL-3.0-only */
#ifndef X0X_BREAK_RETRIGGER_PACKED_H
#define X0X_BREAK_RETRIGGER_PACKED_H
#include <stdint.h>

/* Four neighboring pot settings share one group. The first twelve bytes
 * contain four little-endian unsigned Q24 probabilities. Byte 12 holds one
 * two-bit biased residual per position, lowest pot first. */
static const uint8_t brk_retrigger_groups[3][25][13] = {
#include "break_retrigger_pairs.inc"
};

static inline float brk_retrigger_probability(unsigned length, unsigned pot)
{
    if (pot >= 100) return 1.0f;
    const uint8_t *group = brk_retrigger_groups[length / 2][pot / 4];
    unsigned position = pot % 4;
    const uint8_t *bytes = group + 3 * position;
    uint32_t q = bytes[0] | ((uint32_t)bytes[1] << 8) | ((uint32_t)bytes[2] << 16);
    if (length & 1u) {
        uint32_t square = (uint32_t)(((uint64_t)q * q) >> 24);
        unsigned residual = (group[12] >> (2 * position)) & 3u;
        q = 2u * q - square + residual - 2u;
    }
    return (float)q * 0x1p-24f;
}
#endif
