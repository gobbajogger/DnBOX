/* SPDX-License-Identifier: GPL-3.0-only */
#ifndef DNBX_CORE_H
#define DNBX_CORE_H
#include <stddef.h>
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif

/* Disposable analytic DnBOX candidate. This is not the SuperJ6 app ABI.
 * All storage is caller owned; calls on one instance must be serialized.
 * Input events take effect before the next rendered sample. */
/* Default diagnostic build retains render counters. The compact hardware
 * profile must explicitly pass -DDNBX_DIAGNOSTIC_COUNTERS=0. */
#ifndef DNBX_DIAGNOSTIC_COUNTERS
#define DNBX_DIAGNOSTIC_COUNTERS 1
#endif
#if DNBX_DIAGNOSTIC_COUNTERS != 0 && DNBX_DIAGNOSTIC_COUNTERS != 1
#error DNBX_DIAGNOSTIC_COUNTERS_must_be_zero_or_one
#endif
#define DNBX_ABI_MAJOR UINT32_C(2)
#define DNBX_ABI_MINOR UINT32_C(0)
#define DNBX_ABI_VERSION ((DNBX_ABI_MAJOR << 16) | DNBX_ABI_MINOR)
typedef char dnbx_abi_version_value_check[(DNBX_ABI_VERSION == UINT32_C(131072)) ? 1 : -1];
#define DNBX_VOICES 6u
#define DNBX_LAYERS 2u
#define DNBX_RAW_CONTROLS 218u
#define DNBX_MAX_FRAMES 1024u
#define DNBX_ENVELOPE_TICK 16u
#define DNBX_OUTPUT_ABS_BOUND 32.0f

enum { DNBX_OK = 0, DNBX_E_ARGUMENT = -1, DNBX_E_RANGE = -2,
       DNBX_E_UNINITIALIZED = -3, DNBX_E_ABSENT = -4, DNBX_E_UNSUPPORTED = -5,
       DNBX_NOTE_OFF_ONLY = 6 };
enum { DNBX_RAW_STORED_ONLY = 1, DNBX_RAW_RESERVED = 2,
       DNBX_RAW_EXTERNALLY_OWNED = 3 };
enum { DNBX_VELOCITY_LINEAR = 0, DNBX_VELOCITY_SQUARED = 1 };
enum { DNBX_OSC_STOCK_BLEP = 0, DNBX_OSC_RAW = 1, DNBX_OSC_SUPERJ6 = 2 };
enum { DNBX_ENV_IDLE, DNBX_ENV_ATTACK, DNBX_ENV_HOLD,
       DNBX_ENV_DECAY, DNBX_ENV_SUSTAIN,
       DNBX_ENV_RELEASE_HOLD, DNBX_ENV_RELEASE };

typedef struct {
    /* Times are 0..16 s. SuperJ6 AMP decay and release allow 0..24 s.
     * Sustain is a fraction 0..1.
     * Zero stages settle immediately except: a completed positive-duration
     * attack spends one 16-frame hold tick when hold_s is zero, matching the
     * supplied amp observations. A zero attack has no such minimum hold. */
    float attack_s, hold_s, decay_s, sustain, release_hold_s, release_s;
} dnbx_envelope_params;

typedef struct {
    dnbx_envelope_params amp, filter;
    float waveform;          /* 0 sine, 1/3 triangle, 2/3 saw, 1 square;
                             * linear crossfade between adjacent stops. */
    float pulse_width;       /* Physical duty fraction .03 .. .5. */
    float gain;              /* Explicit linear gain 0 .. 4, NOT an app law. */
    float sub_gain;          /* Linear half-frequency square gain 0 .. .5. */
    float cutoff_hz;         /* 20 .. 20000 Hz, bounded stock filter equation. */
    float resonance;         /* 0 .. 1, stock feedback multiplier .8. */
    float filter_env_hz;     /* Signed cutoff excursion -20000 .. 20000 Hz. */
    float lfo_hz;            /* Physical frequency 0 .. 16 Hz; zero freezes. */
    float lfo_pwm_depth;     /* Duty excursion 0 .. .47, final duty .03 .. .5. */
    float tune_semitones;    /* -48 .. 48, pitch increments cap at .45 cycles/sample. */
    float pan;              /* -1 left, 0 equal half gains, +1 right. */
    uint32_t oscillator_mode; /* Stock BLEP, raw, or a discrete factory oscillator. */
} dnbx_layer_params;

/* Public layout supports static allocation; fields below are implementation
 * state and must not be modified by a caller. No pointers or global state. */
