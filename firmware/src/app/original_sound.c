/* SPDX-License-Identifier: GPL-3.0-only */
#include "original_sound.h"
#include "unit_control.h"
#if X0X_SYNTHONE_PATCH
#include "s1_patch.h"
#endif
#if X0X_S1C
#include "s1c_part.h"
#endif
#include <string.h>
#include "x0x_cold.h"

static const char *const original_sound_fx_names[ORIGINAL_CHICAGO_FX_CHOICES] = {
    "OFF", "CHO/FLG", "PHASER", "RING MOD", "COMPRESSOR", "DECIMATOR", "EQ",
    "ENSEM", "SH DLY", "BPM DL", "HALL", "GT REV"
};
static const x0x_param_t original_sound_chicago_params[22] = {
    {"Wave",127,0,0},{"Glide",127,0,0},{"Cutoff",127,0,0},{"Peak",127,0,0},
    {"EnvAmt",127,0,0},{"Bite",127,0,0},{"Gnaw",127,0,0},{"Attack",127,0,0},
    {"Decay",127,0,0},{"Sust",127,0,0},{"Rel",127,0,0},{"AmpMod",127,0,0},
    {"Level",127,0,0},{"ArpOn",127,0,0},{"ArpMod",127,0,0},{"ArpRng",127,0,0},
    {"ArpSpd",127,0,0},{"FxType",ORIGINAL_CHICAGO_FX_CHOICES - 1u,0,original_sound_fx_names},{"FxEd1",127,0,0},{"FxEd2",127,0,0},
    {"PBend",127,0,0},{"Damper",127,0,0}
};

/* Visible Chicago factory order -> original program ID. Positions 0..54 keep
 * the stored indexes of the 55 compact-policy programs (saved schema-2 cells
 * stay the same sound). Positions 55..80 are the 26 programs restored with the
 * wip Ensemble, Short Delay, BPM Delay, Hall and Gate Reverb inserts.
 * Program data and untouched floats remain in their original immutable table. */
static const uint8_t original_sound_chicago_factories[ORIGINAL_CHICAGO_FACTORIES] = {
    0,1,2,3,4,5,6,11,12,13,14,15,17,18,19,21,22,29,30,31,32,34,35,36,37,38,39,40,
    41,42,43,46,47,49,52,53,54,55,57,58,59,60,62,63,67,69,70,71,72,73,74,76,78,79,80,
    7,8,9,10,16,20,23,24,25,26,27,28,33,44,45,48,50,51,56,61,64,65,66,68,75,77
};
static const uint8_t original_sound_fx_types[ORIGINAL_CHICAGO_FX_CHOICES] = {0,4,5,10,18,22,24,3,12,13,15,16};
static const uint8_t original_sound_fx_macros[ORIGINAL_CHICAGO_FX_CHOICES] = {0,5,7,4,2,3,1,6,8,9,10,11};
static const x0x_param_t original_sound_juno_params[18] = {
 {"Wave",127,0,0},{"Sub",127,0,0},{"Cutoff",127,0,0},{"Res",127,0,0},
 {"EnvAmt",127,0,0},{"Attack",127,0,0},{"Decay",127,0,0},{"Sustain",127,0,0},
 {"Release",127,0,0},{"HPF",127,0,0},{"LFORate",127,0,0},{"LFODep",127,0,0},
 {"Width",127,0,0},{"ChoRate",127,0,0},{"Chorus",127,0,0},{"Level",127,0,0},
 {"PBend",127,0,0},{"Damper",127,0,0}
};
static const uint8_t original_sound_juno_factories[4][16] = {
 {42,28,70,18,45,8,40,92,40,0,15,8,64,25,45,100},
 {90,18,42,24,35,65,55,95,75,0,12,15,55,40,55,100},
 {15,50,58,36,80,0,35,30,20,5,20,0,64,25,18,110},
 {70,10,105,12,20,0,20,110,15,12,25,5,80,60,50,96}
};
static const char *const original_sound_juno_names[4] = {"SUPERJ6 POLY", "SUPERJ6 PAD", "SUPERJ6 PLUCK", "SUPERJ6 BRASS"};
static const x0x_param_t original_sound_synthone_params[ORIGINAL_SYNTHONE_NPARAMS] = {
    {"PBend",127,64,0},{"Damper",127,0,0},{"Crush",127,0,0}
};
static const char *const original_sound_synthone_names[ORIGINAL_SYNTHONE_FACTORIES] = {
    "INIT"
};
static const x0x_param_t original_sound_miami_params[18] = {
    {"CrWave",127,0,0},{"CrLvl",127,0,0},{"Sub",127,0,0},{"CrPit",127,0,0},
    {"Glide",127,0,0},{"XmLvl",127,0,0},{"XmWave",127,0,0},{"XmTune",127,0,0},
    {"XmDep",127,0,0},{"Cutoff",127,0,0},{"Crush",127,0,0},{"WobShp",127,0,0},
    {"WobDep",127,0,0},{"WobRat",127,0,0},{"Output",127,0,0},{"Stereo",127,0,0},
    {"PBend",127,0,0},{"Damper",127,0,0}
};

