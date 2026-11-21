/* SPDX-License-Identifier: GPL-3.0-only */
#pragma once
#include "../dsp/s1_dry_voice.h"
#if X0X_SYNTHONE_PATCH
#include "s1_patch.h"
#endif

enum { S1_CORE_VOICES = 6, S1_WIDTH_FIRST_FRAMES = 1365, S1_WIDTH_TAIL_BYTES = 2520 };

typedef struct {
    uint8_t *first;
    uint8_t tail[S1_WIDTH_TAIL_BYTES];
    unsigned position, pending;
    float target, applied;
} s1_stereo_width;

typedef struct {
    s1_dry_voice voice[S1_CORE_VOICES];
    uint8_t note[S1_CORE_VOICES], down[S1_CORE_VOICES], damper, next;
    float bend, crush_interval, crush_next, crush_age, crush_last;
#if X0X_SYNTHONE_PATCH
    s1_dry_controls patch;
#endif
    s1_stereo_width width;
} s1_dry_part;

/* Original bitCrushSampleRate is 2048..48000 Hz. Turning up the crush
 * amount lowers the clock from the INIT value to the minimum. */
static inline float s1_part_crush_hz(float amount) { return 48000.f - 45952.f * amount; }
void s1_part_init(s1_dry_part *part);
void s1_part_all_off(s1_dry_part *part);
void s1_part_bind_width(s1_dry_part *part, uint8_t *history);
#if X0X_SYNTHONE_PATCH
int s1_patch_slot(unsigned factory_id);
float s1_part_patch_value(const s1_dry_part *, unsigned slot);
void s1_part_apply_cell(s1_dry_part *, const uint8_t cell[32]);
#endif
int s1_part_param(s1_dry_part *part, unsigned id, float value);
int s1_part_note(s1_dry_part *part, unsigned key, unsigned velocity);
int s1_part_active(const s1_dry_part *part);
void s1_part_render(s1_dry_part *part, float *left, float *right, unsigned frames);
