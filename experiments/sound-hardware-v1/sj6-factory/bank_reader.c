/* SPDX-License-Identifier: GPL-3.0-only */
#include "bank_reader.h"
#include <string.h>
#include "bank_data.inc"

#define BANK_COUNT (sizeof bank_start / sizeof bank_start[0] - 1u)

int sj6_select_position(unsigned bank, unsigned position, sj6_record_reader *r)
{
    if (!r || bank >= BANK_COUNT || position >= (unsigned)(bank_start[bank + 1] - bank_start[bank]))
        return 0;
    unsigned i = bank_start[bank] + position;
    const uint8_t *start = record_start + 3 * i;
    unsigned offset = start[0] | (unsigned)start[1] << 8 | (unsigned)start[2] << 16;
    r->bit = offset * 8u;
    r->field = layout_start[shape[i]];
    r->end = layout_start[shape[i] + 1];
    return 1;
}

int sj6_select_uid(const uint8_t bank[16], const uint8_t program[16], sj6_record_reader *r)
{
    if (!bank || !program || !r) return 0;
    for (unsigned b = 0; b < BANK_COUNT; ++b) {
        if (memcmp(bank, bank_uid + 16 * b, 16)) continue;
        for (unsigned p = bank_start[b]; p < bank_start[b + 1]; ++p)
            if (!memcmp(program, program_uid + 16 * p, 16))
                return sj6_select_position(b, p - bank_start[b], r);
        return 0;
    }
    return 0;
}

static uint32_t get_bits(sj6_record_reader *r, unsigned n)
{
    uint32_t value = 0;
    unsigned shift = 0;
    while (n) {
        unsigned within = r->bit & 7u;
        unsigned take = 8u - within;
        if (take > n) take = n;
        unsigned mask = (1u << take) - 1u;
        value |= (uint32_t)((bits[r->bit / 8u] >> within) & mask) << shift;
        r->bit += take;
        n -= take;
        shift += take;
    }
    return value;
}

int sj6_next_field(sj6_record_reader *r, unsigned *id, uint32_t *binary32)
{
    if (!r || !id || !binary32 || r->field >= r->end) return 0;
    unsigned field = field_ids[r->field++];
    unsigned code = get_bits(r, bit_width[field]);
    unsigned start = dict_start[field];
    *binary32 = code < (unsigned)dict_start[field + 1] - start ?
        words[start + code] : get_bits(r, 32);
    *id = field;
    return 1;
}