/* Only these original bipolar panel controls need an exact editable midpoint.
 * The persisted byte still spans0..127; untouched factory floats bypass this
 * editor conversion. Keep the two rounded upper-half operations explicit. */
static int original_sound_centered(unsigned kind, unsigned parameter)
{
    /* Pitch bend is centred too: control 64 is exactly 0.5 (no bend), as the
     * saved rest value in original_sound_value. */
    return (kind == ORIGINAL_SOUND_CHICAGO && (parameter == 4u || parameter == ORIGINAL_CHICAGO_PITCH_BEND)) ||
           (kind == ORIGINAL_SOUND_MIAMI && (parameter == 7u || parameter == ORIGINAL_MIAMI_PITCH_BEND)) ||
           (kind == ORIGINAL_SOUND_JUNO && parameter == 16u) ||
           (kind == ORIGINAL_SOUND_SYNTHONE && parameter == 0u);
}
static uint8_t original_sound_display_byte(unsigned kind, unsigned parameter, float value)
{
    float position = original_sound_centered(kind, parameter) ?
        (value < 0.5f ? 128.0f * value : 64.0f + 126.0f * (value - 0.5f)) :
        127.0f * value;
    return (uint8_t)(position + 0.5f);
}

unsigned original_sound_count(unsigned kind)
{ return kind == ORIGINAL_SOUND_CHICAGO ? 22u : kind == ORIGINAL_SOUND_MIAMI ? 18u :
    kind == ORIGINAL_SOUND_JUNO ? ORIGINAL_JUNO_NPARAMS : kind == ORIGINAL_SOUND_SYNTHONE ? ORIGINAL_SYNTHONE_NPARAMS : 0u; }
#if X0X_S1C
#define S1_FACTORIES (1u + ORIGINAL_SYNTHONE_PROGRAMS)
static unsigned s1_cell_program(const uint8_t *cell)
{
    return cell[21] ? (unsigned)(cell[21] & 0x3fu) << 7 | (cell[7] & 127u) : 0u;
}
unsigned original_sound_s1_program(const uint8_t cell[ORIGINAL_SOUND_BYTES])
{
    return cell && cell[29] == ORIGINAL_SOUND_SYNTHONE ? s1_cell_program(cell) : 0u;
}
#else
#define S1_FACTORIES ORIGINAL_SYNTHONE_FACTORIES
#endif
unsigned original_sound_factories(unsigned kind)
{ return kind == ORIGINAL_SOUND_CHICAGO ? ORIGINAL_CHICAGO_FACTORIES : kind == ORIGINAL_SOUND_MIAMI ? 30u :
    kind == ORIGINAL_SOUND_JUNO ? ORIGINAL_JUNO_FACTORIES : kind == ORIGINAL_SOUND_SYNTHONE ? S1_FACTORIES : 0u; }
