/* SPDX-License-Identifier: GPL-3.0-only
 * Per-pattern synth-slot preset in the existing one-byte rsv[voice] field.
 * Never change the project stream layout for an engine choice.
 *
 * Historical encodings are immutable:
 *     0             = no recall
 *     1..81         = Chicago factory (one-based)
 *     129..158      = Miami factory (0x80 | one-based)
 *     0xC1..0xC4    = historical slot-2 codes; all load as Synth One init
 *     0xE1..0xE4    = Juno factory (one-based)
 *     0xE5          = source-owned SUPERJ6 WIDE (build opt-in)
 * Codes outside these ranges are malformed, not guessed.
 * New Synth One recalls write only 0xC1 (init). 0xC2..0xC4 remain load-only.
 */
#pragma once
#include <stdint.h>
#include "original_sound.h"

static inline unsigned bass_pattern_preset_encode(unsigned kind, unsigned one_based)
{
    if (!one_based) return 0u;
    if (one_based > original_sound_factories(kind)) return 0u;
    if (kind == ORIGINAL_SOUND_JUNO) return 0xE0u | one_based;
    if (kind == ORIGINAL_SOUND_SYNTHONE) return one_based == 1u ? 0xC1u : 0u;
    if (kind == ORIGINAL_SOUND_CHICAGO || kind == ORIGINAL_SOUND_MIAMI)
        return (kind << 7) | one_based;
    return 0u;
}

static inline int bass_pattern_preset_decode(unsigned code, unsigned *kind, unsigned *one_based)
{
    if (!kind || !one_based || code > 255u) return 0;
    if (!code) { *kind = *one_based = 0u; return 1; }
    if (code >= 0xC1u && code <= 0xC4u) {
        *kind = ORIGINAL_SOUND_SYNTHONE;
        *one_based = 1u; /* historical factories all become init */
        return 1;
    }
    unsigned juno = (code & 0xE0u) == 0xE0u;
    unsigned k = juno ? ORIGINAL_SOUND_JUNO : code >> 7;
    unsigned p = juno ? code & 0x1Fu : code & 0x7Fu;
    if (k == ORIGINAL_SOUND_SYNTHONE) return 0;
    if (!p || p > original_sound_factories(k)) return 0;
    *kind = k; *one_based = p;
    return 1;
}
