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
#if X0X_SUPERJ6_BUILTIN_INIT
/* Authored build-9 IDs: 82, 83, 86, 210, 78, 211, 79. */
enum { SJ6_WAVE1, SJ6_WAVE2, SJ6_RANGE, SJ6_PWM1_ON, SJ6_PWM1, SJ6_PWM2_ON, SJ6_PWM2, SJ6_OSC_KNOBS,
       SJ6_AMP_ATTACK = SJ6_OSC_KNOBS, SJ6_AMP_DECAY, SJ6_AMP_SUSTAIN, SJ6_AMP_RELEASE,
       SJ6_FILTER_ATTACK, SJ6_FILTER_DECAY, SJ6_FILTER_SUSTAIN, SJ6_FILTER_RELEASE,
       SJ6_FILTER_LINK, SJ6_MASTER_LEVEL, SJ6_LFO1_RATE, SJ6_LFO2_RATE, SJ6_LFO3_RATE,
       SJ6_LFO2_PWM_DEPTH, SJ6_LFO2_PWM_ROUTE, SJ6_LFO1_MODE, SJ6_LFO2_MODE, SJ6_LFO3_MODE,
       SJ6_CHORUS_I, SJ6_CHORUS_II, SJ6_CHORUS_RATE, SJ6_CHORUS_MIX,
       SJ6_VIBRATO_DEPTH, SJ6_VIBRATO_RATE, SJ6_A_REFERENCE, SJ6_LFO2_SHAPE,
       SJ6_LAYER1_SEMI, SJ6_LAYER2_SEMI, SJ6_LAYER1_FINE, SJ6_LAYER2_FINE, SJ6_OSC2_LEVEL, SJ6_OSC1_LEVEL, SJ6_FILTER_VELOCITY, SJ6_KNOBS, SJ6_LFO2_PWM_BASE = SJ6_KNOBS };
enum { SJ6_LAYER1_SEMI_ID = 72, SJ6_LAYER2_SEMI_ID = 73, SJ6_SEMI_CENTER = 12, SJ6_SEMI_MAX = 24 };
enum { SJ6_LAYER1_FINE_ID = 74, SJ6_FINE_CENTER = 50, SJ6_FINE_MAX = 100 };
enum { SJ6_A_REFERENCE_ID = 206, SJ6_A_REFERENCE_STEPS = 100, SJ6_A_REFERENCE_DEFAULT = 60 };
enum { SJ6_LFO_COUNT = 3, SJ6_LFO_WORD_COUNT = 6, SJ6_LFO_DIVISIONS = 18 };
typedef struct {
    float word[SJ6_LFO_WORD_COUNT]; /* Rates, then Hz mode words, in index order. */
    uint8_t present;
} juno6_lfo_words;
typedef struct {
    float rate, mode, hz;
    uint64_t phase;
    uint8_t present; /* Bit 0 = rate, bit 1 = mode. */
    uint8_t division; /* Build-9 index, valid with both words and mode zero. */
} juno6_lfo_view;
enum { SJ6_ADSR_STAGES = 4, SJ6_EXTRA_COUNT = SJ6_LFO2_PWM_DEPTH - SJ6_OSC_KNOBS };
enum { SJ6_LFO2_PWM_DEPTH_MASK = 127, SJ6_LFO2_PWM_ROUTE_SHIFT = 7,
       SJ6_LFO2_PWM_ROUTE_MASK = 3u << SJ6_LFO2_PWM_ROUTE_SHIFT,
       SJ6_LFO2_PWM_SHAPE_SHIFT = 9 };
/* Build-9 normalized wave choices; internal sine is not selector ID98. */
enum { SJ6_LFO_TRI, SJ6_LFO_UP, SJ6_LFO_DOWN, SJ6_LFO_SQUARE, SJ6_LFO_HELD,
       SJ6_LFO2_SHAPE_MAX = SJ6_LFO_HELD, SJ6_LFO2_SHAPE_MASK = 7 };
enum { SJ6_WAVE_STOPS = 3, SJ6_RANGE_STOPS = 2, SJ6_PWM_STEPS = 127, SJ6_MASTER_INIT = 128 };
#endif
static inline int juno6_factory_range_valid(float raw)
{ return raw == 0.f || raw == .5f || raw == 1.f; }
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
#if defined(X0X_SJ6_BEND_TYPED_PROBE) && X0X_SJ6_BEND_TYPED_PROBE
 uint8_t pb_typed, pb_down, pb_up;
#endif
 float macro[18], bend, hpf, hp_state, hp_input, lfo_phase;
#if X0X_SUPERJ6_FACTORY
 dnbx_part core;
 /* An idle typed wave or AMP2 edit selects the app renderer until cold adoption. */
 uint8_t typed, amp1_independent; /* qualified by committed factory source, never inferred from raw words */
 /* Main writes under the audio-source mask. The block boundary commits each
  * field once. Cold selection clears staged edits with the whole synth. */
 float pending_field[7]; /* AMP2 0..3, master 4, independent AMP1 attack 5, oscillator range 6 */
 float master_gain;
 juno6_oscillator_edit pending_wave[2];
 uint8_t pending_field_mask, pending_wave_mask, pending_filter_link, osc2_level_code;
 float typed_hp_state[2], typed_hp_input[2];
