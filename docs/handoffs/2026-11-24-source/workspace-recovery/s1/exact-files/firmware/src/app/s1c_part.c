/* SPDX-License-Identifier: GPL-3.0-only */
#include "s1c_part.h"
#include "../dsp/s1c/s1c_front.h"
#include "../dsp/s1pack/s1pack.h"
#include "../dsp/fastmath.h"
#include "../crc32_nibble.h"
#if X0X_SYNTHONE_PATCH
#include "original_sound.h"
#endif
#include <stddef.h>
#include <math.h>
#include <string.h>

#define S1C_SR 44100.0f
/* Synth One factory parameter ids (s1c uses the same numbering). */
enum { P_CUT = 16, P_MONO = 20, P_GLIDE = 21, P_DET2 = 30, P_DETM = 31, P_BCSR = 34, P_PAN_RATE = 36, P_DELAY_TIME = 43, P_CUTOFF_LFO = 48, P_RESONANCE_LFO = 49, P_OSC_MIX_LFO = 50, P_REVERB_MIX_LFO = 51, P_AMP_DECAY_LFO = 52, P_NOISE_LFO = 53, P_FM_LFO = 54, P_DETUNE_LFO = 55, P_PITCH_LFO = 57, P_TREMOLO_LFO = 59, P_TEMPO_SYNC = 139 };

#if !X0X_SYNTH_CONTROLS_INIT_ONLY
static const uint8_t *s1_part_pack;
static uint16_t s1_part_pack_count;
#endif

_Static_assert(S1C_RVB_BYTES % 4u == 0u && S1C_RVB_BYTES + 1024u <= S1C_ARENA_BYTES,
               "reverb lines leave at least 1 KiB of delay pool in the Chicago FX2 arena");

/* Effects need the same behaviour, not an exact match: one fixed reverb fit for every program
 * (no per-program table). Delay lines are half floats. */
static void fit_apply(s1c *e)
{
    e->rv.fit[0] = 0.5f; e->rv.fit[1] = 1.5f; e->rv.fit[2] = 1.0f; e->dfmt = 0;
}

#if !X0X_SYNTH_CONTROLS_INIT_ONLY
static void emit(void *context, unsigned id, float physical) { (void)s1c_front_set((s1c *)context, id, physical); }
#endif

static void load_init(s1c *e)
{
    s1c_init(e, S1C_SR, NULL);
    for (unsigned i = 0; i < 150u; ++i) (void)s1c_front_set(e, i, s1c_front_init(i));
}

#if X0X_SYNTH_CONTROLS_INIT_ONLY
unsigned s1_part_set_pack(const uint8_t *p, uint32_t bytes) { (void)p; (void)bytes; return 0; }
unsigned s1_part_programs(void) { return 0; }
const char *s1_part_program_name(unsigned program) { (void)program; return "INIT"; }
#else
unsigned s1_part_set_pack(const uint8_t *p, uint32_t bytes)
{
    s1_part_pack = NULL;
    s1_part_pack_count = 0;
    if (!p || bytes < S1PACK_HEADER) return 0;
    unsigned n = s1pack_programs(p, bytes);
    if (!n || x0x_crc32(p + S1PACK_HEADER, s1pack_total(p) - S1PACK_HEADER) != s1pack_crc(p)) return 0;
    s1_part_pack = p;
    return s1_part_pack_count = (uint16_t)n;
}

unsigned s1_part_programs(void) { return s1_part_pack_count; }

const char *s1_part_program_name(unsigned program)
{
    static char name[S1PACK_NAME_MAX];
    if (program < s1_part_programs() && s1pack_name(s1_part_pack, program, name) && name[0]) return name;
    unsigned n = program + 1u, i = 0;
    char digits[6];
    name[i++] = 'S'; name[i++] = '1'; name[i++] = ' ';
    unsigned d = 0;
    do { digits[d++] = (char)('0' + n % 10u); n /= 10u; } while (n && d < sizeof digits);
    while (d) name[i++] = digits[--d];
    name[i] = 0;
    return name;
}
#endif

