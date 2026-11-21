/* SPDX-License-Identifier: GPL-3.0-only */
#pragma once
#include <stdint.h>

typedef struct {
    unsigned bit;
    unsigned field;
    unsigned end;
} sj6_record_reader;

/* Foreground selection by original identities, or by bank and authored position.
 * The cursor belongs to the caller. A failed selection leaves it untouched. */
int sj6_select_uid(const uint8_t bank[16], const uint8_t program[16], sj6_record_reader *);
int sj6_select_position(unsigned bank, unsigned position, sj6_record_reader *);
/* Iterates the authored field order. Returns zero after the final field.
 * IDs 218/219 encode named legacy fields for the data reader only. Globals
 * and legacy fields are retained here, independent of their DSP meaning. */
int sj6_next_field(sj6_record_reader *, unsigned *id, uint32_t *binary32);
