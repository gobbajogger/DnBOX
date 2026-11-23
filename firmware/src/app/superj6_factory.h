/* SPDX-License-Identifier: GPL-3.0-only */
#pragma once
#include <stdint.h>

/* A foreground reader starts each program at {0} and feeds its authored
 * fields in record order. The first occurrence of each supported ID wins.
 * Presence distinguishes authored +0/-0 from an absent PWM word. */
enum {
    SJ6_ROW_AMP2 = 0, SJ6_ROW_MASTER = 4, SJ6_ROW_WAVE = 5,
    SJ6_ROW_AMOUNT = 7, SJ6_ROW_ENABLE = 9, SJ6_ROW_COUNT = 11
};
typedef struct {
    float value[SJ6_ROW_COUNT];
    uint16_t present;
} sj6_factory_row;

static inline void sj6_factory_add_word(sj6_factory_row *row, unsigned id, uint32_t bits)
{
    unsigned slot;
    if (id == 0) slot = SJ6_ROW_MASTER;
    else if (id >= 119 && id <= 122) slot = SJ6_ROW_AMP2 + id - 119;
    else if (id == 82 || id == 83) slot = SJ6_ROW_WAVE + id - 82;
    else if (id == 78 || id == 79) slot = SJ6_ROW_AMOUNT + id - 78;
    else if (id == 210 || id == 211) slot = SJ6_ROW_ENABLE + id - 210;
    else return;
    unsigned bit = 1u << slot;
    if (!(row->present & bit)) {
        __builtin_memcpy(&row->value[slot], &bits, sizeof bits);
        row->present |= (uint16_t)bit;
    }
}