int s1_part_program(s1_dry_part *part, unsigned program)
{
    s1c *e = &part->e;
    int ok = 1;
    if (program != S1_PART_INIT && program >= s1_part_programs()) { program = S1_PART_INIT; ok = 0; }
    load_init(e);
#if !X0X_SYNTH_CONTROLS_INIT_ONLY
    if (program != S1_PART_INIT) {
        char name[S1PACK_NAME_MAX];
        if (s1pack_read(s1_part_pack, program, name, emit, e) < 0) { load_init(e); program = S1_PART_INIT; ok = 0; }
    }
#endif
#if X0X_S1_EXTRA
    part->extra_live_mask = 0;
    part->lfo_live_mask = 0;
    part->clock_factory_pan = e->t[P_PAN_RATE];
    part->clock_factory_delay = e->t[P_DELAY_TIME];
    part->reverb_lfo_factory = e->t[P_REVERB_MIX_LFO];
    part->cutoff_lfo_factory = e->t[P_CUTOFF_LFO];
    part->fx_gate_factory[0] = e->t[S1C_P_DLON];
    part->fx_gate_factory[1] = e->t[S1C_P_RVON];
    part->tremolo_lfo_factory = e->t[P_TREMOLO_LFO];
    part->pitch_lfo_factory = e->t[P_PITCH_LFO];
    part->detune_lfo_factory = e->t[P_DETUNE_LFO];
    part->detune_factory = e->t[P_DET2];
    part->voice_factory[0] = e->t[P_MONO];
    part->voice_factory[1] = e->t[P_GLIDE];
    for (unsigned i=0;i<2;++i) part->coarse_factory[i] = e->t[3u+i];
    for (unsigned i = 0; i < S1_MOD_EXT_COUNT; ++i)
        part->mod_ext_factory[i] = e->t[s1_patch_at(S1_MOD_EXT_FIRST+i)->factory_id];
    for (unsigned i = 0; i < S1_ROUTE_COUNT; ++i)
        part->route_factory[i] = e->t[s1_patch_at(S1_ROUTE_FIRST+i)->factory_id];
    for (unsigned i = 0; i < S1_BASE_EDIT_COUNT; ++i)
        part->base_factory[i] = e->t[s1_patch_at(S1_BASE_EDIT_FIRST+i)->factory_id];
    for (unsigned i = 0; i < S1_EXTRA_COUNT; ++i)
        part->extra_factory[i] = e->t[s1_patch_at(S1_EXTRA_FIRST+i)->factory_id];
    for (unsigned i = 0; i < S1_LFO_COUNT; ++i)
        part->lfo_factory[i] = e->t[s1_patch_at(S1_LFO_FIRST+i)->factory_id];
    for (unsigned i = 0; i < S1_WET_COUNT; ++i)
        part->wet_factory[i] = e->t[s1_patch_at(S1_WET_FIRST+i)->factory_id];
#endif
    fit_apply(e);
    /* The engine clears its voices when mono changes; a program load is not a change. */
    e->mono_prev = (uint8_t)e->t[P_MONO];
    part->program = (uint16_t)program;
#if X0X_SYNTHONE_PATCH
    part->filter_link = 0;
#endif
    part->detm = e->t[P_DETM];
#if defined(X0X_S1_BEND_RANGE_PROBE) && X0X_S1_BEND_RANGE_PROBE
    part->bend_typed = 0;
    part->bend_min = -2.f;
    part->bend_max = 2.f;
    part->bend_raw = 8192;
#endif
    part->held = 0;
    if (part->arena)
        s1c_attach(e, part->arena, part->wbuf, part->arena + S1C_RVB_BYTES, (int)(S1C_ARENA_BYTES - S1C_RVB_BYTES));
    return ok;
}

void s1_part_init(s1_dry_part *part)
{
    memset(part, 0, sizeof *part);
    (void)s1_part_program(part, S1_PART_INIT);
}

void s1_part_bind(s1_dry_part *part, void *arena)
{
    part->arena = (uint8_t *)arena;
    if (arena)
        s1c_attach(&part->e, part->arena, part->wbuf, part->arena + S1C_RVB_BYTES, (int)(S1C_ARENA_BYTES - S1C_RVB_BYTES));
}

void s1_part_all_off(s1_dry_part *part)
{
    s1c_stop_all(&part->e);
    part->held = 0;
    part->damper = 0;
    part->e.t[P_DETM] = part->detm;
#if defined(X0X_S1_BEND_RANGE_PROBE) && X0X_S1_BEND_RANGE_PROBE
    part->bend_raw = 8192;
#endif
}

#if X0X_SYNTHONE_PATCH
/* Linear 7-bit edits across the original physical domains. The INIT preset
 * retains its exact unquantized values. Storage order must never change. */
