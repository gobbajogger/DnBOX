/* SPDX-License-Identifier: GPL-3.0-only */
#pragma once
#include <stdint.h>
#include "../../../experiments/original_synth/chicago_program.h"
#include "../../../experiments/original_synth/miami_program.h"
#include "../dsp/x0x_param.h"

#define ORIGINAL_SOUND_BYTES 32u
#define ORIGINAL_SOUND_CHICAGO 0u
#define ORIGINAL_SOUND_MIAMI 1u
#define ORIGINAL_SOUND_SYNTHONE 2u
#define ORIGINAL_SOUND_JUNO 3u
#define ORIGINAL_SOUND_JUNO_ID UINT32_C(0x4f4a5531)
#define ORIGINAL_JUNO_NPARAMS 18u
#define ORIGINAL_JUNO_FACTORIES 4u
#define ORIGINAL_SOUND_CHICAGO_ID UINT32_C(0x4f434831)
#define ORIGINAL_SOUND_MIAMI_ID UINT32_C(0x4f4d4931)
#define ORIGINAL_SOUND_SYNTHONE_ID UINT32_C(0x4f533131)
#define ORIGINAL_SOUND_LEGACY_ODR1_ID UINT32_C(0x4f445231)
#define ORIGINAL_SOUND_SCHEMA 2u
#define ORIGINAL_CHICAGO_FACTORIES 81u
#define ORIGINAL_SYNTHONE_FACTORIES 1u
#define ORIGINAL_SYNTHONE_NPARAMS 13u
#define ORIGINAL_SYNTHONE_BASE_COUNT 6u
enum {
    ORIGINAL_SYNTHONE_TREM = ORIGINAL_SYNTHONE_BASE_COUNT,
    ORIGINAL_SYNTHONE_LFO1_SHAPE, ORIGINAL_SYNTHONE_LFO1_LEVEL, ORIGINAL_SYNTHONE_LFO1_RATE,
    ORIGINAL_SYNTHONE_LFO2_SHAPE, ORIGINAL_SYNTHONE_LFO2_LEVEL, ORIGINAL_SYNTHONE_LFO2_RATE
};
#define ORIGINAL_SYNTHONE_CRUSH 2u
#define ORIGINAL_SYNTHONE_WIDTH 3u
#define ORIGINAL_SYNTHONE_PAN 4u
#define ORIGINAL_SYNTHONE_PAN_RATE 5u
#define ORIGINAL_SYNTHONE_PAN_RATE_INIT 3u
#define ORIGINAL_SYNTHONE_INIT_FACTORY 0u
#define ORIGINAL_CHICAGO_FX_CHOICES 12u

/* Public control positions used by the physical performance wheels. */
enum { ORIGINAL_CHICAGO_CUTOFF = 2, ORIGINAL_CHICAGO_FX_TYPE = 17, ORIGINAL_CHICAGO_PITCH_BEND = 20,
       ORIGINAL_MIAMI_WOBBLE_SHAPE = 11, ORIGINAL_MIAMI_WOBBLE_DEPTH = 12, ORIGINAL_MIAMI_WOBBLE_RATE = 13,
       ORIGINAL_MIAMI_PITCH_BEND = 16 };

/* Performance macro IDs remain stable across patch-control additions. */
static inline int original_sound_is_performance(unsigned kind, unsigned parameter)
{
    return (kind == ORIGINAL_SOUND_CHICAGO &&
            (parameter == ORIGINAL_CHICAGO_PITCH_BEND || parameter == ORIGINAL_CHICAGO_PITCH_BEND + 1u)) ||
           (kind == ORIGINAL_SOUND_MIAMI &&
            (parameter == ORIGINAL_MIAMI_PITCH_BEND || parameter == ORIGINAL_MIAMI_PITCH_BEND + 1u)) ||
           (kind == ORIGINAL_SOUND_JUNO && (parameter == 16u || parameter == 17u)) ||
           (kind == ORIGINAL_SOUND_SYNTHONE && (parameter == 0u || parameter == 1u));
}

