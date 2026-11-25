/* SPDX-License-Identifier: GPL-3.0-only */
#pragma once
#include <stdint.h>
#include "../dsp/x0x_param.h"

/* Ordinary SMK-37 Elite and Pro builds enable this dry INIT patch interface.
 * X0X_SYNTHONE_PATCH=0 omits these 13 fields. The separate rate reducer
 * uses the ordinary bass engine controls in both configurations.
 * Stable Synth One factory IDs; performance wheels belong to a separate API.
 * Storage slots are in descriptor order and must never be reordered. Existing
 * OS11 INIT and historical eight-control cells use no bits in this range. */
enum { S1_PATCH_FIRST_BYTE = 8, S1_PATCH_COUNT = 13, S1_PATCH_STEPS = 127 };
/* Panel link is a project option outside the upstream factory ID space. */
enum { S1_LINK_BYTE = 6, S1_LINK_ID = 255, S1_FILTER_FIRST = 5, S1_AMP_FIRST = 9 };
#define S1_LINK_MASK (UINT32_C(1) << 21)
#define X0X_S1_EXTRA (X0X_S1C && X0X_SYNTHONE_PATCH)
#if X0X_S1_EXTRA
/* Existing project sound rows 0/1 own the base cells, 2/3 ARP, 4/5 SJ6. */
enum {
    S1_EXTRA_ROW = 6, S1_EXTRA_FIRST = S1_PATCH_COUNT + 1,
    S1_OSC1_MORPH = S1_EXTRA_FIRST, S1_OSC2_MORPH, S1_OSC2_LEVEL,
    S1_SUB_LEVEL, S1_SUB_OCTAVE, S1_SUB_WAVE, S1_FM_LEVEL, S1_FM_INDEX, S1_NOISE_LEVEL,
    S1_FILTER_TYPE, S1_ADSR_TRACK,
    S1_OSC_EXTRA_BYTES = 2 + S1_FILTER_TYPE - S1_EXTRA_FIRST,
    S1_LFO1_RATE = S1_ADSR_TRACK + 1, S1_LFO2_RATE, S1_LFO1_SHAPE, S1_LFO2_SHAPE,
    S1_LFO_SYNC, S1_DELAY_MIX, S1_REVERB_MIX, S1_REVERB_HP, S1_WIDEN, S1_MONO_LEGATO,
    S1_WET_FIRST = S1_DELAY_MIX, S1_WET_COUNT = 5,
    S1_LFO_FIRST = S1_LFO1_RATE, S1_LFO_COUNT = 5,
    S1_EXTRA_COUNT = S1_ADSR_TRACK + 1 - S1_EXTRA_FIRST,
    S1_FILTER_EXTRA_BYTES = 2 + S1_EXTRA_COUNT,
    S1_EXTRA_BYTES = S1_FILTER_EXTRA_BYTES + 2,
    S1_LFO_DIVISIONS = 19, S1_LFO_CODE_SYNC = 129,
    S1_LFO_RATE_CODES = S1_LFO_CODE_SYNC + S1_LFO_DIVISIONS,
    S1_PART_LEVEL = 40, S1_LFO1_DEPTH, S1_LFO2_DEPTH, S1_CUTOFF_LFO, S1_NOISE_LFO, S1_OSC_MIX_LFO, S1_RESONANCE_LFO, S1_FM_LFO, S1_AMP_DECAY_LFO, S1_OSC1_SEMI, S1_OSC2_SEMI, S1_OSC2_DETUNE, S1_VOICE_MODE, S1_MONO_GLIDE, S1_DELAY_ON, S1_REVERB_ON, S1_DETUNE_LFO, S1_PITCH_LFO, S1_TREMOLO_LFO, S1_REVERB_MIX_LFO = S1_TREMOLO_LFO+3,
    S1_BASE_EDIT_FIRST = S1_PART_LEVEL, S1_BASE_EDIT_COUNT = 3, S1_BASE_EDIT_BYTE = 3
};
#if defined(X0X_S1_BEND_RANGE_PROBE) && X0X_S1_BEND_RANGE_PROBE
enum { S1_BEND_LO = S1_TREMOLO_LFO+1, S1_BEND_HI };
static inline int s1_bend_slot(unsigned slot) { return slot-S1_BEND_LO < 2u; }
#endif
/* Formerly invalid q7 top bits. Code zero keeps the exact program;
 * codes 1..37 are the original integer stops -12..+24. */
enum { S1_COARSE_FIRST = S1_OSC1_SEMI, S1_COARSE_STEPS = 36, S1_COARSE_BITS = 6 };
static inline int s1_coarse_slot(unsigned slot) { return slot-S1_COARSE_FIRST < 2u; }
static inline unsigned s1_coarse_code(const uint8_t cell[32], unsigned osc)
{
    unsigned code = 0;
    for (unsigned i=0;i<S1_COARSE_BITS;++i)
        code |= (unsigned)(cell[S1_PATCH_FIRST_BYTE + osc*S1_COARSE_BITS + i] >> 7) << i;
    return code;
}
/* Sound byte 31 bit 0 keeps the existing crush marker. Its previously
 * invalid high bits hold absent or one of the 81 manual -4..+4 detents. */
