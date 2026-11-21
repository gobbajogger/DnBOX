/* SPDX-License-Identifier: GPL-3.0-only */
#pragma once
#include <stdint.h>
#include "../dsp/x0x_param.h"

/* Compile the native target with X0X_SYNTHONE_PATCH=1 to enable this INIT
 * patch interface. The ordinary firmware remains on its verified sound path.
 * Stable Synth One factory IDs; performance wheels belong to a separate API.
 * Storage slots are in descriptor order and must never be reordered. Existing
 * OS11 INIT and historical eight-control cells use no bits in this range. */
enum { S1_PATCH_FIRST_BYTE = 8, S1_PATCH_LEGACY_COUNT = 13, S1_PATCH_STEPS = 127 };
#if X0X_SYNTHONE_WIDEN
enum { S1_PATCH_COUNT = S1_PATCH_LEGACY_COUNT + 1 };
#else
enum { S1_PATCH_COUNT = S1_PATCH_LEGACY_COUNT };
#endif
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
} s1_patch_value;

/* Original 13 controls from a single render boundary. Values are in the same
 * order as s1_patch_at(), including the untouched factory precision. */
typedef struct {
    uint32_t revision;
    unsigned preset;
    uint8_t note_locked;
    float physical[S1_PATCH_LEGACY_COUNT]; /* stable 13-field caller ABI */
} s1_patch_snapshot;

const s1_patch_descriptor *s1_patch_at(unsigned index);
float s1_patch_physical(const s1_patch_descriptor *, unsigned position);
/* Nominal physical increment for one of the 127 equal edit intervals.
 * Endpoints are exact; intermediate binary32 values follow s1_patch_physical. */
static inline float s1_patch_increment(const s1_patch_descriptor *d)
{ return (d->maximum - d->minimum) / (float)S1_PATCH_STEPS; }
/* Value for the firmware's unit_value formatter. Uses real seconds and Hz. */
static inline float s1_patch_display(const s1_patch_descriptor *d, float physical)
{ return d->unit == X0X_U_PCT ? physical * 100.f : physical; }
