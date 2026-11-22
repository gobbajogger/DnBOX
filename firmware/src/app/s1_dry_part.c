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
    part->phaser.target[1] = 12.f;
    part->phaser.target[3] = 800.f;
    part->lfo[0].increment = part->lfo[1].increment = .25f * (1.f / 44100.f);
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
    S1_FIELD(29, "AMP RELEASE", amp.release_s, .004f, 2, X0X_U_S),
    S1_FIELD(7, "SUB LEVEL", sub_level, 0, 1, X0X_U_PCT),
    S1_FIELD(8, "SUB OCTAVE", sub_extra_octave, 0, 1, X0X_U_PCT),
    S1_FIELD(9, "SUB SQUARE", sub_square, 0, 1, X0X_U_PCT),
    S1_FIELD(115, "FILTER TYPE", filter_type, 0, 2, X0X_U_NONE)
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
    if (d->factory_id == 115u) return (float)((position * 2u + 63u) / 127u);
    if (d->factory_id == 8u || d->factory_id == 9u) return position >= 64u ? 1.f : 0.f;
    if (position >= S1_PATCH_STEPS) return d->maximum;
    return d->minimum + (d->maximum - d->minimum) * ((float)position / S1_PATCH_STEPS);
}

float s1_part_patch_value(const s1_dry_part *part, unsigned slot)
{
    const uint8_t *base = (const uint8_t *)&part->patch;
    return *(const float *)(base + patch_parameters[slot].offset);
}