int original_sound_factory_source(unsigned kind, unsigned factory)
{
    if (factory >= original_sound_factories(kind)) return -1;
    return kind == ORIGINAL_SOUND_CHICAGO ? original_sound_chicago_factories[factory] : (int)factory;
}
const char *original_sound_fx_name(unsigned choice)
{ return choice < ORIGINAL_CHICAGO_FX_CHOICES ? original_sound_fx_names[choice] : 0; }
static const float original_sound_upper_value[64] = {
    0x1p-1f,     0x1.041042p-1f,     0x1.082082p-1f,     0x1.0c30c4p-1f,
    0x1.104104p-1f,     0x1.145146p-1f,     0x1.186186p-1f,     0x1.1c71c8p-1f,
    0x1.208208p-1f,     0x1.24924ap-1f,     0x1.28a28ap-1f,     0x1.2cb2ccp-1f,
    0x1.30c30cp-1f,     0x1.34d34ep-1f,     0x1.38e38ep-1f,     0x1.3cf3dp-1f,
    0x1.41041p-1f,     0x1.451452p-1f,     0x1.492492p-1f,     0x1.4d34d4p-1f,
    0x1.514514p-1f,     0x1.555556p-1f,     0x1.596596p-1f,     0x1.5d75d8p-1f,
    0x1.618618p-1f,     0x1.65965ap-1f,     0x1.69a69ap-1f,     0x1.6db6dcp-1f,
    0x1.71c71cp-1f,     0x1.75d75ep-1f,     0x1.79e79ep-1f,     0x1.7df7ep-1f,
    0x1.82082p-1f,     0x1.861862p-1f,     0x1.8a28a4p-1f,     0x1.8e38e4p-1f,
    0x1.924924p-1f,     0x1.965966p-1f,     0x1.9a69a8p-1f,     0x1.9e79e8p-1f,
    0x1.a28a28p-1f,     0x1.a69a6ap-1f,     0x1.aaaaacp-1f,     0x1.aebaecp-1f,
    0x1.b2cb2cp-1f,     0x1.b6db6ep-1f,     0x1.baebbp-1f,     0x1.befbfp-1f,
    0x1.c30c3p-1f,     0x1.c71c72p-1f,     0x1.cb2cb4p-1f,     0x1.cf3cf4p-1f,
    0x1.d34d34p-1f,     0x1.d75d76p-1f,     0x1.db6db8p-1f,     0x1.df7df8p-1f,
    0x1.e38e38p-1f,     0x1.e79e7ap-1f,     0x1.ebaebcp-1f,     0x1.efbefcp-1f,
    0x1.f3cf3cp-1f,     0x1.f7df7ep-1f,     0x1.fbefcp-1f,     0x1p+0f,

};
static const float original_sound_macro_value[12] = {
    0x0p+0f,     0x1.745d18p-4f,     0x1.745d18p-3f,     0x1.1745d2p-2f,
    0x1.745d18p-2f,     0x1.d1745ep-2f,     0x1.1745d2p-1f,     0x1.45d174p-1f,
    0x1.745d18p-1f,     0x1.a2e8bap-1f,     0x1.d1745ep-1f,     0x1p+0f,

};
int original_sound_control_value(unsigned kind, unsigned parameter, unsigned control, float *value)
{
    const x0x_param_t *p = original_sound_param(kind, parameter);
    if (!value || !p || control > p->max) return 0;
    if (kind == ORIGINAL_SOUND_CHICAGO && parameter == ORIGINAL_CHICAGO_FX_TYPE)
        *value = original_sound_macro_value[original_sound_fx_macros[control]];
    else if (original_sound_centered(kind, parameter))
        *value = control < 64u ? (float)control / 128.0f :
            original_sound_upper_value[control - 64u];
    else *value = unit_control(control);
    return 1;
}
uint32_t original_sound_engine_id(unsigned kind)
{ return kind == ORIGINAL_SOUND_CHICAGO ? ORIGINAL_SOUND_CHICAGO_ID : kind == ORIGINAL_SOUND_MIAMI ? ORIGINAL_SOUND_MIAMI_ID :
    kind == ORIGINAL_SOUND_JUNO ? ORIGINAL_SOUND_JUNO_ID : kind == ORIGINAL_SOUND_SYNTHONE ? ORIGINAL_SOUND_SYNTHONE_ID : 0u; }
