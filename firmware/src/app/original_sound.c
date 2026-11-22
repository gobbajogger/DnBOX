/* SPDX-License-Identifier: GPL-3.0-only */
#include "original_sound.h"
#include "unit_control.h"
#if X0X_SYNTHONE_PATCH
#include "s1_patch.h"
#endif
#include <string.h>

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
    {"PBend",127,64,0},{"Damper",127,0,0},{"Crush",127,0,0},{"Width",127,0,0},
    {"Pan",127,0,0},{"Pan Hz",127,ORIGINAL_SYNTHONE_PAN_RATE_INIT,0},
    {"Trem",127,0,0},{"LFO1 wav",127,0,0},{"LFO1 amt",127,0,0},{"LFO1 Hz",127,64,0},
    {"LFO2 wav",127,0,0},{"LFO2 amt",127,0,0},{"LFO2 Hz",127,64,0},
    {"PhMix",127,0,0},{"PhRate",127,5,0},{"PhFeed",127,0,0},{"PhWidth",127,99,0},
    {"Master",127,32,0},{"Ratio",127,0,0},{"Thresh",127,127,0},{"Attack",127,0,0},
    {"Release",127,0,0},{"Makeup",127,18,0},{"Cr LFO",127,0,0}
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
unsigned original_sound_factories(unsigned kind)
{ return kind == ORIGINAL_SOUND_CHICAGO ? ORIGINAL_CHICAGO_FACTORIES : kind == ORIGINAL_SOUND_MIAMI ? 30u :
    kind == ORIGINAL_SOUND_JUNO ? ORIGINAL_JUNO_FACTORIES : kind == ORIGINAL_SOUND_SYNTHONE ? ORIGINAL_SYNTHONE_FACTORIES : 0u; }
int original_sound_factory_source(unsigned kind, unsigned factory)
{
    if (factory >= original_sound_factories(kind)) return -1;
    return kind == ORIGINAL_SOUND_CHICAGO ? original_sound_chicago_factories[factory] : (int)factory;
}
const char *original_sound_fx_name(unsigned choice)
{ return choice < ORIGINAL_CHICAGO_FX_CHOICES ? original_sound_fx_names[choice] : 0; }
/* The source centers at .5, then rounds .5 + .5 * float(index / 63).
 * In this exponent bin the steps are 2^23/63 ulps. Eight upper-half
 * half-way additions round up once more under nearest-even. */
static float centered_upper(unsigned index)
{
    uint32_t mantissa=133152u*index+(index+1u)/2u;
    if (index>=32u && index%4u==2u) ++mantissa;
    uint32_t bits=UINT32_C(0x3f000000)+mantissa;
    float value; memcpy(&value,&bits,sizeof value);
    return value;
}
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
            centered_upper(control-64u);
    else *value = unit_control(control);
    return 1;
}
uint32_t original_sound_engine_id(unsigned kind)
{ return kind == ORIGINAL_SOUND_CHICAGO ? ORIGINAL_SOUND_CHICAGO_ID : kind == ORIGINAL_SOUND_MIAMI ? ORIGINAL_SOUND_MIAMI_ID :
    kind == ORIGINAL_SOUND_JUNO ? ORIGINAL_SOUND_JUNO_ID : kind == ORIGINAL_SOUND_SYNTHONE ? ORIGINAL_SOUND_SYNTHONE_ID : 0u; }
