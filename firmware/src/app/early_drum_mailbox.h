/* SPDX-License-Identifier: GPL-3.0-only */
#pragma once
#include <stdint.h>

/* One CPU0 word, one producer (TIMER5), one consumer (ALNK0).
 * Foreground may clear it only with both interrupts excluded. Zero is empty.
 * The scanner retains a separate physical token for foreground deduplication. */
typedef struct { volatile uint32_t word; } early_drum_mailbox;
enum { EARLY_DRUM_VOICES = 11 };

static inline int early_drum_pending(const early_drum_mailbox *m)
{
    return m->word != 0u;
}

/* Returns zero without changing either route if not admitted. The legacy
 * command queue remains single-producer. Only ordinary 909 hits are admitted. */
static inline int early_drum_publish(early_drum_mailbox *m, unsigned voice,
                                      unsigned vel, int hold, int legacy_pending)
{
    if (hold || legacy_pending || early_drum_pending(m) ||
        voice >= EARLY_DRUM_VOICES || vel == 0u || vel > 127u)
        return 0;
    m->word = (voice << 8) | vel;
    return 1;
}

static inline uint32_t early_drum_take(early_drum_mailbox *m)
{
    uint32_t word = m->word;
    if (word) m->word = 0;
    return word;
}

static inline void early_drum_cancel(early_drum_mailbox *m) { m->word = 0; }
static inline unsigned early_drum_voice(uint32_t word) { return word >> 8; }
static inline unsigned early_drum_vel(uint32_t word) { return word & 127u; }
