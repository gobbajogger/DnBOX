/* SPDX-License-Identifier: GPL-3.0-only */
#include "s1_dry_part.h"
#include "../dsp/fastmath.h"
#include <stddef.h>
#if X0X_SYNTHONE_PATCH
#include "original_sound.h"
#endif
#include <string.h>

void s1_part_init(s1_dry_part *part)
{
    memset(part, 0, sizeof *part);
    part->bend = 0.5f;
    part->crush_interval = 1.f;
#if X0X_SYNTHONE_PATCH
    s1_dry_frozen_controls(&part->patch);
#endif
}

void s1_part_all_off(s1_dry_part *part)
{
    /* Panic clears notes and performance inputs but leaves the patch playing
     * on this owner available to the next note. */
    float interval = part->crush_interval;
#if X0X_SYNTHONE_PATCH
    memset(part, 0, offsetof(s1_dry_part, patch));
#else
    memset(part, 0, sizeof *part);
#endif
    part->bend = 0.5f;
    part->crush_interval = interval;
}

#if X0X_SYNTHONE_PATCH
/* Linear 7-bit edits across the original physical domains implemented by
 * s1_dry_configure. The INIT preset retains its exact unquantized values. */
static const struct {
    s1_patch_descriptor parameter;
    uint8_t offset;
} patch_parameters[S1_PATCH_COUNT] = {
#define S1_FIELD(id, label, member, lo, hi, unit) \
    {{label, lo, hi, id, unit}, (uint8_t)offsetof(s1_dry_controls, member)}
    S1_FIELD(2, "OSC BALANCE", balance, 0, 1, X0X_U_PCT),
    S1_FIELD(5, "OSC 1 LEVEL", oscillator1_level, 0, 1, X0X_U_PCT),
    S1_FIELD(16, "CUTOFF", cutoff_hz, 64, 22050, X0X_U_HZ),
    S1_FIELD(17, "RESONANCE", resonance, 0, .98f, X0X_U_PCT),
    S1_FIELD(19, "FILTER ENV", filter_depth, 0, 1.2f, X0X_U_PCT),
    S1_FIELD(22, "FILTER ATTACK", filter.attack_s, .0005f, 2, X0X_U_S),
    S1_FIELD(23, "FILTER DECAY", filter.decay_s, .005f, 2, X0X_U_S),
    S1_FIELD(24, "FILTER SUSTAIN", filter.sustain, 0, 1, X0X_U_PCT),
    S1_FIELD(25, "FILTER RELEASE", filter.release_s, 0, 2, X0X_U_S),
    S1_FIELD(26, "AMP ATTACK", amp.attack_s, .0005f, 2, X0X_U_S),
    S1_FIELD(27, "AMP DECAY", amp.decay_s, .005f, 2, X0X_U_S),
    S1_FIELD(28, "AMP SUSTAIN", amp.sustain, 0, 1, X0X_U_PCT),
    S1_FIELD(29, "AMP RELEASE", amp.release_s, .004f, 2, X0X_U_S)
#undef S1_FIELD
};

const s1_patch_descriptor *s1_patch_at(unsigned slot)
{ return slot < S1_PATCH_COUNT ? &patch_parameters[slot].parameter : NULL; }

int s1_patch_slot(unsigned id)
{
    for (unsigned i = 0; i < S1_PATCH_COUNT; ++i)
        if (patch_parameters[i].parameter.factory_id == id) return (int)i;
    return -1;
}

float s1_patch_physical(const s1_patch_descriptor *d, unsigned position)
{
    if (position >= S1_PATCH_STEPS) return d->maximum;
    return d->minimum + (d->maximum - d->minimum) * ((float)position / S1_PATCH_STEPS);
}

float s1_part_patch_value(const s1_dry_part *part, unsigned slot)
{
    const uint8_t *base = (const uint8_t *)&part->patch;
    return *(const float *)(base + patch_parameters[slot].offset);
}

void s1_part_apply_cell(s1_dry_part *part, const uint8_t cell[32])
{
    s1_dry_controls next;
    s1_dry_frozen_controls(&next);
    uint32_t mask = original_sound_edit_mask(cell);
    for (unsigned i = 0; i < S1_PATCH_COUNT; ++i) {
        unsigned byte = S1_PATCH_FIRST_BYTE + i;
        if (mask & (UINT32_C(1) << byte)) {
            float *field = (float *)((uint8_t *)&next + patch_parameters[i].offset);
            *field = s1_patch_physical(s1_patch_at(i), cell[byte]);
        }
    }
    if (!memcmp(&next, &part->patch, sizeof next)) return;
    part->patch = next;
    for (unsigned i = 0; i < S1_CORE_VOICES; ++i)
        if (part->voice[i].started) (void)s1_dry_configure(&part->voice[i], &next);
}
#endif