X0X_COLD const char *original_sound_name(unsigned kind, unsigned factory)
{
    int source = original_sound_factory_source(kind, factory);
    if (source < 0) return 0;
    return kind == ORIGINAL_SOUND_CHICAGO ? original_chicago_program_name((unsigned)source) :
    kind == ORIGINAL_SOUND_MIAMI ? original_miami_program_name((unsigned)source) :
    kind == ORIGINAL_SOUND_JUNO ? original_sound_juno_names[factory] :
#if X0X_S1C
    factory ? s1_part_program_name(factory - 1u) :
#endif
    original_sound_synthone_names[0];
}
const x0x_param_t *original_sound_param(unsigned kind, unsigned parameter)
{
    if (parameter >= original_sound_count(kind)) return 0;
    return kind == ORIGINAL_SOUND_CHICAGO ? original_sound_chicago_params + parameter :
    kind == ORIGINAL_SOUND_MIAMI ? original_sound_miami_params + parameter : kind == ORIGINAL_SOUND_JUNO ? original_sound_juno_params + parameter : original_sound_synthone_params + parameter;
}
uint32_t original_sound_edit_mask(const uint8_t cell[ORIGINAL_SOUND_BYTES])
{
    if (!cell) return 0;
    return (uint32_t)cell[22] | (uint32_t)cell[23] << 8 | (uint32_t)cell[24] << 16 | (uint32_t)cell[25] << 24;
}
static void original_sound_put_mask(uint8_t cell[ORIGINAL_SOUND_BYTES], uint32_t mask)
{
    for (unsigned i = 0; i < 4; ++i) cell[22 + i] = (uint8_t)(mask >> (8 * i));
}

