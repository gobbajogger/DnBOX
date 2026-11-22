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
enum {
    S1_PATCH_FIRST_BYTE = 8, S1_PATCH_CELL_COUNT = 13, S1_PATCH_STEPS = 127,
    S1_PATCH_SUB_LEVEL = S1_PATCH_CELL_COUNT, S1_PATCH_SUB_OCTAVE, S1_PATCH_SUB_SQUARE,
    S1_PATCH_FILTER_TYPE, S1_PATCH_COUNT
};
#define S1_PATCH_MASK (((UINT32_C(1) << S1_PATCH_CELL_COUNT) - 1) << S1_PATCH_FIRST_BYTE)

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
} s1_patch_value;

/* Whole audible patch from a single render boundary. Values are in the same
 * order as s1_patch_at(), including the untouched factory precision. */
typedef struct {
    uint32_t revision;
    unsigned preset;
    uint8_t note_locked;
    float physical[S1_PATCH_COUNT];
} s1_patch_snapshot;

const s1_patch_descriptor *s1_patch_at(unsigned index);
float s1_patch_physical(const s1_patch_descriptor *, unsigned position);
/* Value for the firmware's unit_value formatter. Uses real seconds and Hz. */
static inline float s1_patch_display(const s1_patch_descriptor *d, float physical)
{ return d->unit == X0X_U_PCT ? physical * 100.f : physical; }
