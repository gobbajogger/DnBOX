/* SPDX-License-Identifier: GPL-3.0-only */
/* AC79 JL_RAND R64L/R64H. The software emulator models both read registers.
 * This supplies a boot incarnation, never a stable serial or credential. */
#pragma once
#include <stdint.h>

static inline uint32_t fm1_entropy_word(unsigned high)
{
    return *(volatile uint32_t *)(0x13b00u + 4u * high);
}