const char *original_sound_name(unsigned kind, unsigned factory)
{
    int source = original_sound_factory_source(kind, factory);
    if (source < 0) return 0;
    return kind == ORIGINAL_SOUND_CHICAGO ? original_chicago_program_name((unsigned)source) :
    kind == ORIGINAL_SOUND_MIAMI ? original_miami_program_name((unsigned)source) :
    kind == ORIGINAL_SOUND_JUNO ? original_sound_juno_names[factory] : original_sound_synthone_names[factory];
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
    uint32_t marker = cell[26]=='M' ? 1u : 0u;
    uint32_t mask = marker | (uint32_t)cell[22] | (uint32_t)cell[23] << 8 | (uint32_t)cell[24] << 16 | (uint32_t)cell[25] << 24;
    if (cell[29] == ORIGINAL_SOUND_SYNTHONE) {
        if (cell[31] & 128u) return mask & 0x003fff7fu;
        if (cell[31] == 2) return mask & 0x003fffffu;
    }
    return mask;
}
static void original_sound_put_mask(uint8_t cell[ORIGINAL_SOUND_BYTES], uint32_t mask)
{
    if (cell[29] == ORIGINAL_SOUND_SYNTHONE && (cell[31] == 2 || cell[31] & 128u)) {
        if (cell[31] & 128u) mask |= cell[22] & 128u;
        mask |= ((uint32_t)cell[24] << 16 | (uint32_t)cell[25] << 24) & 0xffc00000u;
    }
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
/* The extended tag stores the four seven-bit values and three two-bit
 * selectors in bytes 6, 7, 21 and the ten previously unused high mask bits.
 * Bytes 8..20 retain the optional engine's exact payload and edit bits. */
unsigned original_sound_s1_display(const uint8_t cell[ORIGINAL_SOUND_BYTES], unsigned id)
{
    if (id >= ORIGINAL_SYNTHONE_NPARAMS) return 0;
    if (id == ORIGINAL_SYNTHONE_CRUSH_LFO) {
        unsigned route = (cell[8] >> 7) | ((cell[9] >> 7) << 1);
        return (route * 127u + 1u) / 3u;
    }
    if (id >= ORIGINAL_SYNTHONE_MASTER_VOLUME) return original_sound_synthone_params[id].def;
    if (id >= ORIGINAL_SYNTHONE_PHASER_MIX) {
        if (!(cell[31] & 128u)) return original_sound_synthone_params[id].def;
        if (id == ORIGINAL_SYNTHONE_PHASER_MIX) return cell[0] & 127u;
        if (id == ORIGINAL_SYNTHONE_PHASER_RATE) return cell[1] & 127u;
        if (id == ORIGINAL_SYNTHONE_PHASER_FEEDBACK) return cell[31] & 127u;
        unsigned width = (cell[22] & 128u) >> 1;
        for (unsigned i=0;i<6;++i) width |= (unsigned)(cell[i] >> 7) << i;
        return width;
    }
    if (id < ORIGINAL_SYNTHONE_BASE_COUNT) {
        if ((cell[31] & 128u) && id < 2u) return id == 0 ? 64u : 0u;
        if (id == ORIGINAL_SYNTHONE_PAN_RATE && !(original_sound_edit_mask(cell) & (1u << id)))
            return ORIGINAL_SYNTHONE_PAN_RATE_INIT;
        return cell[id] & 127u;
    }
    if (cell[31] != 2 && !(cell[31] & 128u)) return id == ORIGINAL_SYNTHONE_LFO1_RATE || id == ORIGINAL_SYNTHONE_LFO2_RATE ? 64u : 0u;
    switch (id) {
    case ORIGINAL_SYNTHONE_LFO1_LEVEL: return cell[6] & 127u;
    case ORIGINAL_SYNTHONE_LFO1_RATE: return cell[7] & 127u;
    case ORIGINAL_SYNTHONE_LFO2_LEVEL: return cell[21] & 127u;
    case ORIGINAL_SYNTHONE_LFO2_RATE: return cell[25] & 127u;
    case ORIGINAL_SYNTHONE_TREM: return (((cell[6] >> 7) | (cell[7] >> 7) << 1) * 127u + 1u) / 3u;
    case ORIGINAL_SYNTHONE_LFO1_SHAPE: return (((cell[21] >> 7) | ((cell[24] >> 5) & 2u)) * 127u + 1u) / 3u;
    default: return (((cell[24] >> 7) | (cell[25] >> 7) << 1) * 127u + 1u) / 3u;
    }
}
static void s1_put_lfo(uint8_t cell[ORIGINAL_SOUND_BYTES], unsigned id, unsigned q)
{
    if (cell[31] != 2 && !(cell[31] & 128u)) {
        cell[31] = 2;
        cell[22] |= (1u << ORIGINAL_SYNTHONE_TREM);
        cell[7] = cell[25] = 64;
    }
    /* The optional patch uses only the lower seven bits of these positions. */
    if (id == ORIGINAL_SYNTHONE_CRUSH_LFO) {
        unsigned route = (q * 3u + 63u) / 127u;
        cell[8] = (cell[8] & 127u) | ((route & 1u) << 7);
        cell[9] = (cell[9] & 127u) | ((route >> 1) << 7);
        return;
    }
    /* The two performance inputs always restore neutral. Only the new format
     * reuses their display bytes for phaser mix and rate. Feedback occupies
     * the tag payload. Width uses high bits of bytes 0..5 and mask bit 7. */
    if (id >= ORIGINAL_SYNTHONE_PHASER_MIX) {
        if (!(cell[31] & 128u)) {
            cell[31]=128u;
            cell[0]=0; cell[1]=5;
            s1_put_lfo(cell,ORIGINAL_SYNTHONE_PHASER_WIDTH,99);
        }
        if (id == ORIGINAL_SYNTHONE_PHASER_FEEDBACK) cell[31]=128u|q;
        else if (id == ORIGINAL_SYNTHONE_PHASER_WIDTH) {
            for (unsigned i=0;i<6;++i) cell[i]=(cell[i]&127u)|(((q>>i)&1u)<<7);
            cell[22]=(cell[22]&127u)|((q&64u)<<1);
        } else {
            unsigned byte=id-ORIGINAL_SYNTHONE_PHASER_MIX;
            cell[byte]=(cell[byte]&128u)|q;
        }
        return;
    }
    unsigned slot, lo, hi, bit;
    switch (id) {
    case ORIGINAL_SYNTHONE_LFO1_LEVEL: slot=6; break;
    case ORIGINAL_SYNTHONE_LFO1_RATE: slot=7; break;
    case ORIGINAL_SYNTHONE_LFO2_LEVEL: slot=21; break;
    case ORIGINAL_SYNTHONE_LFO2_RATE: slot=25; break;
    default:
        if (id == ORIGINAL_SYNTHONE_TREM) { lo=6; hi=7; bit=7; }
        else if (id == ORIGINAL_SYNTHONE_LFO1_SHAPE) { lo=21; hi=24; bit=6; }
        else { lo=24; hi=25; bit=7; }
        q = (q * 3u + 63u) / 127u;
        cell[lo] = (cell[lo] & 127u) | ((q & 1u) << 7);
        cell[hi] = (cell[hi] & ~(1u << bit)) | ((q >> 1) << bit);
        return;
    }
    cell[slot] = (cell[slot] & 128u) | q;
}

int original_sound_valid(const uint8_t cell[ORIGINAL_SOUND_BYTES])
{
    if (!cell || cell[27] != 'S' || cell[28] != ORIGINAL_SOUND_SCHEMA ||
        (cell[26] != 'O' && !(cell[26]=='M' && cell[29]==ORIGINAL_SOUND_SYNTHONE))) return 0;
    uint32_t mask = original_sound_edit_mask(cell);
    unsigned count = original_sound_count(cell[29]);
    if (!count || cell[30] >= original_sound_factories(cell[29])) return 0;
    if (cell[29] == ORIGINAL_SOUND_SYNTHONE) {
        const uint32_t effects = (1u << ORIGINAL_SYNTHONE_BASE_COUNT) - 4u;
        const uint32_t ext = 1u << ORIGINAL_SYNTHONE_TREM;
        unsigned tag = (mask & ext) ? 2u : (mask & effects) ? 1u : 0u;
        unsigned phaser=cell[31]&128u;
        if ((phaser ? 2u : cell[31]) != tag) return 0;
        uint32_t allowed = 3u | effects | ext;
#if X0X_SYNTHONE_PATCH
        allowed |= S1_PATCH_MASK;
#endif
        if (mask & ~allowed) return 0;
        for (unsigned i = 0; i < 22; ++i) {
            if (tag == 2 && (i == 6 || i == 7 || i == 21)) continue;
            unsigned packed = (phaser && i<6) || (tag==2 && (i==8 || i==9));
#if X0X_SYNTHONE_PATCH
            packed |= tag==2 && (i==10 || i==11);
#endif
            unsigned q=packed ? cell[i]&127u : cell[i];
            if (q > 127u || (i >= 2 && !(mask & (1u << i)) && q)) return 0;
        }
        return 1;
    }
    if (cell[31] || mask >> count) return 0;
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
    } else return 0;
    next[26] = 'O'; next[27] = 'S'; next[28] = ORIGINAL_SOUND_SCHEMA;
    next[29] = (uint8_t)kind; next[30] = (uint8_t)factory;
    memcpy(cell, next, sizeof next);
    return 1;
}
#if X0X_SYNTHONE_PATCH
/* The V2 tag is 1100. The two alternate-filter tags differ from it in
 * three bits, so one corrupt reserved bit cannot turn V2 into a filter edit.
 * All four marker bytes retain their existing seven-bit master controls. */