static const s1_patch_descriptor patch_parameters[S1_PATCH_COUNT] = {
    {"OSC BALANCE", 0, 1, 2, X0X_U_PCT},
    {"OSC 1 LEVEL", 0, 1, 5, X0X_U_PCT},
    {"CUTOFF", 64, 22050, 16, X0X_U_HZ},
    {"RESONANCE", 0, .98f, 17, X0X_U_PCT},
    {"FILTER ENV", 0, 1.2f, 19, X0X_U_PCT},
    {"FILTER ATTACK", .0005f, 2, 22, X0X_U_S},
    {"FILTER DECAY", .005f, 2, 23, X0X_U_S},
    {"FILTER SUSTAIN", 0, 1, 24, X0X_U_PCT},
    {"FILTER RELEASE", 0, 2, 25, X0X_U_S},
    {"AMP ATTACK", .0005f, 2, 26, X0X_U_S},
    {"AMP DECAY", .005f, 2, 27, X0X_U_S},
    {"AMP SUSTAIN", 0, 1, 28, X0X_U_PCT},
    {"AMP RELEASE", .004f, 2, 29, X0X_U_S},
};

#if X0X_S1_EXTRA
static const s1_patch_descriptor extra_parameters[] = {
    {"OSC 1 MORPH", 0, 1, 0, X0X_U_PCT},
    {"OSC 2 MORPH", 0, 1, 1, X0X_U_PCT},
    {"OSC 2 LEVEL", 0, 1, 6, X0X_U_PCT},
    {"SUB LEVEL", 0, 1, 7, X0X_U_PCT},
    {"SUB OCTAVE", 0, 1, 8, X0X_U_NONE},
    {"SUB WAVE", 0, 1, 9, X0X_U_NONE},
    {"FM LEVEL", 0, 1, 10, X0X_U_PCT},
    {"FM INDEX", 0, 15, 11, X0X_U_NONE},
    {"NOISE LEVEL", 0, .25f, 12, X0X_U_PCT},
    {"FILTER TYPE", 0, 2, 115, X0X_U_NONE},
    {"ADSR PITCH TRACK", 0, 1, 149, X0X_U_PCT},
    {"LFO 1 RATE", 0x1.111112p-11f, 80, 15, X0X_U_HZ},
    {"LFO 2 RATE", 0x1.111112p-11f, 80, 47, X0X_U_HZ},
    {"LFO 1 SHAPE", 0, 3, 13, X0X_U_NONE},
    {"LFO 2 SHAPE", 0, 3, 45, X0X_U_NONE},
    {"SYNC TO ARP", 0, 1, P_TEMPO_SYNC, X0X_U_NONE},
    {"DELAY MIX", 0, 1, S1C_P_DLMIX, X0X_U_PCT},
    {"REVERB MIX", 0, 1, S1C_P_RVMIX, X0X_U_PCT},
    {"REVERB HP", 80, 900, S1C_P_RVHP, X0X_U_HZ},
    {"WIDEN", 0, 1, S1C_P_WIDEN, X0X_U_NONE},
    {"MONO LEGATO", 0, 1, 120, X0X_U_NONE},
};
static const s1_patch_descriptor base_editor[S1_BASE_EDIT_COUNT] = {
    {"PART LEVEL", 0, 2, S1C_P_MVOL, X0X_U_PCT},
    {"LFO 1 DEPTH", 0, 1, 14, X0X_U_PCT},
    {"LFO 2 DEPTH", 0, 1, 46, X0X_U_PCT},
};
static const s1_patch_descriptor routes[S1_ROUTE_COUNT] = {
    {"NOISE LFO", 0, 3, P_NOISE_LFO, X0X_U_NONE},
    {"OSC MIX LFO", 0, 3, P_OSC_MIX_LFO, X0X_U_NONE},
    {"RES LFO", 0, 3, P_RESONANCE_LFO, X0X_U_NONE},
};
static const s1_patch_descriptor mod_ext[S1_MOD_EXT_COUNT] = {
    {"FM LFO", 0, 3, P_FM_LFO, X0X_U_NONE},
    {"AMP DEC LFO", 0, 3, P_AMP_DECAY_LFO, X0X_U_NONE},
};
static const s1_patch_descriptor coarse[2] = {
    {"OSC 1 SEMI", -12, 24, 3, X0X_U_ST},
    {"OSC 2 SEMI", -12, 24, 4, X0X_U_ST},
};
static const s1_patch_descriptor voice_mode[2] = {
    {"VOICE MODE", 0, 1, P_MONO, X0X_U_NONE},
    {"MONO GLIDE", 0, .2f, P_GLIDE, X0X_U_S},
};
static const s1_patch_descriptor detune = {"OSC 2 DETUNE", -4, 4, P_DET2, X0X_U_NONE};
static const s1_patch_descriptor fx_gate[2] = {
    {"DELAY", 0, 1, S1C_P_DLON, X0X_U_NONE},
    {"REVERB", 0, 1, S1C_P_RVON, X0X_U_NONE},
};
static const s1_patch_descriptor detune_lfo = {"DETUNE LFO", 0, 3, P_DETUNE_LFO, X0X_U_NONE};
static const s1_patch_descriptor pitch_lfo = {"PITCH LFO", 0, 3, P_PITCH_LFO, X0X_U_NONE};
static const s1_patch_descriptor tremolo_lfo = {"TREMOLO LFO", 0, 3, P_TREMOLO_LFO, X0X_U_NONE};
static const s1_patch_descriptor reverb_lfo = {"REVERB MIX LFO", 0, 3, P_REVERB_MIX_LFO, X0X_U_NONE};
static const s1_patch_descriptor cutoff_lfo = {"CUTOFF LFO", 0, 3, P_CUTOFF_LFO, X0X_U_NONE};
enum { S1_CATALOG_EXTRA_COUNT = sizeof extra_parameters / sizeof extra_parameters[0] };
_Static_assert(S1_CATALOG_EXTRA_COUNT == S1_WET_FIRST - S1_EXTRA_FIRST + S1_WET_COUNT,
               "Synth One editable catalog follows its persisted slot order");