#if X0X_SUPERJ6_BUILTIN_INIT
 uint64_t lfo_phase_q40[SJ6_LFO_COUNT];
 uint64_t lfo_step_q40[SJ6_LFO_COUNT];
 float lfo_tempo;
 uint16_t lfo2_pwm;
 uint8_t lfo_running, lfo_sync, lfo2_pwm_index, vibrato_left;
 uint32_t lfo2_pwm_until_tick, vibrato_phase, vibrato_step;
 float vibrato_depth, vibrato_ratio, mod_wheel_cutoff_hz;
 uint64_t lfo2_entropy_state;
 float lfo2_hold_elapsed, lfo2_hold_period, lfo2_hold_sample;
#if defined(X0X_HOST) && defined(X0X_SJ6_DELAY_WITNESS)
 struct {
    float *left,*right;
    float dry,from,target,feedback;
    unsigned tap, pos, elapsed, tick, capacity;
    float delay,delay_from,delay_target,fb_from,fb_target,bpm;
    unsigned time_raw,fb_raw,sync,clock_refused,delay_elapsed,fb_elapsed;
    unsigned enabled,mix,pending;
 } delay24;
#endif
 float lfo1_pwm_base, lfo1_cut_current, lfo1_cut_target;
 int32_t lfo1_cut_until;
 uint8_t lfo1_cut_active, lfo1_cut_force;
 uint32_t lfo1_pwm_until_tick;
#if defined(X0X_HOST) && defined(X0X_SJ6_ALT3_WITNESS)
 struct {
    float rate,depth,from_rate,from_depth,to_rate,to_depth;
    uint32_t phase,increment,elapsed,depth_elapsed,duration;
    uint8_t tick, armed, dirty, tempo_sync, last_wheel_code;
 } alt3;
#endif
 uint8_t osc1_level_code, mod_destination, mod_wheel_lfo1, mod_wheel_lfo2;
 uint8_t lfo1_pwm_route, lfo1_pwm_index, wheel_lfo1_amount; /* 0 untouched, 1..128 physical CC1 */
 uint8_t filter_velocity_position;
 float filter_velocity_scale;
#endif
#endif
#if defined(X0X_SJ6_MONO32_PROBE) && X0X_SJ6_MONO32_PROBE
 uint32_t mono32_keys[4];
 uint8_t mono32_on,legato33_on,glide35_detent;
#endif
} juno6_t;
#if X0X_SUPERJ6_FACTORY
enum { JUNO6_CHORUS_LEGACY, JUNO6_CHORUS_OFF, JUNO6_CHORUS_I, JUNO6_CHORUS_II, JUNO6_CHORUS_BOTH };
#endif
typedef struct {
 int16_t delay[JUNO6_CHORUS_SAMPLES];
 uint16_t write;
#if X0X_SUPERJ6_FACTORY
 /* Fits the existing alignment gap; pending is audio-boundary owned. */
 uint8_t factory_mode, factory_pending;
#endif
 uint32_t phase;
 float sample_rate,depth_ms,rate_hz,mix;
} juno6_chorus_t;
void juno6_init(juno6_t*,float);
#if defined(X0X_SJ6_MONO32_PROBE) && X0X_SJ6_MONO32_PROBE
int juno6_mono32_set(juno6_t*,unsigned);
int juno6_legato33_set(juno6_t*,unsigned);
int juno6_glide35_set(juno6_t*,unsigned);
#endif
int juno6_note_on(juno6_t*,uint8_t,uint8_t);
void juno6_note_off(juno6_t*,uint8_t);
void juno6_all_off(juno6_t*);
void juno6_render(juno6_t*,float*,unsigned);
void juno6_chorus_init(juno6_chorus_t*,float);
void juno6_chorus_set(juno6_chorus_t*,float,float,float);
void juno6_chorus_process(juno6_chorus_t*,float*,float*,unsigned);
/* Slot adapter: normalized patch macros; final two are performance-only. */
int juno6_set(juno6_t*, unsigned, float);
#if X0X_SUPERJ6_BUILTIN_INIT
float juno6_sj6_hpf_hz(float unit);
void juno6_set_sj6_hpf(juno6_t *, float unit);
#endif
int juno6_active(const juno6_t*);

/* Caller-owned output must be disjoint from synth. Serialized observation;
 * not a concurrent audio/UI publication primitive. Physical seconds and
 * sustain fraction, no normalized control reinterpretation. One shared
 * amp/filter envelope. No elapsed progress or release-start level is stored. */

