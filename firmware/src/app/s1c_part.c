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
#include <string.h>

#define S1C_SR 44100.0f
/* Synth One factory parameter ids (s1c uses the same numbering). */
enum { P_MONO = 20, P_DETM = 31, P_BCSR = 34 };

const uint8_t *s1_part_pack;
uint32_t s1_part_pack_bytes;

_Static_assert(S1C_RVB_BYTES % 4u == 0u && S1C_RVB_BYTES + 1024u <= S1C_ARENA_BYTES,
               "reverb lines leave at least 1 KiB of delay pool in the Chicago FX2 arena");

/* Effects need the same behaviour, not an exact match: one fixed reverb fit for every program
 * (no per-program table). Delay lines are half floats. */
static void fit_apply(s1c *e)
{
    e->rfit[0] = 0.5f; e->rfit[1] = 1.5f; e->rfit[2] = 1.0f; e->dfmt = 0;
}

static void emit(void *context, unsigned id, float physical) { (void)s1c_front_set((s1c *)context, id, physical); }

static void load_init(s1c *e)
{
    s1c_init(e, S1C_SR, NULL);
    for (unsigned i = 0; i < 150u; ++i) (void)s1c_front_set(e, i, s1c_front_init(i));
}

unsigned s1_part_programs(void)
{
    const uint8_t *p = s1_part_pack;
    if (!p || s1_part_pack_bytes < S1PACK_HEADER) return 0;
    unsigned n = s1pack_programs(p, s1_part_pack_bytes);
    if (!n || x0x_crc32(p + S1PACK_HEADER, s1pack_total(p) - S1PACK_HEADER) != s1pack_crc(p)) return 0;
    return n;
}

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

int s1_part_program(s1_dry_part *part, unsigned program)
{
    s1c *e = &part->e;
    int ok = 1;
    if (program != S1_PART_INIT && program >= s1_part_programs()) { program = S1_PART_INIT; ok = 0; }
    load_init(e);
    if (program != S1_PART_INIT) {
        char name[S1PACK_NAME_MAX];
        if (s1pack_read(s1_part_pack, program, name, emit, e) < 0) { load_init(e); program = S1_PART_INIT; ok = 0; }
    }
    fit_apply(e);
    /* The engine clears its voices when mono changes; a program load is not a change. */
    e->mono_prev = (uint8_t)e->t[P_MONO];
    part->program = (uint16_t)program;
    part->detm = e->t[P_DETM];
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

const s1_patch_descriptor *s1_patch_at(unsigned slot)
{ return slot < S1_PATCH_COUNT ? &patch_parameters[slot] : NULL; }

int s1_patch_slot(unsigned id)
{
    for (unsigned i = 0; i < S1_PATCH_COUNT; ++i)
        if (patch_parameters[i].factory_id == id) return (int)i;
    return -1;
}

float s1_patch_physical(const s1_patch_descriptor *d, unsigned position)
{
    if (position >= S1_PATCH_STEPS) return d->maximum;
    return d->minimum + (d->maximum - d->minimum) * ((float)position / S1_PATCH_STEPS);
}

float s1_part_patch_value(const s1_dry_part *part, unsigned slot)
{
    return slot < S1_PATCH_COUNT ? part->e.t[patch_parameters[slot].factory_id] : 0.0f;
}

void s1_part_apply_cell(s1_dry_part *part, const uint8_t cell[32])
{
    uint32_t mask = original_sound_edit_mask(cell);
    for (unsigned i = 0; i < S1_PATCH_COUNT; ++i) {
        unsigned byte = S1_PATCH_FIRST_BYTE + i;
        if (mask & (UINT32_C(1) << byte))
            (void)s1c_front_set(&part->e, patch_parameters[i].factory_id, s1_patch_physical(&patch_parameters[i], cell[byte]));
    }
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

int s1_part_voice_view(const s1_dry_part *part, float *level, float *modulation, unsigned *state, unsigned *key_down)
{
    int sel = -1;
    for (unsigned i = 0; i < S1C_VOICES; ++i) {
        const s1c_voice *v = &part->e.v[i];
        if (v->root != -1 && v->stage && (sel < 0 || v->amp > part->e.v[sel].amp)) sel = (int)i;
    }
    if (sel < 0) return 0;
    const s1c_voice *v = &part->e.v[sel];
    *level = v->amp;
    *modulation = v->fenv;
    *state = v->stage == 2 ? 3u : v->ea.mode == 1 ? 0u : 1u;
    *key_down = v->stage == 1 && !(part->held & (1u << sel));
    return 1;
}

int s1_part_env_view(const s1_dry_part *part, unsigned filter, float *level, unsigned *stage, unsigned *key_down)
{
    int sel = -1;
    for (unsigned i = 0; i < S1C_VOICES; ++i) {
        const s1c_voice *v = &part->e.v[i];
        if (v->root != -1 && v->stage && (sel < 0 || v->ea.y > part->e.v[sel].ea.y)) sel = (int)i;
    }
    if (sel < 0) return 0;
    const s1c_voice *v = &part->e.v[sel];
    const s1c_env *e = filter ? &v->ef : &v->ea;
    *level = e->y;
    *stage = e->mode;            /* E_CLEAR 0, E_ATK 1, E_DEC 2, E_REL 3 */
    *key_down = v->stage == 1 && !(part->held & (1u << sel));
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