int original_sound_is_legacy_slot2_cell(const uint8_t cell[ORIGINAL_SOUND_BYTES])
{
    unsigned i, used = 0;
    if (!cell || cell[26] != 'O' || cell[27] != 'S' || cell[28] != ORIGINAL_SOUND_SCHEMA ||
        cell[29] != ORIGINAL_SOUND_SYNTHONE || cell[31] || cell[30] > 3u) return 0;
    /* Current Synth One init: factory 0, no edits, bytes 1..21 zero, optional display 64 at [0]. */
    if (cell[30] == 0u && !original_sound_edit_mask(cell)) {
        for (i = 1; i < 22; ++i) if (cell[i]) { used = 1; break; }
        if (!used && (cell[0] == 0 || cell[0] == 64)) return 0;
    }
    /* Historical 8-parameter layout: six q127 values, two performance bytes, tail zeros. */
    for (i = 8; i < 22; ++i) if (cell[i]) return 0;
    if (original_sound_edit_mask(cell) >> 8) return 0;
    return 1;
}
int original_sound_adopt_legacy_cell(uint8_t cell[ORIGINAL_SOUND_BYTES])
{
    if (!cell) return 0;
    if (!original_sound_is_legacy_slot2_cell(cell)) return 0;
    return original_sound_init(cell, ORIGINAL_SOUND_SYNTHONE, ORIGINAL_SYNTHONE_INIT_FACTORY);
}
int original_sound_valid(const uint8_t cell[ORIGINAL_SOUND_BYTES])
{
    if (!cell || cell[26] != 'O' || cell[27] != 'S' || cell[28] != ORIGINAL_SOUND_SCHEMA) return 0;
    int crush_edit = cell[29] == ORIGINAL_SOUND_SYNTHONE &&
        (original_sound_edit_mask(cell) & (1u << ORIGINAL_SYNTHONE_CRUSH));
    if (cell[31] != crush_edit ||
        (cell[29] == ORIGINAL_SOUND_SYNTHONE && !crush_edit && cell[ORIGINAL_SYNTHONE_CRUSH])) return 0;
    unsigned count = original_sound_count(cell[29]);
    if (!count || cell[30] >= original_sound_factories(cell[29])) return 0;
#if X0X_S1C
    /* Synth One program bytes [7] and [21]: checked here, then masked for the zero checks. */
    uint8_t s1c_copy[ORIGINAL_SOUND_BYTES];
    if (cell[29] == ORIGINAL_SOUND_SYNTHONE) {
        if (cell[30] || (cell[21] && ((cell[21] & 0xc0u) != 0x40u || cell[7] > 127u)) || (!cell[21] && cell[7]) ||
            s1_cell_program(cell) > ORIGINAL_SYNTHONE_PROGRAMS) return 0;
        for (unsigned i = 0; i < ORIGINAL_SOUND_BYTES; ++i) s1c_copy[i] = cell[i];
        s1c_copy[7] = s1c_copy[21] = 0;
        cell = s1c_copy;
    }
#endif
#if X0X_SYNTHONE_PATCH
    uint32_t mask = original_sound_edit_mask(cell);
    if (cell[29] == ORIGINAL_SOUND_SYNTHONE && (mask & S1_PATCH_MASK)) {
        if (mask & ~(S1_PATCH_MASK | 7u)) return 0;
        for (unsigned i = 0; i < 22; ++i) {
            if (cell[i] > 127u || (i >= 2 && !(mask & (UINT32_C(1) << i)) && cell[i])) return 0;
        }
        return 1;
    }
    if (mask >> count) return 0;
#else
    if (original_sound_edit_mask(cell) >> count) return 0;
#endif
    for (unsigned i = 0; i < count; ++i) if (cell[i] > original_sound_param(cell[29], i)->max) return 0;
    for (unsigned i = count; i < 22; ++i) if (cell[i]) return 0;
    return 1;
}
int original_sound_init(uint8_t cell[ORIGINAL_SOUND_BYTES], unsigned kind, unsigned factory)
{
    int source = original_sound_factory_source(kind, factory);
    if (!cell || source < 0) return 0;
    uint8_t next[ORIGINAL_SOUND_BYTES] = {0};
    if (kind == ORIGINAL_SOUND_CHICAGO) {
        original_chicago_program p;
        original_chicago_routing routing;
        if (!original_chicago_program_init(&p, (unsigned)source) || !original_chicago_program_routing(&p, &routing)) return 0;
        for (unsigned i = 0; i < 22; ++i) next[i] = original_sound_display_byte(kind, i, p.macro[i]);
        unsigned choice = 0;
        while (choice < ORIGINAL_CHICAGO_FX_CHOICES && original_sound_fx_types[choice] != routing.fx[1][0]) ++choice;
        if (choice == ORIGINAL_CHICAGO_FX_CHOICES) return 0;
        next[ORIGINAL_CHICAGO_FX_TYPE] = (uint8_t)choice;
    } else if (kind == ORIGINAL_SOUND_MIAMI) {
        original_miami_program p;
        if (!original_miami_program_init(&p, (unsigned)source)) return 0;
        for (unsigned i = 0; i < 18; ++i) next[i] = original_sound_display_byte(kind, i, p.macro[i]);
    } else if (kind == ORIGINAL_SOUND_JUNO) {
        memcpy(next, original_sound_juno_factories[factory], 16u);
    } else if (kind == ORIGINAL_SOUND_SYNTHONE) {
        /* Init cell: performance defaults only. Full 150-parameter packing is not admitted. */
        next[0] = 64;
#if X0X_S1C
        if (factory) { next[7] = (uint8_t)(factory & 127u); next[21] = (uint8_t)(0x40u | factory >> 7); }
#endif
    } else return 0;
    next[26] = 'O'; next[27] = 'S'; next[28] = ORIGINAL_SOUND_SCHEMA;
    next[29] = (uint8_t)kind; next[30] = (uint8_t)(kind == ORIGINAL_SOUND_SYNTHONE ? 0u : factory);
    memcpy(cell, next, sizeof next);
    return 1;
}
int original_sound_set(uint8_t cell[ORIGINAL_SOUND_BYTES], unsigned parameter, unsigned value)
{
    if (!original_sound_valid(cell) || parameter >= original_sound_count(cell[29]) ||
        original_sound_is_performance(cell[29], parameter) ||
        value > original_sound_param(cell[29], parameter)->max) return 0;
    uint32_t mask = original_sound_edit_mask(cell) | (UINT32_C(1) << parameter);
    cell[parameter] = (uint8_t)value;
    if (cell[29] == ORIGINAL_SOUND_SYNTHONE && parameter == ORIGINAL_SYNTHONE_CRUSH) cell[31] = 1;
    original_sound_put_mask(cell, mask);
    return 1;
}
#if X0X_SYNTHONE_PATCH
int original_sound_s1_set(uint8_t cell[ORIGINAL_SOUND_BYTES], unsigned slot, unsigned position)
{
    if (slot >= S1_PATCH_COUNT || position > S1_PATCH_STEPS ||
        !original_sound_valid(cell) || cell[29] != ORIGINAL_SOUND_SYNTHONE) return 0;
    unsigned byte = S1_PATCH_FIRST_BYTE + slot;
    cell[byte] = (uint8_t)position;
    original_sound_put_mask(cell, original_sound_edit_mask(cell) | (UINT32_C(1) << byte));
    return 1;
}
#endif

