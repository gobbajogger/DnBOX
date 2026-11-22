/* SPDX-License-Identifier: GPL-3.0-only */
#include "bank.h"
#include <stdint.h>
#include <string.h>

struct bank {
    const char *name;
    uint16_t first_position, first_row, count;
};
struct field {
    uint16_t first_word, bit;
    uint8_t parameter, width;
};
#include "s1_factory_bank.inc"

_Static_assert(sizeof(float) == sizeof(uint32_t), "factory data requires binary32");
_Static_assert(sizeof fields / sizeof fields[0] == S1_FACTORY_COMMANDS, "factory schema mismatch");

int s1_factory_lookup(const char *bank, unsigned position)
{
    if (!bank)
        return -1;
    for (unsigned i = 0; i < sizeof banks / sizeof banks[0]; ++i) {
        if (strcmp(bank, banks[i].name) == 0) {
            unsigned offset = position - banks[i].first_position;
            return offset < banks[i].count ? (int)(banks[i].first_row + offset) : -1;
        }
    }
    return -1;
}

const char *s1_factory_name(unsigned program)
{
    return program < PROGRAM_COUNT ? (const char *)&names[name_offsets[program]] : 0;
}

int s1_factory_read(unsigned program, unsigned command, unsigned *parameter, float *physical)
{
    if (program >= PROGRAM_COUNT || command >= S1_FACTORY_COMMANDS || !parameter || !physical)
        return 0;
    const struct field *f = &fields[command];
    const uint8_t *p = rows + program * ROW_BYTES + f->bit / 8;
    unsigned window = (unsigned)p[0] | ((unsigned)p[1] << 8) | ((unsigned)p[2] << 16);
    unsigned index = (window >> (f->bit % 8)) & ((1u << f->width) - 1);
    uint32_t word = words[f->first_word + index];
    memcpy(physical, &word, sizeof word);
    *parameter = f->parameter;
    return 1;
}
