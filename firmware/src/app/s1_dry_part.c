/* SPDX-License-Identifier: GPL-3.0-only */
#include "s1_dry_part.h"
#include "../dsp/fastmath.h"
#include <string.h>

void s1_part_init(s1_dry_part *part)
{
    memset(part, 0, sizeof *part);
    part->bend = 0.5f;
}

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

int s1_part_param(s1_dry_part *part, unsigned id, float value)
{
    if (id > 1 || !(value >= 0.0f && value <= 1.0f)) return 0;
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
    s1_dry_controls controls;
    s1_dry_frozen_controls(&controls);
    s1_dry_reset(voice);
    if (s1_dry_configure(voice, &controls) != S1_DRY_OK ||
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
    return 0;
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
}