/* New firmware patch representation, not a legacy bank decoder:
 * [0..21] macro display bytes; [22..25] LE edit mask; [26..27] 'OS'; [28] version;
 * [29] engine kind; [30] selected factory index; [31] zero, or one for a
 * Synth One cell with stock effect edits, distinguishing historical cells.
 * Chicago indexes all 81 programs (the 55 compact ones first, then the 26
 * restored-insert ones) and FxType stores a twelve-choice index.
 * Kind 2 is Synth One. New writes use persistent id OS11. Historical id
 * 0x4F445231 and historical 8-parameter kind-2 cells load as the Synth One
 * init cell (factory 0); they are never written back. No song-layout change.
 * With X0X_SYNTHONE_PATCH enabled, bytes/bits 8..20 hold native INIT
 * edits in descriptor order. Stock refuses these edited cells. Untouched
 * cells retain their historical 32 bytes and exact INIT values.
 * The original engines still map to their original selectors; schema1 is refused.
 * Untouched controls come from the immutable binary32 factory data, never
 * their rounded display bytes. Edits represent the firmware's 7-bit controls.
 * Pitch bend and damper are performance-only. Older schema2 bytes/edit bits
 * remain valid storage, but patch restoration ignores them and reports neutral
 * bend0.5/damper0. Their native IDs/counts remain available to wheels and MIDI.
 * This stores a patch, not arbitrary AU floats or live voice/history state.
 * Every cell argument points to 32 accessible bytes. Source/output may alias;
 * failed validation or preparation leaves the destination unchanged. */
static inline unsigned original_sound_kind_from_id(uint32_t engine)
{
    if (engine == ORIGINAL_SOUND_MIAMI_ID) return ORIGINAL_SOUND_MIAMI;
    if (engine == ORIGINAL_SOUND_JUNO_ID) return ORIGINAL_SOUND_JUNO;
    if (engine == ORIGINAL_SOUND_SYNTHONE_ID || engine == ORIGINAL_SOUND_LEGACY_ODR1_ID)
        return ORIGINAL_SOUND_SYNTHONE;
    return ORIGINAL_SOUND_CHICAGO;
}
static inline int original_sound_is_legacy_slot2_id(uint32_t engine)
{
    return engine == ORIGINAL_SOUND_LEGACY_ODR1_ID;
}
int original_sound_is_legacy_slot2_cell(const uint8_t cell[ORIGINAL_SOUND_BYTES]);
/* Rewrites a historical kind-2 8-parameter cell to Synth One init. Returns 1 if
 * rewritten, 0 if the cell is already a current Synth One or other kind. */
int original_sound_adopt_legacy_cell(uint8_t cell[ORIGINAL_SOUND_BYTES]);
unsigned original_sound_count(unsigned kind);
unsigned original_sound_factories(unsigned kind);
/* Returns the immutable original program index, or -1 on invalid input. */
int original_sound_factory_source(unsigned kind, unsigned factory);
/* Convert a firmware control to its actual original macro. FxType is discrete;
 * Chicago ENV4 and Miami TUNE7 have exact midpoint64; all others use q/127.
 * Failed conversion does not change *value. */
int original_sound_control_value(unsigned kind, unsigned parameter, unsigned control, float *value);
static inline unsigned original_sound_bend_id(unsigned kind)
{ return kind == ORIGINAL_SOUND_SYNTHONE ? 0u : original_sound_count(kind) - 2u; }
const char *original_sound_fx_name(unsigned choice);
uint32_t original_sound_engine_id(unsigned kind);
const char *original_sound_name(unsigned kind, unsigned factory);
const x0x_param_t *original_sound_param(unsigned kind, unsigned parameter);
unsigned original_sound_s1_display(const uint8_t cell[ORIGINAL_SOUND_BYTES], unsigned parameter);
int original_sound_valid(const uint8_t cell[ORIGINAL_SOUND_BYTES]);
int original_sound_init(uint8_t cell[ORIGINAL_SOUND_BYTES], unsigned kind, unsigned factory);
int original_sound_set(uint8_t cell[ORIGINAL_SOUND_BYTES], unsigned parameter, unsigned value);
#if X0X_SYNTHONE_PATCH
int original_sound_s1_set(uint8_t cell[ORIGINAL_SOUND_BYTES], unsigned slot, unsigned position);
#endif
uint32_t original_sound_edit_mask(const uint8_t cell[ORIGINAL_SOUND_BYTES]);
int original_sound_restore_miami(original_miami_program *, const uint8_t cell[ORIGINAL_SOUND_BYTES]);
int original_sound_restore_chicago(original_chicago_program *, const uint8_t cell[ORIGINAL_SOUND_BYTES]);
int original_sound_value(const uint8_t cell[ORIGINAL_SOUND_BYTES], unsigned parameter, float *value);