X0X_COLD int original_sound_restore_miami(original_miami_program *out, const uint8_t cell[ORIGINAL_SOUND_BYTES])
{
    if (!out || !original_sound_valid(cell) || cell[29] != ORIGINAL_SOUND_MIAMI) return 0;
    original_miami_program next;
    if (!original_miami_program_init(&next, (unsigned)original_sound_factory_source(cell[29], cell[30]))) return 0;
    uint32_t mask = original_sound_edit_mask(cell);
    for (unsigned i = 0; i < ORIGINAL_MIAMI_PITCH_BEND; ++i) if (mask & (UINT32_C(1) << i)) {
        float value;
        if (!original_sound_control_value(cell[29], i, cell[i], &value) ||
            !original_miami_program_set(&next, i, value)) return 0;
    }
    *out = next;
    return 1;
}
X0X_COLD int original_sound_restore_chicago(original_chicago_program *out, const uint8_t cell[ORIGINAL_SOUND_BYTES])
{
    if (!out || !original_sound_valid(cell) || cell[29] != ORIGINAL_SOUND_CHICAGO) return 0;
    original_chicago_program next;
    if (!original_chicago_program_init(&next, (unsigned)original_sound_factory_source(cell[29], cell[30]))) return 0;
    uint32_t mask = original_sound_edit_mask(cell);
    for (unsigned i = 0; i < ORIGINAL_CHICAGO_PITCH_BEND; ++i) if (mask & (UINT32_C(1) << i)) {
        float value;
        if (!original_sound_control_value(cell[29], i, cell[i], &value) ||
            !original_chicago_program_set(&next, i, value)) return 0;
    }
    *out = next;
    return 1;
}
int original_sound_value(const uint8_t cell[ORIGINAL_SOUND_BYTES], unsigned parameter, float *value)
{
    if (!value || !original_sound_valid(cell) || parameter >= original_sound_count(cell[29])) return 0;
    if (original_sound_is_performance(cell[29], parameter)) {
        *value = parameter == original_sound_bend_id(cell[29]) ? 0.5f : 0.0f;
        return 1;
    }
    float result;
    if (original_sound_edit_mask(cell) & (UINT32_C(1) << parameter)) {
        if (!original_sound_control_value(cell[29], parameter, cell[parameter], &result)) return 0;
    }
    else if (cell[29] == ORIGINAL_SOUND_SYNTHONE)
        result = 0.f;
    else if (cell[29] == ORIGINAL_SOUND_JUNO)
        result = unit_control(original_sound_juno_factories[cell[30]][parameter]);
    else if (cell[29] == ORIGINAL_SOUND_CHICAGO) {
        original_chicago_program p;
        if (!original_chicago_program_init(&p, (unsigned)original_sound_factory_source(cell[29], cell[30]))) return 0;
        result = p.macro[parameter];
    } else {
        original_miami_program p;
        if (!original_miami_program_init(&p, cell[30])) return 0;
        result = p.macro[parameter];
    }
    *value = result;
    return 1;
}

