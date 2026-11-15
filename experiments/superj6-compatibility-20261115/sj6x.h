/* SPDX-License-Identifier: GPL-3.0-only */
/* ORIGINAL experimental feature proxy. Not AudioKit/SuperJ6 DSP or firmware. */
#ifndef SJ6X_H
#define SJ6X_H
#include <stdint.h>
#define SJ6X_VOICES 6u
#define SJ6X_BLOCK_MAX 4096u
#define SJ6X_PWM 1u
#define SJ6X_PITCH 2u
#define SJ6X_CUTOFF 4u
#define SJ6X_AMP 8u
typedef struct {float attack,decay,sustain,release;} sj6x_adsr;
typedef struct {float hz,depth;unsigned routes;} sj6x_lfo;
typedef struct {
 sj6x_adsr envelope[3]; /* amp1, amp2, filter; no link-flag interpretation */
 float saw,pulse,sub,noise,level,cutoff_hz,resonance,filter_env_hz,highpass_hz;
 float pulse_width[2],detune_cents[2],layer_mix[2];
 sj6x_lfo lfo[3]; /* named manual target routes; NOT app numeric route codes */
} sj6x_patch;
typedef struct {float value,release_step;unsigned stage;} sj6x_envelope;
typedef struct {
 float phase[2],subphase,hz,velocity,filter_a,filter_b;
 sj6x_envelope envelope[3];
 unsigned note,gate;
} sj6x_voice;
typedef struct {
 sj6x_patch patch;sj6x_voice voice[SJ6X_VOICES];
 float sample_rate,lfo_phase[3],hp_input,hp_state;
 uint32_t noise_state;unsigned next_voice,tag;
} sj6x_engine;
void sj6x_default_patch(sj6x_patch*);
int sj6x_init(sj6x_engine*,float);
int sj6x_apply(sj6x_engine*,const sj6x_patch*);
int sj6x_note_on(sj6x_engine*,unsigned,unsigned);
int sj6x_note_off(sj6x_engine*,unsigned);
int sj6x_all_off(sj6x_engine*);
int sj6x_render(sj6x_engine*,float*,unsigned);
/* Caller owns aligned initialized state; do not forge/mutate it concurrently.
 * Patch is fully validated before apply and may be the engine's own patch.
 * Render output must be aligned, mapped, writable and disjoint from the whole
 * state. All pointers remain live for each call. Bad API arguments return 0
 * before mutation. This is not hostile-memory/mappedness or app sound proof.
 * Apply preserves active envelope trajectories and original app bytes are
 * never stored here. Mono/stereo/FX/ML/model enums/arp/sequencer unsupported. */
#endif