static unsigned s1_adjunct_tag(const uint8_t *ext)
{
    return (ext[3] >> 7) * 8u + (ext[4] >> 7) * 4u +
           (ext[5] >> 7) * 2u + (ext[6] >> 7);
}
#endif

int original_master_valid(const uint8_t *cell,const uint8_t *ext)
{
    if (cell[26]!='M') return 1;
    if (!ext) return 0;
#if X0X_SYNTHONE_PATCH
    /* V2 uses high bits of the first two master values for octave and
     * square. Its two-bit format marker leaves a single reserved-bit error
     * invalid in v1. Byte zero retains bit 0 for master edit presence. */
    const unsigned tag = s1_adjunct_tag(ext);
    if (tag == 12u || tag == 11u || tag == 7u) {
        for (unsigned j = 1; j < S1_MASTER_EXT_SIZE; ++j)
            if (!(ext[0] & 1u) && (ext[j] & 127u)) return 0;
        return 1;
    }
#endif
    if (ext[0]!=1u) return 0;
    for(unsigned j=1;j<S1_MASTER_EXT_SIZE;++j) if(ext[j]>127u) return 0;
    return 1;
}
unsigned original_master_display(const uint8_t *cell,const uint8_t *ext,unsigned id)
{
    if (!original_sound_s1_master_id(id)) return 0;
    if (!original_master_edited(cell, ext)) return original_sound_synthone_params[id].def;
#if X0X_SYNTHONE_PATCH
    return ext[1+id-ORIGINAL_SYNTHONE_MASTER_VOLUME] & 127u;
#else
    return ext[1+id-ORIGINAL_SYNTHONE_MASTER_VOLUME];
#endif
}
int original_sound_set_extended(uint8_t *cell,uint8_t *ext,unsigned id,unsigned value)
{
    if (cell[29]!=ORIGINAL_SOUND_SYNTHONE || !original_sound_s1_master_id(id))
        return original_sound_set(cell,id,value);
    if (id>=ORIGINAL_SYNTHONE_NPARAMS || value>127 || !ext || !original_sound_valid(cell) || !original_master_valid(cell,ext)) return 0;
#if X0X_SYNTHONE_PATCH
    if (!original_master_edited(cell, ext)) {
        if (cell[26] != 'M') memset(ext, 0, S1_MASTER_EXT_SIZE);
        ext[0] |= 1u;
        for (unsigned j=0; j<S1_MASTER_EXT_SIZE-1; ++j)
            ext[1+j]=(ext[1+j]&128u)|original_sound_synthone_params[ORIGINAL_SYNTHONE_MASTER_VOLUME+j].def;
        cell[26]='M';
    }
    const unsigned index = 1+id-ORIGINAL_SYNTHONE_MASTER_VOLUME;
    ext[index]=(ext[index]&128u)|(uint8_t)value;
#else
    if (cell[26]!='M') {
        ext[0]=1;
        for(unsigned j=0;j<S1_MASTER_EXT_SIZE-1;++j)
            ext[1+j]=original_sound_synthone_params[ORIGINAL_SYNTHONE_MASTER_VOLUME+j].def;
        cell[26]='M';
    }
    ext[1+id-ORIGINAL_SYNTHONE_MASTER_VOLUME]=(uint8_t)value;
#endif
    return 1;
}
int original_sound_set(uint8_t cell[ORIGINAL_SOUND_BYTES], unsigned parameter, unsigned value)
{
    if (!original_sound_valid(cell) || parameter >= original_sound_count(cell[29]) ||
        original_sound_is_performance(cell[29], parameter) ||
        value > original_sound_param(cell[29], parameter)->max) return 0;
    if (cell[29] == ORIGINAL_SOUND_SYNTHONE && original_sound_s1_master_id(parameter)) return 0;
    if (cell[29] == ORIGINAL_SOUND_SYNTHONE && parameter >= ORIGINAL_SYNTHONE_BASE_COUNT) {
        s1_put_lfo(cell, parameter, value);
        return 1;
    }
    uint32_t mask = original_sound_edit_mask(cell) | (UINT32_C(1) << parameter);
    cell[parameter] = cell[29] == ORIGINAL_SOUND_SYNTHONE && (cell[31] & 128u)
        ? (uint8_t)((cell[parameter]&128u)|value) : (uint8_t)value;
    if (cell[29] == ORIGINAL_SOUND_SYNTHONE && parameter >= ORIGINAL_SYNTHONE_CRUSH && cell[31] == 0) cell[31] = 1;
    original_sound_put_mask(cell, mask);
    return 1;
}
#if X0X_SYNTHONE_PATCH
int original_sound_s1_set(uint8_t cell[ORIGINAL_SOUND_BYTES], unsigned slot, unsigned position)
{
    if (slot >= S1_PATCH_CELL_COUNT || position > S1_PATCH_STEPS ||
        !original_sound_valid(cell) || cell[29] != ORIGINAL_SOUND_SYNTHONE) return 0;
    unsigned byte = S1_PATCH_FIRST_BYTE + slot;
    cell[byte] = (cell[byte] & 128u) | (uint8_t)position;
    original_sound_put_mask(cell, original_sound_edit_mask(cell) | (UINT32_C(1) << byte));
    return 1;
}