#endif

const s1_patch_descriptor *s1_patch_at(unsigned slot)
{
    if (slot < S1_PATCH_COUNT) return &patch_parameters[slot];
#if X0X_S1_EXTRA
#if defined(X0X_S1_BEND_RANGE_PROBE) && X0X_S1_BEND_RANGE_PROBE
    static const s1_patch_descriptor bend_range[] = {
        {"PB DOWN", -24.f, 0.f, 141, X0X_U_ST},
        {"PB UP", 0.f, 24.f, 142, X0X_U_ST},
    };
    if (s1_bend_slot(slot)) return &bend_range[slot-S1_BEND_LO];
#endif
    if (slot == S1_REVERB_MIX_LFO) return &reverb_lfo;
    if (slot == S1_CUTOFF_LFO) return &cutoff_lfo;
    if (slot == S1_TREMOLO_LFO) return &tremolo_lfo;
    if (slot == S1_PITCH_LFO) return &pitch_lfo;
    if (slot == S1_DETUNE_LFO) return &detune_lfo;
    if (s1_fx_gate_slot(slot)) return &fx_gate[slot-S1_DELAY_ON];
    if (slot == S1_OSC2_DETUNE) return &detune;
    if (slot-S1_VOICE_MODE < 2u) return &voice_mode[slot-S1_VOICE_MODE];
    if (s1_coarse_slot(slot)) return &coarse[slot-S1_COARSE_FIRST];
    if (s1_mod_ext_slot(slot)) return &mod_ext[slot-S1_MOD_EXT_FIRST];
    if (s1_base_route_slot(slot)) return &routes[slot-S1_ROUTE_FIRST];
    if (s1_base_edit_slot(slot)) return &base_editor[slot-S1_BASE_EDIT_FIRST];
    if (slot - S1_EXTRA_FIRST < S1_CATALOG_EXTRA_COUNT) return &extra_parameters[slot-S1_EXTRA_FIRST];
#endif
    return NULL;
}

int s1_patch_slot(unsigned id)
{
    for (unsigned i = 0; i < S1_PATCH_COUNT; ++i)
        if (patch_parameters[i].factory_id == id) return (int)i;
#if X0X_S1_EXTRA
#if defined(X0X_S1_BEND_RANGE_PROBE) && X0X_S1_BEND_RANGE_PROBE
    if (id-141u < 2u) return (int)(S1_BEND_LO+id-141u);
#endif
    if (id == P_REVERB_MIX_LFO) return S1_REVERB_MIX_LFO;
    if (id == P_CUTOFF_LFO) return S1_CUTOFF_LFO;
    if (id == P_TREMOLO_LFO) return S1_TREMOLO_LFO;
    if (id == P_PITCH_LFO) return S1_PITCH_LFO;
    if (id == P_DETUNE_LFO) return S1_DETUNE_LFO;
    if (id == S1C_P_DLON || id == S1C_P_RVON) return id == S1C_P_DLON ? S1_DELAY_ON : S1_REVERB_ON;
    if (id == P_DET2) return S1_OSC2_DETUNE;
    if (id-P_MONO < 2u) return (int)(S1_VOICE_MODE+id-P_MONO);
    if (id-3u < 2u) return (int)(S1_COARSE_FIRST+id-3u);
    for (unsigned i = 0; i < S1_MOD_EXT_COUNT; ++i)
        if (id == mod_ext[i].factory_id) return (int)(S1_MOD_EXT_FIRST+i);
    for (unsigned i = 0; i < S1_ROUTE_COUNT; ++i)
        if (id == routes[i].factory_id) return (int)(S1_ROUTE_FIRST+i);
    for (unsigned i = 0; i < S1_BASE_EDIT_COUNT; ++i)
        if (id == base_editor[i].factory_id) return (int)(S1_BASE_EDIT_FIRST + i);
    for (unsigned i = 0; i < S1_CATALOG_EXTRA_COUNT; ++i)
        if (extra_parameters[i].factory_id == id) return (int)(S1_EXTRA_FIRST + i);
#endif
    return -1;
}