typedef struct {
    float level, curve, delta, asymptote, step_rate, endpoint, correction;
    uint32_t remaining;
    uint8_t stage, tick_left, timed_stage, reserved;
} dnbx_envelope_state;
typedef struct {
    float phase, increment, subphase, filter_first, filter_second;
    float oscillator_history[4];
    dnbx_envelope_state amp, filter;
} dnbx_layer_voice;
typedef struct {
    dnbx_layer_voice layer[DNBX_LAYERS];
    float velocity;
    uint8_t note, gate, age_rank, pedal_held;
} dnbx_voice;
typedef struct {
    dnbx_layer_params params;
    float lfo_phase;
} dnbx_layer;
typedef struct {
    uint32_t magic;
    float sample_rate;
    dnbx_layer layer[DNBX_LAYERS];
    dnbx_voice voice[DNBX_VOICES];
    float raw[DNBX_RAW_CONTROLS];
    uint32_t raw_present[(DNBX_RAW_CONTROLS + 31u) / 32u];
    uint8_t damper, velocity_mode, superj6_amp_range, reserved;
} dnbx_part;

/* Optional caller-owned render-only semantic counters. They count dnbx_render
 * work, excluding all note/control/initialization pathways. They accumulate
 * (no automatic reset), contain no timing measurements, and are excluded from
 * part size. Counts are not native CPU cycles or instructions. Full selected
 * event/control work is measured by the separate test-only trace harness. */
typedef struct {
    uint64_t frames, voice_slots, active_voice_samples, layer_samples;
    uint64_t envelope_samples, envelope_ticks, stage_transitions;
    uint64_t exp2_calls, log2_calls, small_exp_calls, lfo_samples, blep_calls, sine_calls;
    uint32_t maximum_transition_chain, reserved;
} dnbx_work;

typedef struct {
    float amp[DNBX_LAYERS], filter[DNBX_LAYERS], phase[DNBX_LAYERS];
    uint8_t amp_stage[DNBX_LAYERS], filter_stage[DNBX_LAYERS];
    uint8_t note, gate, age_rank, active;
} dnbx_voice_snapshot;

uint32_t dnbx_abi_version(void);
/* Returns1 when render counters are available, otherwise0. */
uint32_t dnbx_diagnostic_counters_enabled(void);
size_t dnbx_part_size(void);
size_t dnbx_layer_params_size(void);
size_t dnbx_work_size(void);
void dnbx_default_layer_params(dnbx_layer_params *);
int dnbx_init(dnbx_part *, float sample_rate);
int dnbx_init_superj6(dnbx_part *, float sample_rate); /* 8000 .. 192000 inclusive. */
int dnbx_set_layer(dnbx_part *, unsigned layer, const dnbx_layer_params *);
int dnbx_get_layer(const dnbx_part *, unsigned layer, dnbx_layer_params *);
/* Positive velocity returns chosen voice index 0..5. Velocity 0 performs
 * note-off and returns DNBX_NOTE_OFF_ONLY. MIDI values outside 0..127 fail. */
int dnbx_note_on(dnbx_part *, unsigned note, unsigned velocity);
int dnbx_note_off(dnbx_part *, unsigned note);
int dnbx_set_sustain(dnbx_part *, unsigned down); /* Strict boolean 0 or 1. */
/* Applies to new and existing notes, from the stored linear velocity. */
int dnbx_set_velocity_mode(dnbx_part *, unsigned mode);
int dnbx_all_off(dnbx_part *);
int dnbx_active_voices(const dnbx_part *);
int dnbx_read_voice(const dnbx_part *, unsigned voice, dnbx_voice_snapshot *);
/* Overwrites separate L/R arrays of at least frames floats. Arrays must be
 * disjoint from each other, part, and work. 0..1024 frames per call; a zero
 * call accepts null arrays and advances nothing. Checked argument failures do
 * not mutate state or output. Buffer capacity and partial/state/work overlap
 * are caller preconditions, not checked (only null/exact L=R is rejected).
 * A build with diagnostic counters disabled requires work==NULL, including
 * zero-frame calls; nonnull work returns DNBX_E_UNSUPPORTED before mutation.
 * No allocations, locks, variable-bound retry, or I/O. */
int dnbx_render(dnbx_part *, float *left, float *right, unsigned frames,
                dnbx_work *work);

/* Lossless finite raw admission, deliberately separate from physical DSP.
 * NONE of the 218 app control IDs is automatically applied to the DSP.
 * Gain laws and selector dictionaries are not inferred. 207 is reserved;
 * 212..214 are externally owned key-lock metadata. All retain exact float
 * bits and return a status showing that no DSP behavior was applied. */
int dnbx_store_raw(dnbx_part *, unsigned app_id, float raw);
int dnbx_read_raw(const dnbx_part *, unsigned app_id, float *raw);
/* Independently documented scalar laws, not a preset/application binding. */
int dnbx_attack_from_unit(float unit, float *seconds);
int dnbx_lfo_from_unit(float unit, float *hz);
int dnbx_pwm_from_unit(float unit, float *duty);
int dnbx_range_from_stop(float stop, float *ratio);
/* Stateless probes support stock BLEP and raw. Factory shapes use voice history. */
int dnbx_wave_probe(float phase, float increment, float waveform,
                    float pulse_width, float *sample);
int dnbx_wave_probe_mode(float phase, float increment, float waveform,
                         float pulse_width, unsigned mode, float *sample);

#ifdef __cplusplus
}
#endif
#endif