static float pitch_hz(const s1_dry_part *part, unsigned key)
{
    /* One equal-tempered octave at A4 = 440 Hz, rounded to binary32.
     * Scaling by an integer power of two preserves every unbent note. */
    static const float octave_hz[12] = {
        8.17579937f, 8.661956787f, 9.177023888f, 9.722718239f,
        10.30086136f, 10.91338253f, 11.56232548f, 12.24985695f,
        12.97827148f, 13.75f, 14.56761742f, 15.43385315f
    };
    float unbent = octave_hz[key % 12u] * (float)(1u << (key / 12u));
    return unbent * fm_exp2f((part->bend - 0.5f) * (4.0f / 12.0f));
}

/* Original 44.1 kHz hold intervals for the 128 front-panel positions.
 * The original rounds log2(hz) to binary32 before exp2; the few ulps
 * change which frame is held over a sustained note. */
// IEEE binary32 low words, then 5-bit deltas of the monotone high words.
// The first 11 slots are unity; high-word checkpoints bound cold decode
// to 15 additions. The sample loop never decodes this table.
static const uint16_t crush_low[117] = {
    0x3ba1, 0x4bae, 0x604c, 0x7997, 0x97a1, 0xba96, 0xe299, 0x0fbf,
    0x4237, 0x7a19, 0xb796, 0xface, 0x43e7, 0x9318, 0xe87a, 0x4447,
    0xa6af, 0x0fd4, 0x7ff6, 0xf73f, 0x75ec, 0xfc39, 0x8a51, 0x2081,
    0xbf06, 0x661a, 0x1607, 0xcf13, 0x918a, 0x5dbe, 0x33fc, 0x1492,
    0xffe5, 0xf64e, 0xf831, 0x05ea, 0x1fec, 0x46a3, 0x7a8c, 0xbc11,
    0x0bbf, 0x6a12, 0xd7aa, 0x5504, 0xe2cc, 0x81a4, 0x3234, 0xf53d,
    0xcb73, 0xb5a9, 0xb4ad, 0xc973, 0xf4d5, 0x37dd, 0x9389, 0x0910,
    0x9981, 0x4636, 0x107a, 0xf9b3, 0x0361, 0x9793, 0xbf5f, 0xf9f6,
    0x4858, 0xab92, 0x24c6, 0xb523, 0x5df4, 0x20a4, 0xfeac, 0xf9b5,
    0x136f, 0x4dd1, 0xaadb, 0x2cd4, 0xd61a, 0xa962, 0xa981, 0xd9a5,
    0x3d3e, 0xd81c, 0xae73, 0xc4d1, 0x205e, 0xc6c6, 0xbe3b, 0x0de6,
    0xbd89, 0xd5f5, 0x612c, 0x6a50, 0x7f1b, 0x15a9, 0x012d, 0x4b22,
    0xfea7, 0x2882, 0xd7c0, 0x1e0f, 0x104d, 0xc779, 0x619a, 0x0335,
    0xd909, 0x1a93, 0x0d81, 0x0548, 0xc228, 0x08e6, 0x3eb5, 0xef10,
    0xdef1, 0x2e52, 0x93c9, 0x6686, 0x4400,
};
static const uint16_t crush_high[8] = {
    0x3f80, 0x3f93, 0x3fad, 0x3fd3, 0x4007, 0x403b, 0x4097, 0x4149,
};
static const uint8_t crush_hi_delta[75] = {
    0x20, 0x84, 0x10, 0x42, 0x10, 0x21, 0x84, 0x20, 0x42, 0x10, 0x41, 0x84,
    0x20, 0x82, 0x10, 0x41, 0x88, 0x20, 0x84, 0x10, 0x41, 0x88, 0x21, 0x84,
    0x10, 0x43, 0x88, 0x21, 0xc6, 0x10, 0x63, 0x8c, 0x31, 0xc8, 0x20, 0x83,
    0x90, 0x51, 0x86, 0x10, 0x43, 0x0c, 0x31, 0x86, 0x18, 0x64, 0x0c, 0x32,
    0x08, 0x21, 0x85, 0x94, 0x62, 0x8a, 0x39, 0xe6, 0x20, 0x54, 0x4a, 0x29,
    0xe5, 0x18, 0x84, 0x90, 0x5a, 0xcb, 0x3d, 0x96, 0x58, 0x7b, 0x13, 0x77,
    0xaa, 0x01, 0x00,
};

static float crush_interval_at(unsigned position)
{
    if (position < 11u) return 1.f;
    position -= 11u;
    uint32_t hi = crush_high[position >> 4];
    for (unsigned i = (position & ~15u) + 1u; i <= position; ++i) {
        unsigned bit = i * 5u, off = bit >> 3;
        unsigned word = crush_hi_delta[off] | ((unsigned)crush_hi_delta[off + 1u] << 8);
        hi += (word >> (bit & 7u)) & 31u;
    }
    uint32_t bits = (hi << 16) | crush_low[position];
    float interval;
    memcpy(&interval, &bits, sizeof interval);
    return interval;
}