float s1_patch_physical(const s1_patch_descriptor *d, unsigned position)
{
    if (position >= S1_PATCH_STEPS) return d->maximum;
    return d->minimum + (d->maximum - d->minimum) * ((float)position / S1_PATCH_STEPS);
}

float s1_part_patch_value(const s1_dry_part *part, unsigned slot)
{
#if X0X_S1_EXTRA && defined(X0X_S1_BEND_RANGE_PROBE) && X0X_S1_BEND_RANGE_PROBE
    if (s1_bend_slot(slot)) return slot == S1_BEND_LO ? part->bend_min : part->bend_max;
#endif
    if (slot == S1_PATCH_COUNT) return part->filter_link;
    if (part->filter_link && slot >= S1_FILTER_FIRST && slot < S1_AMP_FIRST)
        return part->filter_unlinked[slot - S1_FILTER_FIRST];
    const s1_patch_descriptor *d = s1_patch_at(slot);
    return d ? part->e.t[d->factory_id] : 0.0f;
}

#if X0X_S1_EXTRA
/* Called only at the existing patch/lock audio boundary. Restore immutable
 * program words directly before applying an authored clock as one tuple. */
static void s1_part_clock(s1_dry_part *part, unsigned code)
{
    s1c *e = &part->e;
#if defined(X0X_S1_AUTOPAN_PROBE) && X0X_S1_AUTOPAN_PROBE
    const float pan_hz=e->t[P_PAN_RATE];
#endif
    s1c_set(e, P_TEMPO_SYNC, part->lfo_factory[S1_LFO_SYNC - S1_LFO_FIRST]);
    s1c_set(e, extra_parameters[S1_LFO1_RATE - S1_EXTRA_FIRST].factory_id, part->lfo_factory[0]);
    s1c_set(e, extra_parameters[S1_LFO2_RATE - S1_EXTRA_FIRST].factory_id, part->lfo_factory[1]);
#if defined(X0X_S1_AUTOPAN_PROBE) && X0X_S1_AUTOPAN_PROBE
    if(!e->pan_authored)
#endif
    s1c_set(e, P_PAN_RATE, part->clock_factory_pan);
    s1c_set(e, P_DELAY_TIME, part->clock_factory_delay);
    if (code) {
        (void)s1c_front_set(e, P_TEMPO_SYNC, 1.f);
        if (code == 1u) (void)s1c_front_set(e, P_TEMPO_SYNC, 0.f);
    }
#if defined(X0X_S1_AUTOPAN_PROBE) && X0X_S1_AUTOPAN_PROBE
    if(e->pan_authored)(void)s1c_front_set(e,P_PAN_RATE,pan_hz);
#endif
}

