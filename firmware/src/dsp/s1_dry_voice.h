/* SPDX-License-Identifier: GPL-3.0-only */
#ifndef S1_DRY_VOICE_H
#define S1_DRY_VOICE_H

#include "dnbx_s1_envelope.h"
#include <stdint.h>
#if X0X_SYNTHONE_PATCH
#include "s1_sub_osc.h"
#endif

/* Mono Synth One dry voice. Each instance owns its envelope and filter history. */
#define S1_DRY_SAMPLE_RATE 44100.0f
#define S1_DRY_MAX_BLOCK 64u

typedef struct {
    float attack_s, decay_s, sustain, release_s;
} s1_dry_adsr;

typedef struct {
    s1_dry_adsr amp, filter;
    float cutoff_hz, resonance, filter_depth;
    float oscillator1_level, balance;
#if X0X_SYNTHONE_PATCH
    float sub_level, sub_extra_octave, sub_square;
#endif
} s1_dry_controls;

typedef struct {
    /* The envelopes consume the prepared retentions and sustain below. Keep
     * their physical ADSR inputs only in the editable patch. */
    float cutoff_hz, resonance, filter_depth, oscillator1_level, balance;
    dnbx_s1_envelope_params amp, filter;
    float cached_cutoff_hz, pole_retention, feedback;
#if X0X_SYNTHONE_PATCH
    float sub_level;
    uint8_t sub_extra_octave, sub_square;
#endif
} s1_dry_prepared;

typedef struct {
    s1_dry_prepared prepared;
    dnbx_s1_envelope_state amp, filter;
    float phase, phase_increment, velocity_squared;
    float filter_history[5];
    uint8_t gate, started, configured, reserved;
#if X0X_SYNTHONE_PATCH
    s1_sub_osc sub;
    float sub_base_hz;
#endif
} s1_dry_voice;

enum { S1_DRY_OK = 0, S1_DRY_ARGUMENT = -1, S1_DRY_RANGE = -2,
       S1_DRY_UNSUPPORTED = -3 };

void s1_dry_reset(s1_dry_voice *voice);
void s1_dry_frozen_controls(s1_dry_controls *controls);
/* Configure outside render. Invalid input leaves the object unchanged. */
int s1_dry_configure(s1_dry_voice *voice, const s1_dry_controls *controls);
/* One fresh note per reset. The frequency is supplied by the numeric packet;
 * phase is explicit and is never fitted to recordings. Velocity zero is off. */
int s1_dry_start(s1_dry_voice *voice, float frequency_hz,
                 unsigned velocity, float phase);
#if X0X_SYNTHONE_PATCH
void s1_dry_pitch(s1_dry_voice *voice, float frequency_hz);
#endif
int s1_dry_note_off(s1_dry_voice *voice);
/* Original release ends at a render boundary when amplitude falls below 0.01. */
void s1_dry_begin_block(s1_dry_voice *voice);
float s1_dry_sample(s1_dry_voice *voice);
/* Add one active voice after the caller has applied its block boundary. */
void s1_dry_mix(s1_dry_voice *voice, float *mix, unsigned frames);
int s1_dry_render(s1_dry_voice *voice, float *mono, unsigned frames);
/* Exposed physical control law for numerical validation, not a control ID API. */
float s1_dry_cutoff(float base_hz, float depth, float envelope);

#endif