void s1_part_apply_cell(s1_dry_part *part, const uint8_t cell[32], const uint8_t *ext)
{
    s1_dry_controls next;
    s1_dry_frozen_controls(&next);
    uint32_t mask = original_sound_edit_mask(cell);
    for (unsigned i = 0; i < S1_PATCH_CELL_COUNT; ++i) {
        unsigned byte = S1_PATCH_FIRST_BYTE + i;
        if (mask & (UINT32_C(1) << byte)) {
            float *field = (float *)((uint8_t *)&next + patch_parameters[i].offset);
            *field = s1_patch_physical(s1_patch_at(i), cell[byte] & 127u);
        }
    }
    for (unsigned i = S1_PATCH_SUB_LEVEL; i < S1_PATCH_COUNT; ++i) {
        float *field = (float *)((uint8_t *)&next + patch_parameters[i].offset);
        *field = s1_patch_physical(s1_patch_at(i), original_sound_s1_sub_position(cell, ext, i));
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
static float crush_interval(unsigned position)
{
    /* The original rounds log2(hz) before exp2. Reconstruct the rounded
     * rational hold length and apply its small measured ULP correction.
     * This cold setter runs only when a control or stored patch changes. */
    static const uint8_t correction[64]={
        0x66,0x66,0x66,0x66,0x66,0x86,0x54,0x49,0x83,0x96,0x86,0x37,0x38,0x93,0x86,0x56,
        0x49,0x84,0x88,0x57,0xa8,0x34,0x63,0x33,0x92,0x87,0xb4,0x56,0x36,0x57,0x26,0x9a,
        0x2b,0x19,0x94,0x7a,0x96,0x57,0x74,0x68,0x58,0x49,0x36,0x37,0x77,0x58,0x84,0x33,
        0x1b,0xa9,0x83,0x50,0xa3,0xa6,0x25,0x77,0x58,0x65,0x14,0x37,0x4d,0xfb,0x77,0x61,
    };
    const unsigned numerator=44100u*127u;
    unsigned divisor=48000u*127u-45952u*position;
    if (divisor>=numerator) return 1.f;
    unsigned exponent=127;
    while (divisor*2u<=numerator) { divisor*=2u; ++exponent; }
    unsigned remainder=numerator-divisor, mantissa=0;
    for (unsigned i=0;i<23u;++i) {
        remainder*=2u;
        mantissa*=2u;
        if (remainder>=divisor) { remainder-=divisor; ++mantissa; }
    }
    if (2u*remainder>divisor || (2u*remainder==divisor && (mantissa&1u))) ++mantissa;
    unsigned delta=(correction[position>>1]>>((position&1u)*4u))&15u;
    fm_bits_t result={.u=(exponent<<23)+mantissa+(delta==15?12u:delta-6u)};
    return result.f;
}

float s1_lfo_hz(unsigned position)
{
    if (position == 0) return 1.f / 1920.f;
    if (position == 127) return 80.f;
    float offset=(float)position - 64.f;
    return .25f * fm_exp2f(offset * (position < 64 ? 8.906890596f/64.f : 8.321928095f/63.f));
}

static void master_param(s1_master *,unsigned,unsigned);

int s1_part_param(s1_dry_part *part, unsigned id, float value)
{
    if (id >= ORIGINAL_SYNTHONE_NPARAMS || !(value >= 0.0f && value <= 1.0f)) return 0;
    unsigned position = (unsigned)(value * 127.f + .5f);
    if (id == ORIGINAL_SYNTHONE_CRUSH_LFO) {
        part->crush_route = (uint8_t)((position * 3u + 63u) / 127u);
        return 1;
    }
    if (id >= ORIGINAL_SYNTHONE_MASTER_VOLUME) {
        master_param(&part->master,id-ORIGINAL_SYNTHONE_MASTER_VOLUME,position);
        return 1;
    }
    if (id >= ORIGINAL_SYNTHONE_PHASER_MIX) {
        unsigned slot=id-ORIGINAL_SYNTHONE_PHASER_MIX;
        part->phaser.target[slot]=s1_phaser_control(slot,position);
        return 1;
    }
    if (id >= ORIGINAL_SYNTHONE_TREM) {
        unsigned choice = (position * 3u + 63u) / 127u;
        if (id == ORIGINAL_SYNTHONE_TREM) part->tremolo = (uint8_t)choice;
        else {
            s1_trem_lfo *lfo = &part->lfo[id >= ORIGINAL_SYNTHONE_LFO2_SHAPE];
            if (id == ORIGINAL_SYNTHONE_LFO1_SHAPE || id == ORIGINAL_SYNTHONE_LFO2_SHAPE) lfo->shape = (uint8_t)choice;
            else if (id == ORIGINAL_SYNTHONE_LFO1_LEVEL || id == ORIGINAL_SYNTHONE_LFO2_LEVEL) lfo->level = value;
            else lfo->increment = s1_lfo_hz(position) * (1.f / 44100.f);
        }
        return 1;
    }
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
        part->crush_interval = crush_interval(position);
        part->crush_log_hz = fm_log2f(s1_part_crush_hz(value));
        return 1;
    }
    if (id == 0) {
        part->bend = value;
        for (unsigned i = 0; i < S1_CORE_VOICES; ++i) {
            if (part->voice[i].started) {
#if X0X_SYNTHONE_PATCH
                s1_dry_pitch(&part->voice[i], pitch_hz(part, part->note[i]));
#else
                part->voice[i].phase_increment = pitch_hz(part, part->note[i]) / S1_DRY_SAMPLE_RATE;
#endif
            }
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
    const uint32_t sub_phase = voice->sub.phase;
#else
    s1_dry_controls frozen;
    s1_dry_frozen_controls(&frozen);
    const s1_dry_controls *controls = &frozen;
#endif
    s1_dry_reset(voice);
    if (s1_dry_configure(voice, controls) != S1_DRY_OK ||
        s1_dry_start(voice, pitch_hz(part, key), velocity, 0.0f) != S1_DRY_OK)
        return 0;
#if X0X_SYNTHONE_PATCH
    voice->sub.phase = sub_phase;
#endif
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

/* The source uses two independent floating phasors and a separate
 * 5.3125 ms half-time discontinuity smoother for each wave. The inverse
 * envelope follows smoothing; the third route averages the two envelopes. */
static float lfo_step(s1_trem_lfo *lfo)
{
    float phase=lfo->phase, wave;
    lfo->phase=phase+lfo->increment;
    if (lfo->phase >= 1.f) lfo->phase -= 1.f;
    switch (lfo->shape) {
    case 1: wave=phase > .5f ? 1.f : -1.f; break;
    case 2: wave=(phase-.5f)*2.f; break;
    case 3: wave=(.5f-phase)*2.f; break;
    default: wave=fm_sin_pi((phase > .5f ? phase-1.f : phase)*FM_TWO_PI); break;
    }
    lfo->smooth=0x1.8338p-9f*wave+0x1.fe7cc8p-1f*lfo->smooth;
    return 1.f - (.5f*(1.f-lfo->smooth)*lfo->level);
}
/* The two original modulation destinations share clocks and discontinuity
 * smoothers. The hold clock reads the rising envelope before tremolo applies
 * its independent inverse envelope to the held sample. */
static __attribute__((noinline)) void modulated_crush(s1_dry_part *part, float *left, float *right, unsigned frames)
{
    s1_trem_lfo *a = &part->lfo[0], *b = &part->lfo[1];
    for (unsigned i = 0; i < frames; ++i) {
        float gain1 = lfo_step(a), gain2 = lfo_step(b);
        if (part->crush_next <= part->crush_age) {
            float mod1 = .5f * (1.f + a->smooth) * a->level;
            float mod2 = .5f * (1.f + b->smooth) * b->level;
            float mod = part->crush_route == 1 ? mod1 : part->crush_route == 2 ? mod2 : .5f * (mod1 + mod2);
            float hz = fm_exp2f(part->crush_log_hz + 4.f * mod);
            float interval = hz < 44100.f ? 44100.f / hz : 1.f;
            part->crush_last = left[i];
            part->crush_next += interval;
            part->crush_next -= part->crush_age;
            part->crush_age = 0.f;
        }
        part->crush_age += 1.f;
        float out = part->crush_last;
        if (part->tremolo) out *= part->tremolo == 1 ? gain1 : part->tremolo == 2 ? gain2 : .5f * (gain1 + gain2);
        left[i] = right[i] = out;
    }
}
static void tremolo_mono(s1_dry_part *part, float *left, float *right, unsigned frames)
{
    s1_trem_lfo *a=&part->lfo[0], *b=&part->lfo[1];
    /* The default phases and waves match. Keep one warm clock history until
     * the user separates them, including the pre-edit and silent intervals. */
    int same=a->shape==b->shape && a->increment==b->increment &&
             a->phase==b->phase && a->smooth==b->smooth;
    for (unsigned i=0;i<frames;++i) {
        float gain1=lfo_step(a);
        float gain2=same ? 1.f-(.5f*(1.f-a->smooth)*b->level) : lfo_step(b);
        if (part->tremolo) {
            float gain=part->tremolo == 1 ? gain1 : part->tremolo == 2 ? gain2 : .5f*(gain1+gain2);
            left[i]=right[i]=left[i]*gain;
        }
    }
    if (same) { b->phase=a->phase; b->smooth=a->smooth; }
}

#include "s1_phaser.inc"
#include "s1_master.inc"

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
    phaser_stereo(&part->phaser,left,right,frames);
    master_stereo(&part->master,left,right,frames);
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

    if (part->crush_route && part->crush_interval > 1.f) {
        modulated_crush(part, left, right, frames);
    } else {
        /* Even while the rate is above 44.1 kHz the saved last sample and
         * clock must remain valid for a subsequent live control change. */
        if (part->crush_interval == 1.f && part->crush_age == 1.f && part->crush_next == 1.f) {
            part->crush_last = left[frames-1];
        } else {
            for (unsigned i=0; i<frames; ++i) {
                if (part->crush_next <= part->crush_age) {
                    part->crush_last = left[i];
                    part->crush_next += part->crush_interval;
                    part->crush_next -= part->crush_age;
                    part->crush_age = 0.f;
                } else {
                    left[i] = right[i] = part->crush_last;
                }
                part->crush_age += 1.f;
            }
        }
        tremolo_mono(part, left, right, frames);
    }
    pan_stereo(part, left, right, frames);
}