void s1_part_apply_extra(s1_dry_part *part, const uint8_t *extra)
{
    s1_extra_state state;
    if (!s1_extra_read(extra, &state)) return;
    s1c_set(&part->e, P_REVERB_MIX_LFO, state.reverb_mix_lfo ? (float)(state.reverb_mix_lfo - 1u) : part->reverb_lfo_factory);
    s1c_set(&part->e, P_CUTOFF_LFO, state.cutoff_lfo ? (float)(state.cutoff_lfo - 1u) : part->cutoff_lfo_factory);
    unsigned next = 0;
    for (unsigned i = 0; i < S1_EXTRA_COUNT; ++i) {
        unsigned digit = state.panel[i], bit = 1u << i;
        if (!(digit || (part->extra_live_mask & bit))) continue;
        const s1_patch_descriptor *d = &extra_parameters[i];
        float value = part->extra_factory[i];
        if (digit) {
            next |= bit;
            unsigned position = digit - 1u;
            value = s1_patch_steps(S1_EXTRA_FIRST+i) < S1_PATCH_STEPS ? (float)position :
                    s1_patch_physical(d, position);
        }
#if S1C_KR > 1
        /* A held mode switch needs the new coefficients on its first sample. */
        if (i == S1_FILTER_TYPE - S1_EXTRA_FIRST && value != part->e.t[d->factory_id])
            for (unsigned v = 0; v < S1C_VOICES; ++v) part->e.v[v].kc = 0;
#endif
        (void)s1c_front_set(&part->e, d->factory_id, value);
    }
    for (unsigned i = 0; i < S1_WET_COUNT; ++i) {
        unsigned digit = state.wet[i], bit = 1u << (S1_EXTRA_COUNT+i);
        if (!(digit || (part->extra_live_mask & bit))) continue;
        const s1_patch_descriptor *d = &extra_parameters[S1_WET_FIRST - S1_EXTRA_FIRST + i];
        float value = part->wet_factory[i];
        if (digit) value = s1_patch_steps(S1_WET_FIRST+i) == 1u ?
                           (float)(digit-1u) : s1_patch_physical(d, digit-1u);
        s1c_set(&part->e, d->factory_id, value);
        if (digit) next |= bit;
    }
    part->extra_live_mask = (uint16_t)next;
    const uint8_t *values = state.lfo;
    if (!(part->lfo_live_mask | values[0] | values[1] | values[2] | values[3] | values[4])) return;
    const unsigned clock_index = S1_LFO_SYNC - S1_LFO_FIRST;
    const unsigned clock_bit = 1u << clock_index;
    const unsigned clock = values[clock_index];
    int clock_dirty = clock || (part->lfo_live_mask & clock_bit);
    if (clock_dirty) s1_part_clock(part, clock);
    unsigned live = clock ? clock_bit : 0;
    for (unsigned i = 0; i < clock_index; ++i) {
        unsigned id = extra_parameters[S1_LFO_FIRST - S1_EXTRA_FIRST + i].factory_id;
        if (values[i]) {
            if (i < 2) s1c_front_lfo_apply(&part->e, id, values[i]);
            else s1c_set(&part->e, id, (float)(values[i] - 1u));
            live |= 1u << i;
        } else if ((!clock_dirty || i >= 2) && (part->lfo_live_mask & (1u << i))) {
            /* Program targets are already in applied units. Do not re-quantize. */
            s1c_set(&part->e, id, part->lfo_factory[i]);
        }
    }
    part->lfo_live_mask = (uint8_t)live;
}
#endif

void s1_part_apply_cell(s1_dry_part *part, const uint8_t cell[32])
{
    uint32_t mask = original_sound_edit_mask(cell);
#if X0X_S1_EXTRA
    for (unsigned i=0;i<2;++i) {
        unsigned code = s1_coarse_code(cell,i);
        s1c_set(&part->e, 3u+i, code ? (float)((int)code-13) : part->coarse_factory[i]);
    }
    unsigned voice = s1_voice_code(cell);
    unsigned mono = voice / S1_GLIDE_RADIX, glide = voice % S1_GLIDE_RADIX;
    s1c_set(&part->e, P_MONO, mono ? (float)(mono-1u) : part->voice_factory[0]);
    s1c_set(&part->e, P_GLIDE, glide ? (float)(glide-1u) / 200.f : part->voice_factory[1]);
    unsigned detune_code = s1_detune_code(cell);
    s1c_set(&part->e, P_DET2, detune_code ? (float)((int)detune_code-41) / 10.f : part->detune_factory);
    unsigned mod_code = mask & S1_MOD_EXT_MARK ? cell[S1_MOD_EXT_BYTE] : 0u;
    for (unsigned i = 0; i < S1_MOD_EXT_COUNT; ++i) {
        unsigned digit = mod_code % S1_MOD_RADIX;
        s1c_set(&part->e, mod_ext[i].factory_id,
                digit ? (float)(digit - 1u) : part->mod_ext_factory[i]);
        mod_code /= S1_MOD_RADIX;
    }
    unsigned delay = mod_code % S1_FX_GATE_RADIX, reverb = mod_code / S1_FX_GATE_RADIX;
    s1c_set_delay_gate(&part->e, delay ? (float)(delay-1u) : part->fx_gate_factory[0], delay != 0);
    s1c_set(&part->e, S1C_P_RVON, reverb ? (float)(reverb-1u) : part->fx_gate_factory[1]);
    unsigned detune_route = s1_detune_route_code(cell);
    s1c_set(&part->e, P_DETUNE_LFO, detune_route ? (float)(detune_route-1u) : part->detune_lfo_factory);
    unsigned pitch_route = s1_pitch_route_code(cell);
    s1c_set(&part->e, P_PITCH_LFO, pitch_route ? (float)(pitch_route-1u) : part->pitch_lfo_factory);
    unsigned tremolo_route = s1_tremolo_route_code(cell);
    s1c_set(&part->e, P_TREMOLO_LFO, tremolo_route ? (float)(tremolo_route-1u) : part->tremolo_lfo_factory);
    uint8_t codes[S1_ROUTE_COUNT];
    if (s1_route_decode(mask >> S1_ROUTE_SHIFT, codes))
        for (unsigned i = 0; i < S1_ROUTE_COUNT; ++i)
            s1c_set(&part->e, routes[i].factory_id,
                    codes[i] ? (float)(codes[i]-1u) : part->route_factory[i]);
    for (unsigned i = 0; i < S1_BASE_EDIT_COUNT; ++i) {
        const s1_patch_descriptor *d = &base_editor[i];
        const unsigned byte = S1_BASE_EDIT_BYTE + i;
        s1c_set(&part->e, d->factory_id, mask & (UINT32_C(1) << byte) ?
                s1_patch_physical(d, cell[byte] & 127u) : part->base_factory[i]);
    }
#endif
    if (part->filter_link)
        for (unsigned i = 0; i < 4; ++i)
            part->e.t[patch_parameters[S1_FILTER_FIRST + i].factory_id] = part->filter_unlinked[i];
    for (unsigned i = 0; i < S1_PATCH_COUNT; ++i) {
        unsigned byte = S1_PATCH_FIRST_BYTE + i;
        if (mask & (UINT32_C(1) << byte))
            (void)s1c_front_set(&part->e, patch_parameters[i].factory_id, s1_patch_physical(&patch_parameters[i], cell[byte] & 127u));
    }
    part->filter_link = (mask & S1_LINK_MASK) != 0;
    if (part->filter_link)
        for (unsigned i = 0; i < 4; ++i) {
            unsigned filter_id = patch_parameters[S1_FILTER_FIRST + i].factory_id;
            unsigned amp_id = patch_parameters[S1_AMP_FIRST + i].factory_id;
            part->filter_unlinked[i] = part->e.t[filter_id];
            part->e.t[filter_id] = part->e.t[amp_id];
        }
}
#endif

