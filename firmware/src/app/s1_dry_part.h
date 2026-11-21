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
    float phase, increment, smooth, level;
    uint8_t shape;
} s1_trem_lfo;

enum { S1_MASTER_CONTROLS = 6 };
typedef struct {
    float target_volume, volume, attack, release, gain_smooth, ratio_scale, threshold, linear_threshold, makeup;
    float detect[2], gain_db[2];
    uint8_t knobs[S1_MASTER_CONTROLS], active;
} s1_master;
void s1_master_release(s1_master *,unsigned slot,unsigned default_pot);
void s1_master_targets(s1_master *,const float values[S1_MASTER_CONTROLS]);

typedef struct { float z1[4], z2[4], feedback; } s1_phaser_channel;
typedef struct {
    s1_phaser_channel channel[2];
    float target[4], current[4], sine, cosine;
    unsigned started;
} s1_phaser;

typedef struct {
    s1_dry_voice voice[S1_CORE_VOICES];
    uint8_t note[S1_CORE_VOICES], down[S1_CORE_VOICES], damper, next;
    float bend, crush_interval, crush_next, crush_age, crush_last;
#if X0X_SYNTHONE_PATCH
    s1_dry_controls patch;
#endif
    s1_stereo_width width;
    struct {
        uint32_t phase;
        float rate, rate_target, depth, depth_target;
        uint8_t engaged, mix;  /* gain-law transition in 64 samples */
    } pan;
    s1_trem_lfo lfo[2];
    uint8_t tremolo;
    s1_phaser phaser;
    s1_master master;
} s1_dry_part;

/* Original bitCrushSampleRate is 2048..48000 Hz. Turning up the crush
 * amount lowers the clock from the INIT value to the minimum. */
static inline float s1_part_crush_hz(float amount) { return 48000.f - 45952.f * amount; }
/* Three low-rate steps reach the original 0.25 Hz INIT exactly. */
static inline float s1_pan_hz(unsigned position)
{
    const float minimum = 1.f / 1920.f;
    if (position <= 3) return minimum + (0.25f - minimum) * (float)position / 3.f;
    return 0.25f + (float)(position - 3u) * (9.75f / 124.f);
}
float s1_phaser_control(unsigned slot, unsigned position);
float s1_lfo_hz(unsigned position);
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
int s1_part_effect_tail(const s1_dry_part *part);
void s1_part_render(s1_dry_part *part, float *left, float *right, unsigned frames);