unsigned original_sound_s1_sub_position(const uint8_t *cell, const uint8_t *ext, unsigned slot)
{
    if (slot == S1_PATCH_PITCH_LFO) {
        if (cell[31] != 2 && !(cell[31] & 128u)) return 0;
        const unsigned route = (cell[10] >> 7) | ((cell[11] >> 7) << 1);
        return (route * 127u + 1u) / 3u;
    }
    if (cell[26] != 'M' || !ext) return 0;
    const unsigned tag = s1_adjunct_tag(ext);
    if (tag != 12u && tag != 11u && tag != 7u) return 0;
    if (slot == S1_PATCH_FILTER_TYPE)
        return tag == 11u ? 64u : tag == 7u ? 127u : 0u;
    if (slot == S1_PATCH_SUB_LEVEL) return ext[0] >> 1;
    if (slot == S1_PATCH_SUB_OCTAVE) return (ext[1] & 128u) ? 127u : 0u;
    if (slot == S1_PATCH_SUB_SQUARE) return (ext[2] & 128u) ? 127u : 0u;
    return 0;
}

int original_sound_s1_set_extended(uint8_t *cell, uint8_t *ext, unsigned slot, unsigned position)
{
    if (slot < S1_PATCH_CELL_COUNT) return original_sound_s1_set(cell, slot, position);
    if (slot == S1_PATCH_PITCH_LFO) {
        if (position > S1_PATCH_STEPS || !original_sound_valid(cell) ||
            cell[29] != ORIGINAL_SOUND_SYNTHONE) return 0;
        const unsigned route = (position * 3u + 63u) / 127u;
        if (cell[31] != 2 && !(cell[31] & 128u)) {
            if (!route) return 1;
            s1_put_lfo(cell, ORIGINAL_SYNTHONE_CRUSH_LFO, 0u);
        }
        cell[10] = (cell[10] & 127u) | ((route & 1u) << 7);
        cell[11] = (cell[11] & 127u) | ((route >> 1) << 7);
        return 1;
    }
    if (slot >= S1_PATCH_COUNT || position > S1_PATCH_STEPS || !ext ||
        !original_sound_valid(cell) || cell[29] != ORIGINAL_SOUND_SYNTHONE ||
        !original_master_valid(cell, ext)) return 0;
    if (cell[26] != 'M') {
        memset(ext, 0, S1_MASTER_EXT_SIZE);
        cell[26] = 'M';
    }
    unsigned tag = s1_adjunct_tag(ext);
    if (slot == S1_PATCH_FILTER_TYPE)
        tag = position <= 31u ? 12u : position >= 96u ? 7u : 11u;
    else if (tag != 11u && tag != 7u)
        tag = 12u;
    for (unsigned j = 3; j < S1_MASTER_EXT_SIZE; ++j)
        ext[j] = (ext[j] & 127u) | (uint8_t)((tag << (j + 1u)) & 128u);
    if (slot == S1_PATCH_FILTER_TYPE) return 1;
    if (slot == S1_PATCH_SUB_LEVEL) ext[0] = (ext[0] & 1u) | (uint8_t)(position << 1);
    else {
        const unsigned index = slot == S1_PATCH_SUB_OCTAVE ? 1u : 2u;
        ext[index] = (ext[index] & 127u) | (position >= 64u ? 128u : 0u);
    }
    return 1;
}
#endif

int original_sound_restore_miami(original_miami_program *out, const uint8_t cell[ORIGINAL_SOUND_BYTES])
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
int original_sound_restore_chicago(original_chicago_program *out, const uint8_t cell[ORIGINAL_SOUND_BYTES])
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
    if (cell[29] == ORIGINAL_SOUND_SYNTHONE && parameter >= ORIGINAL_SYNTHONE_BASE_COUNT) {
        *value = unit_control(original_sound_s1_display(cell,parameter));
        return 1;
    }
    float result;
    if (original_sound_edit_mask(cell) & (UINT32_C(1) << parameter)) {
        unsigned raw=cell[29]==ORIGINAL_SOUND_SYNTHONE ? original_sound_s1_display(cell,parameter) : cell[parameter];
        if (!original_sound_control_value(cell[29], parameter, raw, &result)) return 0;
    }
    else if (cell[29] == ORIGINAL_SOUND_SYNTHONE)
        result = parameter == ORIGINAL_SYNTHONE_PAN_RATE ? unit_control(ORIGINAL_SYNTHONE_PAN_RATE_INIT) : 0.f;
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