/* AudioKit Manager routes normalized CC1 to the inverted logarithmic
 * 120..7600 scale, times 3. Front end clamps cutoff to 64..22050 Hz. */
#if X0X_SYNTHONE_PATCH && defined(X0X_S1_MOD_RATE_PROBE) && X0X_S1_MOD_RATE_PROBE
int s1_part_mod_route(s1_dry_part *part, unsigned route)
{
    if (route > 2u) return 0;
    part->mod_wheel_route = (uint8_t)route;
    return 1;
}
#endif
void s1_part_mod_wheel(s1_dry_part *part, unsigned cc1)
{
    if (cc1 > 127u) cc1 = 127u;
#if X0X_SYNTHONE_PATCH && defined(X0X_S1_MOD_RATE_PROBE) && X0X_S1_MOD_RATE_PROBE
    if (part->mod_wheel_route) {
        s1c_front_mod_rate(&part->e, part->mod_wheel_route == 1u ? 15u : 47u, cc1);
        return;
    }
#endif
    /* Fixed logarithmic CC1 curve: factor[i] is (7600/120)^(2^i/127).
     * Seven factors cover the complete 7-bit input without a vendor libm
     * dependency or a 128-entry table. Input and endpoints are unchanged. */
    static const float factor[7] = {
        0x1.08800e0000000p+0f, 0x1.11485e0000000p+0f, 0x1.23bb6e0000000p+0f,
        0x1.4c73a60000000p+0f, 0x1.afbc2a0000000p+0f, 0x1.6c0d900000000p+1f,
        0x1.02db480000000p+3f
    };
    float cutoff = 360.f;
    for (unsigned bits = 127u - cc1, i = 0; bits; bits >>= 1, ++i)
        if (bits & 1u) cutoff *= factor[i];
    (void)s1c_front_set(&part->e, P_CUT, cutoff);
}
#if defined(X0X_S1_BEND_RANGE_PROBE) && X0X_S1_BEND_RANGE_PROBE
/* Original signed bounds act only on the transient MIDI wheel word. */
static void s1_bend_refresh(s1_dry_part *part)
{
    const unsigned raw = part->bend_raw;
    float st = raw < 8192 ? part->bend_min * ((float)(8191u - raw) / 8191.f)
                          : part->bend_max * ((float)(raw - 8192u) / 8192.f);
    part->e.t[P_DETM] = st == 0.f ? part->detm : part->detm * fm_exp2f(st / 12.f);
}
int s1_part_bend_ranges(s1_dry_part *part, float min_st, float max_st)
{
    if (!part || !(min_st >= -24.f && min_st <= 0.f && max_st >= 0.f && max_st <= 24.f)) return 0;
    part->bend_typed = 1;
    part->bend_min = min_st;
    part->bend_max = max_st;
    s1_bend_refresh(part);
    return 1;
}
#endif

