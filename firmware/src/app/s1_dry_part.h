/* SPDX-License-Identifier: GPL-3.0-only */
#pragma once
#include "../dsp/s1_dry_voice.h"
#if X0X_SYNTHONE_PATCH
#include "s1_patch.h"
#endif

enum { S1_CORE_VOICES = 6 };

typedef struct {
    s1_dry_voice voice[S1_CORE_VOICES];
    uint8_t note[S1_CORE_VOICES], down[S1_CORE_VOICES], damper, next;
    float bend;
#if X0X_SYNTHONE_PATCH
    s1_dry_controls patch;
#if X0X_SYNTHONE_WIDEN
    uint8_t widen_tail[2520];
    unsigned widen_frame;
    float widen_target, widen_current;
#endif
#endif
} s1_dry_part;

void s1_part_init(s1_dry_part *part);
#if X0X_SYNTHONE_PATCH
void s1_part_all_off(s1_dry_part *part);
int s1_patch_slot(unsigned factory_id);
float s1_part_patch_value(const s1_dry_part *, unsigned slot);
void s1_part_apply_cell(s1_dry_part *, const uint8_t cell[32]);
#endif
int s1_part_param(s1_dry_part *part, unsigned id, float value);
int s1_part_note(s1_dry_part *part, unsigned key, unsigned velocity);
int s1_part_active(const s1_dry_part *part);
void s1_part_render(s1_dry_part *part, float *left, float *right, unsigned frames);