enum { S1_DETUNE_STEPS = 80 };
static inline unsigned s1_detune_code(const uint8_t cell[32]) { return cell[31] >> 1; }
/* The prior typed link domain was exactly 0/1. Preserve bit 0 and
 * allocate the old-invalid high bits to independent 3 x 42 digits. */
enum { S1_VOICE_MARK_BYTE = 20, S1_GLIDE_STEPS = 40, S1_GLIDE_RADIX = S1_GLIDE_STEPS+2, S1_VOICE_CODES = 3*S1_GLIDE_RADIX };
static inline unsigned s1_voice_code(const uint8_t cell[32]) { return cell[S1_LINK_BYTE] >> 1; }
#define S1_MOD_EXT_MARK (UINT32_C(1) << 23)
/* Radix five stores independent absent/explicit OFF/LFO1/LFO2/BOTH digits.
 * FM-only codes 0..4 are identical to the preceding typed binding. */
enum { S1_MOD_EXT_BYTE = 30, S1_MOD_EXT_FIRST = S1_FM_LFO, S1_MOD_EXT_COUNT = 2,
       S1_MOD_RADIX = 5, S1_ROUTE_EXT_CODES = S1_MOD_RADIX * S1_MOD_RADIX,
       S1_FX_GATE_RADIX = 3, S1_MOD_EXT_CODES = S1_ROUTE_EXT_CODES * 9 };
/* Typed q7 MSBs of three owned base editor bytes; absent/explicit 0..3.
 * S1_MOD_EXT_MARK must also be present, preventing legacy slot-2 adoption. */
static inline unsigned s1_detune_route_code(const uint8_t cell[32])
{
    const uint8_t *p = cell+S1_BASE_EDIT_BYTE;
    return (p[0] >> 7) | ((p[1] >> 7) << 1) | ((p[2] >> 7) << 2);
}
/* Only formerly invalid bits 3..5 of the historic high ordinal byte.
 * Bit7 is independently reserved and not claimed by this route. */
enum { S1_PROGRAM_BYTE_MASK = 0xc7 };
static inline unsigned s1_pitch_route_code(const uint8_t cell[32])
{ return (cell[21] & 0x38u) >> 3; }
/* Formerly invalid high bits [0..2], with typed mod witness. Lower
 * legacy performance/crush words are unchanged. */
static inline unsigned s1_tremolo_route_code(const uint8_t cell[32])
{ return (cell[0] >> 7) | ((cell[1] >> 7) << 1) | ((cell[2] >> 7) << 2); }
static inline int s1_fx_gate_slot(unsigned slot) { return slot-S1_DELAY_ON < 2u; }
static inline int s1_mod_ext_slot(unsigned slot) { return slot - S1_MOD_EXT_FIRST < S1_MOD_EXT_COUNT; }
static inline unsigned s1_mod_ext_divisor(unsigned slot) { return slot == S1_FM_LFO ? 1u : S1_MOD_RADIX; }
#define S1_EXTRA_MARK (UINT32_C(1) << 22)
#define S1_BASE_EDIT_MASK (((UINT32_C(1) << S1_BASE_EDIT_COUNT) - 1u) << S1_BASE_EDIT_BYTE)
/* Synth One route group: legacy Noise/Osc lanes keep their exact bytes.
 * Formerly-invalid bit-7 range encodes an explicit third route as a radix-5 ordinal. */
