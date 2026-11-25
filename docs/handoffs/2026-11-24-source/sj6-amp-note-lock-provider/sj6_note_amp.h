/* SPDX-License-Identifier: GPL-3.0-only */
#pragma once
#include <stdint.h>

/* Version-2 Juno-only note-lock overlay replacing the derived identity tag.
 * The surrounding lane validator owns canonical cell and identity checks.
 * Byte 8: presence[3:0], link[5:4] (0 absent, 1 off, 2 on).
 * Byte 9 stores the canonical edited flag (existing meaning, not an override).
 * Byte 10 and absent words must be zero; byte 11 is 0xa2 (v1 tag forbids it). No arithmetic on Q16 values. */
enum { SJ6_NOTE_AMP_VERSION = 2, SJ6_NOTE_AMP_BYTES = 12, SJ6_NOTE_AMP_MARK = 0xa2 };
typedef struct {
    uint16_t q16[4];
    uint8_t mask, link;
} sj6_note_amp_t;

static inline int sj6_note_amp_decode(sj6_note_amp_t *out, const uint8_t b[12],
                                      unsigned edited)
{
    unsigned mask = b[8] & 15u, link = b[8] >> 4;
    if (link > 2 || (!mask && !link) || edited > 1 ||
        b[9] != edited || b[10] || b[11] != SJ6_NOTE_AMP_MARK)
        return 0;
    sj6_note_amp_t next = {{0,0,0,0}, (uint8_t)mask, (uint8_t)link};
    for (unsigned i = 0; i < 4; ++i) {
        unsigned w = (unsigned)b[2*i] | (unsigned)b[2*i+1] << 8;
        if (w && !(mask & (1u << i))) return 0;
        next.q16[i] = (uint16_t)w;
    }
    *out = next;
    return 1;
}

static inline int sj6_note_amp_encode(uint8_t b[12], const sj6_note_amp_t *s,
                                      unsigned edited)
{
    if (s->mask > 15 || s->link > 2 || (!s->mask && !s->link) || edited > 1)
        return 0;
    for (unsigned i = 0; i < 4; ++i) {
        unsigned w = (s->mask & (1u << i)) ? s->q16[i] : 0u;
        b[2*i] = (uint8_t)w;
        b[2*i+1] = (uint8_t)(w >> 8);
    }
    b[8] = (uint8_t)(s->mask | s->link << 4);
    b[9] = (uint8_t)edited;
    b[10] = 0;
    b[11] = SJ6_NOTE_AMP_MARK;
    return 1;
}