int s1_part_param(s1_dry_part *part, unsigned id, float value)
{
    if (id > 2 || !(value >= 0.0f && value <= 1.0f)) return 0;
    if (id == 2) {
        unsigned position = (unsigned)(value * 127.f + .5f);
        part->crush_interval = crush_interval_at(position);
        return 1;
    }
    if (id == 0) {
        part->bend = value;
        for (unsigned i = 0; i < S1_CORE_VOICES; ++i) {
            if (part->voice[i].started)
                part->voice[i].phase_increment = pitch_hz(part, part->note[i]) / S1_DRY_SAMPLE_RATE;
        }
    } else {
        part->damper = value >= 0.5f;
        if (!part->damper) {
            for (unsigned i = 0; i < S1_CORE_VOICES; ++i) {
                if (part->voice[i].started && !part->down[i])
                    s1_dry_note_off(&part->voice[i]);
            }
        }
    }
    return 1;
}

int s1_part_note(s1_dry_part *part, unsigned key, unsigned velocity)
{
    if (key > 127 || velocity > 127) return 0;
    unsigned matching = S1_CORE_VOICES, unused = S1_CORE_VOICES;
    for (unsigned i = 0; i < S1_CORE_VOICES; ++i) {
        if (!part->voice[i].started) {
            if (unused == S1_CORE_VOICES) unused = i;
        } else if (part->note[i] == key) {
            matching = i;
        }
    }
    if (!velocity) {
        if (matching < S1_CORE_VOICES) {
            part->down[matching] = 0;
            if (!part->damper) s1_dry_note_off(&part->voice[matching]);
        }
        return 1;
    }
    if (matching < S1_CORE_VOICES) {
        /* A repeated pitch keeps its oscillator, envelope and filter history.
         * The next sample observes a new attack only if the gate was down. */
        s1_dry_voice *voice = &part->voice[matching];
        float fraction = (float)velocity / 127.0f;
        voice->velocity_squared = fraction * fraction;
        voice->gate = 1;
        part->down[matching] = 1;
        part->next = (matching + 1u) % S1_CORE_VOICES;
        return 1;
    }
    /* Reuse a retired voice first. An audible release occupies its slot
     * until the render boundary retires it, just like a held note. */
    unsigned slot = unused < S1_CORE_VOICES ? unused : part->next;

    s1_dry_voice *voice = &part->voice[slot];
#if X0X_SYNTHONE_PATCH
    const s1_dry_controls *controls = &part->patch;
#else
    s1_dry_controls frozen;
    s1_dry_frozen_controls(&frozen);
    const s1_dry_controls *controls = &frozen;
#endif
    s1_dry_reset(voice);
    if (s1_dry_configure(voice, controls) != S1_DRY_OK ||
        s1_dry_start(voice, pitch_hz(part, key), velocity, 0.0f) != S1_DRY_OK)
        return 0;
    part->next = (slot + 1u) % S1_CORE_VOICES;
    part->note[slot] = (uint8_t)key;
    part->down[slot] = 1;
    return 1;
}

int s1_part_active(const s1_dry_part *part)
{
    for (unsigned i = 0; i < S1_CORE_VOICES; ++i)
        if (part->voice[i].started) return 1;
    return part->crush_next > part->crush_age && part->crush_last != 0.f;
}

void s1_part_render(s1_dry_part *part, float *left, float *right, unsigned frames)
{
    if (!frames) return;
    for (unsigned i = 0; i < S1_CORE_VOICES; ++i)
        s1_dry_begin_block(&part->voice[i]);

    for (unsigned f = 0; f < frames; ++f) {
        float sample = 0.0f;
        for (unsigned i = 0; i < S1_CORE_VOICES; ++i) {
            if (part->voice[i].started) sample += s1_dry_sample(&part->voice[i]);
        }
        left[f] = right[f] = sample;
    }

    /* The untouched INIT rate takes every input frame. Keep the last sample
     * for a live rate change without running the hold loop on a dry block. */
    if (part->crush_interval == 1.f && part->crush_age == 1.f && part->crush_next == 1.f) {
        part->crush_last = left[frames - 1u];
        return;
    }
    for (unsigned f = 0; f < frames; ++f) {
        if (part->crush_next <= part->crush_age) {
            part->crush_last = left[f];
            part->crush_next += part->crush_interval;
            part->crush_next -= part->crush_age;
            part->crush_age = 0.f;
        } else {
            left[f] = right[f] = part->crush_last;
        }
        part->crush_age += 1.f;
    }
}