int juno6_read_envelope(const juno6_t *, juno6_envelope_snapshot *);
#if X0X_SUPERJ6_FACTORY
int juno6_render_stereo(juno6_t *, float *, float *, unsigned);
int juno6_can_edit_amp2(const juno6_t *);
#if X0X_SUPERJ6_BUILTIN_INIT
unsigned juno6_knob_position(const juno6_t *s, unsigned slot);
void juno6_apply_knobs(juno6_t *s, unsigned mask, const uint8_t positions[SJ6_OSC_KNOBS]);
void juno6_apply_panel_envelopes(juno6_t *, unsigned mask, const uint8_t positions[SJ6_EXTRA_COUNT], int reset);
void juno6_apply_panel_master(juno6_t *, unsigned position);
/* Transient Build-9 default MOD destination, LFO1 RATE. Audio owner only. */
#if defined(X0X_SJ6_BEND_TYPED_PROBE) && X0X_SJ6_BEND_TYPED_PROBE
int juno6_set_bend_ranges(juno6_t *, unsigned down, unsigned up);
#endif
int juno6_set_mod_wheel(juno6_t *, unsigned value);
/* ID179 original editor index; only source-qualified receivers are enabled. */
int juno6_set_mod_destination(juno6_t *, unsigned selected);
#if defined(X0X_HOST) && defined(X0X_SJ6_ALT3_WITNESS)
int juno6_arm_alt3_running(juno6_t *,unsigned verified_ramp_frames,int tempo_sync);
int juno6_arm_alt3_free(juno6_t *,unsigned verified_ramp_frames,int tempo_sync);
void juno6_alt3_process(juno6_t *,float *,float *,unsigned);
#endif
/* Qualified original continuous Triangle LFO1 route only (selector 69/70).
 * Amount and base are authored normalized Float; No save or UI transport. */
int juno6_configure_lfo1_pwm(juno6_t *,unsigned selector,unsigned enabled,float base,float amount);
/* Bounded source ID181=1 (normal ID7 cutoff), Loop/Triangle only. */
int juno6_configure_lfo1_cutoff(juno6_t *,float authored_base,float live_current,float amount);
float juno6_mod_cutoff_hz(unsigned value);
#if defined(X0X_HOST) && defined(X0X_SJ6_DELAY_WITNESS)
enum { SJ6_DELAY24=90, SJ6_DELAY27=91, SJ6_DELAY25=92, SJ6_DELAY26=93, SJ6_DELAYSYNC=94 };
int juno6_delay24_arm(juno6_t *,float *,float *,unsigned,float,unsigned);
int juno6_delay24_set(juno6_t *,unsigned,unsigned);
int juno6_delay25_can(const juno6_t *,unsigned,float,unsigned);
int juno6_delay25_profile(juno6_t *,unsigned,float,unsigned);
int juno6_delay25_clock(juno6_t *,float,unsigned);
unsigned juno6_delay_position(const juno6_t *,unsigned);
void juno6_delay24_process(juno6_t *,float *,float *,unsigned);
#endif
void juno6_set_lfo_words(juno6_t *, const juno6_lfo_words *, int reset_phase);
void juno6_set_panel_lfo(juno6_t *, unsigned lfo, int position, unsigned hz_mode);
void juno6_set_lfo_tempo(juno6_t *, float bpm);
static inline void juno6_set_panel_lfo_rate(juno6_t *s, unsigned lfo, unsigned position)
{ juno6_set_panel_lfo(s, lfo, (int)position, 1); }
void juno6_read_lfo(const juno6_t *, unsigned lfo, juno6_lfo_view *);
void juno6_set_lfo2_pwm(juno6_t *, uint16_t setting);
void juno6_set_lfo2_shape(juno6_t *, unsigned shape);
void juno6_set_vibrato(juno6_t *, float depth_unit, float rate_unit, int reset);
/* Binary32 unit interval, including an authored negative zero. */
static inline int juno6_unit_word_valid(uint32_t bits)
{ return bits <= UINT32_C(0x3f800000) || bits == UINT32_C(0x80000000); }
int juno6_set_filter_velocity(juno6_t *, unsigned position);
void juno6_set_vibrato_words(juno6_t *, const float values[2], unsigned present, int reset);
unsigned juno6_layer_fine(const juno6_t *, unsigned layer);
void juno6_set_layer_fine(juno6_t *, unsigned layer, unsigned position);
unsigned juno6_layer_semi(const juno6_t *, unsigned layer);
void juno6_set_layer_semi(juno6_t *, unsigned layer, unsigned position);
static inline unsigned juno6_layer1_semi(const juno6_t *s) { return juno6_layer_semi(s, 0); }
static inline void juno6_set_layer1_semi(juno6_t *s, unsigned position)
{ juno6_set_layer_semi(s, 0, position); }
void juno6_set_a_reference(juno6_t *, float normalized, unsigned present);
float juno6_a_reference_unit(unsigned position);
static inline float juno6_a_reference_hz(float normalized)
{ return 200.f + 400.f * normalized; }
int juno6_read_panel_envelope(const juno6_t *, unsigned filter, juno6_envelope_snapshot *, unsigned *linked);
#endif
void juno6_apply_pre_note_wave(juno6_t *, unsigned, const juno6_oscillator_edit *);
void juno6_apply_factory_waves(juno6_t *, const juno6_oscillator_edit waves[2]);
/* Audio boundary, idle, with already-qualified independent envelopes. */
/* Prevalidated idle only, negative internal sentinel means absent. */
void juno6_apply_pre_note_range(juno6_t*,float);
int juno6_apply_independent_amp1_attack(juno6_t *, float);
#endif