int s1_part_param(s1_dry_part *part, unsigned id, float value)
{
    if (id > 2 || !(value >= 0.0f && value <= 1.0f)) return 0;
    if (id == 2) {
        /* The edited clock replaces the program's own (0 = 48 kHz, no crush). */
        (void)s1c_front_set(&part->e, P_BCSR, s1_part_crush_hz(value));
        return 1;
    }
    if (id == 0) {
#if defined(X0X_S1_BEND_RANGE_PROBE) && X0X_S1_BEND_RANGE_PROBE
        /* Invert the existing normalized ingress exactly for all 14-bit MIDI
         * words. The physical wheel/motion use this same receiver with q7. */
        if (part->bend_typed) {
            part->bend_raw = (uint16_t)(value < .5f ? value * 16384.f + .5f :
                (value - .5f) * 16382.f + 8192.5f);
            s1_bend_refresh(part);
            return 1;
        }
#endif
        /* +-2 semitones on the program's frequency multiplier (smoothed by the engine). */
        part->e.t[P_DETM] = part->detm * fm_exp2f((value - 0.5f) * (4.0f / 12.0f));
        return 1;
    }
    part->damper = value >= 0.5f;
    if (!part->damper && part->held) {
        for (unsigned i = 0; i < S1C_VOICES; ++i)
            if (part->held & (1u << i)) s1c_stop(&part->e, part->e.v[i].root);
        part->held = 0;
    }
    return 1;
}

int s1_part_note(s1_dry_part *part, unsigned key, unsigned velocity)
{
    if (key > 127 || velocity > 127) return 0;
    if (velocity) {
        for (unsigned i = 0; i < S1C_VOICES; ++i)
            if (part->e.v[i].root == (int)key) part->held &= (uint8_t)~(1u << i);
        s1c_start(&part->e, (int)key, (int)velocity);
        return 1;
    }
    if (part->damper) {
        for (unsigned i = 0; i < S1C_VOICES; ++i)
            if (part->e.v[i].root == (int)key && part->e.v[i].stage == 1) part->held |= (uint8_t)(1u << i);
        return 1;
    }
    s1c_stop(&part->e, (int)key);
    return 1;
}

int s1_part_active(const s1_dry_part *part)
{
    for (unsigned i = 0; i < S1C_VOICES; ++i)
        if (part->e.v[i].root != -1 && part->e.v[i].stage) return 1;
    return 0;
}

/* Loudest sounding voice (amp envelope), or -1; key_down: gate held and not by the damper. */
static int loudest(const s1_dry_part *part, unsigned *key_down)
{
    int sel = -1;
    for (unsigned i = 0; i < S1C_VOICES; ++i) {
        const s1c_voice *v = &part->e.v[i];
        if (v->root != -1 && v->stage && (sel < 0 || v->ea.y > part->e.v[sel].ea.y)) sel = (int)i;
    }
    if (sel >= 0) *key_down = part->e.v[sel].stage == 1 && !(part->held & (1u << sel));
    return sel;
}

int s1_part_voice_view(const s1_dry_part *part, float *level, float *modulation, unsigned *state, unsigned *key_down)
{
    int sel = loudest(part, key_down);
    if (sel < 0) return 0;
    const s1c_voice *v = &part->e.v[sel];
    *level = v->amp;
    *modulation = v->fenv;
    *state = v->stage == 2 ? 3u : v->ea.mode == 1 ? 0u : 1u;
    return 1;
}

int s1_part_env_view(const s1_dry_part *part, unsigned filter, float *level, unsigned *stage, unsigned *key_down)
{
    int sel = loudest(part, key_down);
    if (sel < 0) return 0;
    const s1c_env *e = filter ? &part->e.v[sel].ef : &part->e.v[sel].ea;
    *level = e->y;
    *stage = e->mode;            /* E_CLEAR 0, E_ATK 1, E_DEC 2, E_REL 3 */
    return 1;
}

void s1_part_render(s1_dry_part *part, float *left, float *right, unsigned frames)
{
    if (!frames) return;
    if (!part->arena) {
        memset(left, 0, frames * sizeof *left);
        memset(right, 0, frames * sizeof *right);
        return;
    }
    s1c_render(&part->e, left, right, frames);
}
