/* SPDX-License-Identifier: GPL-3.0-only */
#pragma once
#include "../dsp/s1_dry_voice.h"

enum { S1_CORE_VOICES = 6 };

typedef struct {
    s1_dry_voice voice[S1_CORE_VOICES];
    uint8_t note[S1_CORE_VOICES], down[S1_CORE_VOICES], damper, next;
    float bend;
} s1_dry_part;

void s1_part_init(s1_dry_part *part);
int s1_part_param(s1_dry_part *part, unsigned id, float value);
int s1_part_note(s1_dry_part *part, unsigned key, unsigned velocity);
int s1_part_active(const s1_dry_part *part);
void s1_part_render(s1_dry_part *part, float *left, float *right, unsigned frames);
