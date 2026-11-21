/* SPDX-License-Identifier: GPL-3.0-only */
#include "s1_dry_part.h"
#include "../../../experiments/sound-hardware-v1/synthone-widen/widen.h"
#include "../dsp/fastmath.h"
#include <stddef.h>
#include "original_sound.h"
#include <string.h>

void s1_part_init(s1_dry_part *part)
{
    memset(part, 0, sizeof *part);
    part->bend = 0.5f;
    part->crush_interval = 1.f;
    part->pan.rate_target = .25f;
#if X0X_SYNTHONE_PATCH
    s1_dry_frozen_controls(&part->patch);
#endif
}

/* The backend union contains the last 840 frames. Its owner's retired Chicago
 * insert supplies the first 1365 frames. A dry-only host can omit that span. */
typedef char s1_width_complete_history[
    3 * S1_WIDTH_FIRST_FRAMES + S1_WIDTH_TAIL_BYTES == S1_WIDEN_BYTES ? 1 : -1];

void s1_part_bind_width(s1_dry_part *part, uint8_t *history)
{
    part->width.first = history;
    if (history) memset(history, 0, 3u * S1_WIDTH_FIRST_FRAMES);
}

void s1_part_all_off(s1_dry_part *part)
{
    /* Panic clears notes and performance inputs but leaves the patch playing
     * on this owner available to the next note. */
    float interval = part->crush_interval;
#if X0X_SYNTHONE_PATCH
    memset(part, 0, offsetof(s1_dry_part, patch));
#else
    memset(part, 0, offsetof(s1_dry_part, width));
#endif
    if (part->width.first) {
        memset(part->width.first, 0, 3u * S1_WIDTH_FIRST_FRAMES);
        memset(part->width.tail, 0, sizeof part->width.tail);
        part->width.position = part->width.pending = 0;
    }
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
static const float crush_intervals[128] = {
    0x1.000000p+0f, 0x1.000000p+0f, 0x1.000000p+0f, 0x1.000000p+0f,
    0x1.000000p+0f, 0x1.000000p+0f, 0x1.000000p+0f, 0x1.000000p+0f,
    0x1.000000p+0f, 0x1.000000p+0f, 0x1.000000p+0f, 0x1.007742p+0f,
    0x1.02975cp+0f, 0x1.04c098p+0f, 0x1.06f32ep+0f, 0x1.092f42p+0f,
    0x1.0b752cp+0f, 0x1.0dc532p+0f, 0x1.101f7ep+0f, 0x1.12846ep+0f,
    0x1.14f432p+0f, 0x1.176f2cp+0f, 0x1.19f59cp+0f, 0x1.1c87cep+0f,
    0x1.1f2630p+0f, 0x1.21d0f4p+0f, 0x1.24888ep+0f, 0x1.274d5ep+0f,
    0x1.2a1fa8p+0f, 0x1.2cffecp+0f, 0x1.2fee7ep+0f, 0x1.32ebd8p+0f,
    0x1.35f872p+0f, 0x1.3914a2p+0f, 0x1.3c4102p+0f, 0x1.3f7e0cp+0f,
    0x1.42cc34p+0f, 0x1.462c0ep+0f, 0x1.499e26p+0f, 0x1.4d2314p+0f,
    0x1.50bb7cp+0f, 0x1.5467f8p+0f, 0x1.582924p+0f, 0x1.5bffcap+0f,
    0x1.5fec9cp+0f, 0x1.63f062p+0f, 0x1.680bd4p+0f, 0x1.6c3fd8p+0f,
    0x1.708d46p+0f, 0x1.74f518p+0f, 0x1.797822p+0f, 0x1.7e177ep+0f,
    0x1.82d424p+0f, 0x1.87af54p+0f, 0x1.8caa08p+0f, 0x1.91c598p+0f,
    0x1.970348p+0f, 0x1.9c6468p+0f, 0x1.a1ea7ap+0f, 0x1.a796e6p+0f,
    0x1.ad6b52p+0f, 0x1.b3695ap+0f, 0x1.b992e6p+0f, 0x1.bfe9aap+0f,
    0x1.c66fbap+0f, 0x1.cd2712p+0f, 0x1.d41220p+0f, 0x1.db3302p+0f,
    0x1.e28c6cp+0f, 0x1.ea20f4p+0f, 0x1.f1f366p+0f, 0x1.fa06c2p+0f,
    0x1.012f26p+1f, 0x1.057ebep+1f, 0x1.09f3ecp+1f, 0x1.0e90b0p+1f,
    0x1.135724p+1f, 0x1.18498cp+1f, 0x1.1d6a46p+1f, 0x1.22bbe8p+1f,
    0x1.284148p+1f, 0x1.2dfd58p+1f, 0x1.33f36ap+1f, 0x1.3a26dep+1f,
    0x1.409ba2p+1f, 0x1.4755b6p+1f, 0x1.4e59a8p+1f, 0x1.55ac34p+1f,
    0x1.5d52c4p+1f, 0x1.655302p+1f, 0x1.6db34ap+1f, 0x1.767a7cp+1f,
    0x1.7fb038p+1f, 0x1.895ce6p+1f, 0x1.9389a2p+1f, 0x1.9e40bcp+1f,
    0x1.a98d8cp+1f, 0x1.b57c76p+1f, 0x1.c21bccp+1f, 0x1.cf7b12p+1f,
    0x1.ddabeap+1f, 0x1.ecc258p+1f, 0x1.fcd4a0p+1f, 0x1.06fe36p+2f,
    0x1.102b52p+2f, 0x1.1a025ap+2f, 0x1.249644p+2f, 0x1.2ffd4ep+2f,
    0x1.3c5104p+2f, 0x1.49af80p+2f, 0x1.583c1ep+2f, 0x1.68209ap+2f,
    0x1.798ef2p+2f, 0x1.8cc334p+2f, 0x1.a2066ap+2f, 0x1.b9b212p+2f,
    0x1.d43526p+2f, 0x1.f21b02p+2f, 0x1.0a0a90p+3f, 0x1.1d8450p+3f,
    0x1.3411ccp+3f, 0x1.4e7d6ap+3f, 0x1.6dde20p+3f, 0x1.93bde2p+3f,
    0x1.c25ca4p+3f, 0x1.fd2792p+3f, 0x1.24cd0cp+4f, 0x1.588800p+4f,
};

int s1_part_param(s1_dry_part *part, unsigned id, float value)
{
    if (id >= ORIGINAL_SYNTHONE_NPARAMS || !(value >= 0.0f && value <= 1.0f)) return 0;
    unsigned position = (unsigned)(value * 127.f + .5f);
    if (id == ORIGINAL_SYNTHONE_PAN_RATE) {
        part->pan.rate_target = s1_pan_hz(position);
        return 1;
    }
    if (id == ORIGINAL_SYNTHONE_PAN) {
        /* Position 0 recalls legacy dual-unity bypass. The 127 active
         * positions span the original 0..1 depth including true center. */
        part->pan.engaged = position != 0;
        part->pan.depth_target = position ? (float)(position - 1u) / 126.f : 0.f;
        return 1;
    }
    if (id == ORIGINAL_SYNTHONE_WIDTH) {
        if (value != 0.f && !part->width.first) return 0;
        part->width.target = value;
        return 1;
    }
    if (id == ORIGINAL_SYNTHONE_CRUSH) {
        part->crush_interval = crush_intervals[position];
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

static void widen_right(s1_stereo_width *width, float *right, unsigned frames)
{
    if (!width->first) return;
    unsigned position = width->position, pending = width->pending;
    float wet = width->applied;
    for (unsigned i = 0; i < frames; ++i) {
        float input = right[i];
        float delay = s1_widen_tap(width->first, S1_WIDTH_FIRST_FRAMES, width->tail, &position, input);
        if (input != 0.f) pending = S1_WIDEN_FRAMES;
        else if (pending) --pending;
        /* Soundpipe port.c uses a 0.1-second half-time at 44.1 kHz. */
        if (wet != 0.f || width->target != 0.f) {
            wet = 0x1.49a000p-13f * width->target + 0x1.ffeb66p-1f * wet;
            right[i] = wet * delay + (1.f - wet) * input;
        }
    }
    width->position = position;
    width->pending = pending;
    width->applied = wet;
}

/* Soundpipe sp_port, sp_osc and sp_pan2(type 0), at the original 44.1 kHz.
 * Compute the endpoint sines instead of storing another 4096-word table. */
static float pan_port(float current, float target)
{
    return 0x1.49a000p-13f * target + 0x1.ffeb66p-1f * current;
}

static unsigned pan_increment(float hz)
{
    /* Nearest-even for the nonnegative 24-bit phase increment. */
    float value = hz * (16777216.f / 44100.f);
    fm_bits_t nearest = {.f = value + 0x1.8p23f};
    return nearest.u - 0x4b400000u;
}

static void pan_stereo(s1_dry_part *part, float *left, float *right, unsigned frames)
{
    unsigned phase = part->pan.phase;
    float rate = part->pan.rate;
    unsigned mix = part->pan.mix;
    if (!part->pan.engaged && !mix) {
        /* Keep time even during silent and legacy dry blocks. Once the
         * binary32 port reaches its fixed point advance a block at once. */
        for (unsigned i = 0; i < frames; ++i) {
            float next = pan_port(rate, part->pan.rate_target);
            unsigned step = pan_increment(next);
            if (next == rate) {
                phase += step * (frames - i);
                break;
            }
            rate = next;
            phase += step;
        }
        /* A bypassed depth port may still be decaying after a live edit.
         * Its setting has no effect on the untouched INIT path. */
        if (part->pan.depth != 0.f)
            for (unsigned i = 0; i < frames; ++i) part->pan.depth = pan_port(part->pan.depth, 0.f);
    } else {
        float depth = part->pan.depth;
        for (unsigned i = 0; i < frames; ++i) {
            rate = pan_port(rate, part->pan.rate_target);
            depth = pan_port(depth, part->pan.depth_target);
            unsigned index = (phase >> 12) & 4095u;
            float a = fm_sin_turns((float)index / 4096.f);
            float b = fm_sin_turns((float)((index + 1u) & 4095u) / 4096.f);
            float fraction = (float)(phase & 4095u) / 4096.f;
            float pan = (a + fraction * (b - a)) * depth;
            phase += pan_increment(rate);
            float angle = (1.f + pan) * (FM_PI * .25f);
            float input = left[i];
            float l = input * fm_sin_pi(FM_PI * .5f - angle);
            float r = input * fm_sin_pi(angle);
            if (part->pan.engaged) { if (mix < 64u) ++mix; }
            else if (mix) --mix;
            if (mix < 64u) {
                float wet = (float)mix / 64.f;
                l = input + wet * (l - input);
                r = input + wet * (r - input);
            }
            left[i] = l;
            right[i] = r;
        }
        part->pan.depth = depth;
    }
    part->pan.phase = phase & 0xffffffu;
    part->pan.rate = rate;
    part->pan.mix = (uint8_t)mix;
    widen_right(&part->width, right, frames);
}

void s1_part_render(s1_dry_part *part, float *left, float *right, unsigned frames)
{
    if (!frames) return;
    memset(left, 0, frames * sizeof *left);
    for (unsigned i = 0; i < S1_CORE_VOICES; ++i) {
        s1_dry_begin_block(&part->voice[i]);
        if (part->voice[i].started)
            s1_dry_mix(&part->voice[i], left, frames);
    }
    if (left != right) memcpy(right, left, frames * sizeof *left);

    /* The untouched INIT rate takes every input frame. Keep the last sample
     * for a live rate change without running the hold loop on a dry block. */
    if (part->crush_interval == 1.f && part->crush_age == 1.f && part->crush_next == 1.f) {
        part->crush_last = left[frames - 1u];
        pan_stereo(part, left, right, frames);
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
    pan_stereo(part, left, right, frames);
}
