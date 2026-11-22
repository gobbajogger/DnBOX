/* SPDX-License-Identifier: GPL-3.0-only */
/* Six-voice Juno-style subtractive engine and one shared short-delay chorus.
 * Compact original equations informed by the Plaits analog oscillator/filter
 * design; no Plaits source is copied. */
#pragma once
#include <stdint.h>
#include "envelope_snapshot.h"
#if X0X_SUPERJ6_FACTORY
#include "dnbx_core.h"
/* The original app and the 18 legacy macros have different IDs for the
 * same numbers. Tag a decoded app ID before passing it to the runtime. */
#define JUNO6_APP_PARAM_BASE 256u
/* Qualified raw master-volume domain includes the authored over-unity stops. */
#define JUNO6_FACTORY_MASTER_MAX 1.32f
#endif
#define JUNO6_VOICES 6u
#define JUNO6_CHORUS_BYTES 512u
#define JUNO6_CHORUS_SAMPLES 256u
#if X0X_SUPERJ6_FACTORY
typedef struct {
 float selector, amount, enable;
} juno6_oscillator_edit;
#endif
typedef struct {
 float phase,increment,envelope,filter,filter_state,velocity;
 float subphase;
 uint8_t note,gate,stage,reserved;
 uint32_t age;
} juno6_voice_t;
typedef struct {
 juno6_voice_t voice[JUNO6_VOICES];
 uint32_t age;
 float sample_rate,attack,decay,sustain,release,cutoff,resonance,pulse_width,sub_level;
 uint8_t waveform, damper;
 float macro[18], bend, hpf, hp_state, hp_input, lfo_phase;
#if X0X_SUPERJ6_FACTORY
 dnbx_part core;
 /* An idle typed wave or AMP2 edit selects the app renderer until cold adoption. */
 uint8_t typed, factory_chorus_bypass;
 /* Main writes under the audio-source mask. The block boundary commits each
  * field once. Cold selection clears staged edits with the whole synth. */
 float pending_field[6]; /* AMP2 ADSR, output attenuation, AMP1 attack */
 float master_gain;
 juno6_oscillator_edit pending_wave[2];
 uint8_t pending_field_mask, pending_wave_mask;
 float typed_hp_state[2], typed_hp_input[2];
#endif
} juno6_t;
typedef struct {
 int16_t delay[JUNO6_CHORUS_SAMPLES];
 uint16_t write;
 uint32_t phase;
 float sample_rate,depth_ms,rate_hz,mix;
} juno6_chorus_t;
void juno6_init(juno6_t*,float);
int juno6_note_on(juno6_t*,uint8_t,uint8_t);
void juno6_note_off(juno6_t*,uint8_t);
void juno6_all_off(juno6_t*);
void juno6_render(juno6_t*,float*,unsigned);
void juno6_chorus_init(juno6_chorus_t*,float);
void juno6_chorus_set(juno6_chorus_t*,float,float,float);
void juno6_chorus_process(juno6_chorus_t*,float*,float*,unsigned);
/* Slot adapter: normalized patch macros; final two are performance-only. */
int juno6_set(juno6_t*, unsigned, float);
int juno6_active(const juno6_t*);

/* Caller-owned output must be disjoint from synth. Serialized observation;
 * not a concurrent audio/UI publication primitive. Physical seconds and
 * sustain fraction, no normalized control reinterpretation. One shared
 * amp/filter envelope. No elapsed progress or release-start level is stored. */

int juno6_read_envelope(const juno6_t *, juno6_envelope_snapshot *);
#if X0X_SUPERJ6_FACTORY
int juno6_render_stereo(juno6_t *, float *, float *, unsigned);
int juno6_can_edit_amp2(const juno6_t *);
void juno6_apply_pre_note_attack(juno6_t *, float);
void juno6_apply_pre_note_wave(juno6_t *, unsigned, const juno6_oscillator_edit *);
void juno6_apply_factory_waves(juno6_t *, const juno6_oscillator_edit waves[2]);
#endif
