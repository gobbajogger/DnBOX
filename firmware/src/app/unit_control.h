/* SPDX-License-Identifier: GPL-3.0-only */
#pragma once
#include <stdint.h>
#include <string.h>

/* Correctly rounded raw / 127 for a seven-bit control. Convert through Q31
 * to avoid the target's fast-math reciprocal approximation. 2^31 divided by
 * 127 has quotient 16909320 and remainder 8. Rounding the remainder for each
 * raw value 0..127 gives (raw+8)/16 (integer division). The power-of-two
 * scale preserves the conversion's single binary32 rounding. */
static inline float unit_control(unsigned raw)
{
    uint32_t q31 = raw * 16909320u + (raw + 8u) / 16u;
    return (float)q31 * 0x1p-31f;
}

/* The rounded binary32 reciprocal of 10000 is slightly low. These bits
 * retain the exact historical gain for the detents where multiplying the
 * square by that reciprocal rounds one ULP below the original division. */
static inline float mix_level_control(unsigned v)
{
    static const uint8_t mix_level_round_up[16] = {
        0x0, 0x80, 0xa0, 0xc0, 0xa, 0x44, 0x80, 0x58, 0xce, 0x80, 0xb0, 0x1a, 0x2, 0x40, 0xc2, 0x11
    };
    float gain = (float)(v * v) * 0x1.a36e2ep-14f;
    uint32_t bits;
    memcpy(&bits, &gain, sizeof bits);
    bits += (mix_level_round_up[v >> 3] >> (v & 7)) & 1u;
    memcpy(&gain, &bits, sizeof gain);
    return gain;
}