enum { S1_ROUTE_FIRST = S1_NOISE_LFO, S1_ROUTE_COUNT = 3, S1_ROUTE_SHIFT = 24 };
#define S1_ROUTE_MASK (UINT32_C(255) << S1_ROUTE_SHIFT)
static inline int s1_route_decode(unsigned word, uint8_t code[S1_ROUTE_COUNT])
{
    if (!(word & 128u)) {
        code[0] = (uint8_t)(word & 7u); code[1] = (uint8_t)((word >> 3) & 7u); code[2] = 0;
        return word < 64u && code[0] <= 4u && code[1] <= 4u;
    }
    word -= 128u;
    if (word >= 100u) return 0;
    code[2] = (uint8_t)(1u + word / 25u);
    word %= 25u;
    code[0] = (uint8_t)(word / 5u); code[1] = (uint8_t)(word % 5u);
    return 1;
}
static inline unsigned s1_route_encode(const uint8_t code[S1_ROUTE_COUNT])
{
    return code[2] ? 128u + (code[2]-1u)*25u + code[0]*5u + code[1] :
           code[0] | (unsigned)code[1] << 3;
}
static inline int s1_base_route_slot(unsigned slot)
{ return slot - S1_ROUTE_FIRST < S1_ROUTE_COUNT; }
static inline int s1_base_edit_slot(unsigned slot)
{ return slot == S1_TREMOLO_LFO || slot == S1_PITCH_LFO || slot == S1_DETUNE_LFO || s1_fx_gate_slot(slot) || slot - S1_VOICE_MODE < 2u || slot == S1_OSC2_DETUNE || s1_coarse_slot(slot) || s1_mod_ext_slot(slot) || s1_base_route_slot(slot) || slot - S1_BASE_EDIT_FIRST < S1_BASE_EDIT_COUNT; }
float s1c_morph_preview(float position, uint32_t phase);
static inline unsigned s1_extra_bits(const uint8_t *data)
{ return (unsigned)data[0] | (((unsigned)data[1] & 7u) << 8); }
/* Used only for a validated Synth One owner, never to claim opaque rows. */
static inline int s1_extra_packed(const uint8_t *data)
{ return !(data[0] | data[1]); }
static inline unsigned s1_extra_size(const uint8_t *data)
{
    if (data[1] >> 3 || s1_extra_packed(data)) return S1_EXTRA_BYTES;
    return s1_extra_bits(data) >> (S1_FILTER_TYPE - S1_EXTRA_FIRST) ?
           S1_FILTER_EXTRA_BYTES : S1_OSC_EXTRA_BYTES;
}
static inline int s1_lfo_rate_slot(unsigned slot)
{ return slot - S1_LFO_FIRST < 2u; }
/* Zero means unset in each phase. Panel and wet digits hold position + 1.
 * LFO rate digits keep their existing absolute/free-or-division code. */
typedef struct {
    uint8_t panel[S1_EXTRA_COUNT], lfo[S1_LFO_COUNT], wet[S1_WET_COUNT], cutoff_lfo, reverb_mix_lfo;
} s1_extra_state;
int s1_extra_read(const uint8_t *data, s1_extra_state *state);
#else
#define S1_EXTRA_MARK 0u
#define S1_BASE_EDIT_MASK 0u
#define S1_ROUTE_MASK 0u
#define S1_MOD_EXT_MARK 0u
#endif
static inline unsigned s1_patch_steps(unsigned slot)
{
#if X0X_S1_EXTRA
#if defined(X0X_S1_BEND_RANGE_PROBE) && X0X_S1_BEND_RANGE_PROBE
    if (s1_bend_slot(slot)) return 24u;
#endif
    if (slot == S1_VOICE_MODE || s1_fx_gate_slot(slot)) return 1u;
    if (slot == S1_MONO_GLIDE) return S1_GLIDE_STEPS;
    if (slot == S1_OSC2_DETUNE) return S1_DETUNE_STEPS;
    if (s1_coarse_slot(slot)) return S1_COARSE_STEPS;
    if (slot == S1_SUB_OCTAVE || slot == S1_SUB_WAVE) return 1;
    if (slot == S1_FILTER_TYPE) return 2;
    if (slot == S1_LFO_SYNC || slot == S1_WIDEN || slot == S1_MONO_LEGATO) return 1;
    if (slot - S1_LFO1_SHAPE < 2u || slot == S1_CUTOFF_LFO || slot == S1_DETUNE_LFO || slot == S1_PITCH_LFO || slot == S1_TREMOLO_LFO || slot == S1_REVERB_MIX_LFO || s1_base_route_slot(slot) || s1_mod_ext_slot(slot)) return 3;
#endif
    return slot == S1_PATCH_COUNT ? 1u : S1_PATCH_STEPS;
}
#define S1_PATCH_MASK (((UINT32_C(1) << S1_PATCH_COUNT) - 1) << S1_PATCH_FIRST_BYTE)

typedef struct {
    const char *label;
    float minimum, maximum;
    uint8_t factory_id, unit;
} s1_patch_descriptor;

typedef struct {
    uint32_t revision;
    unsigned preset;
    float physical; /* applied DSP target; the exact INIT value before an edit */
    uint8_t position;      /* nearest editable position */
    uint8_t note_locked;   /* temporary sequencer patch owns the heard value */
    uint8_t max_position;  /* original division stops while a rate is synced */
} s1_patch_value;

/* Whole audible patch from a single render boundary. Values are in the same
 * order as s1_patch_at(), including the untouched factory precision. */
typedef struct {
    uint32_t revision;
    unsigned preset;
    uint8_t note_locked;
    uint8_t filter_link;
    float physical[S1_PATCH_COUNT];
} s1_patch_snapshot;

const s1_patch_descriptor *s1_patch_at(unsigned index);
float s1_patch_physical(const s1_patch_descriptor *, unsigned position);
/* Value for the firmware's unit_value formatter. Uses real seconds and Hz. */
static inline float s1_patch_display(const s1_patch_descriptor *d, float physical)
{ return d->unit == X0X_U_PCT ? physical * 100.f : physical; }
