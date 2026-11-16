/* SPDX-License-Identifier: GPL-3.0-only */
/* Live SuperJ6 voice. Compiled only when X0X_SUPERJ6=1.
 * Six analytic voices (the Juno polyphony used by the compatibility proxy)
 * plus one resynthesis slot. State is caller-owned; the firmware keeps it
 * in the pool. No heap, no libm. */
#ifndef SJ6_ENGINE_H
#define SJ6_ENGINE_H
#include <stdint.h>
#include "sj6_analytic.h"
#include "sj6_blocks.h"
#include "sj6_resynth.h"
#ifdef __cplusplus
extern "C" {
#endif

enum { SJ6_POLY = 6, SJ6_CTRLS = 218 };
/* engine_sj6_event kinds (firmware/src/app/engine.c, X0X_SUPERJ6=1 only). */
enum { SJ6_EV_SELECT = 1, SJ6_EV_NOTE_ON, SJ6_EV_NOTE_OFF, SJ6_EV_RESYNTH };
int engine_sj6_event(int kind, int a, int b);
int engine_sj6_preset(const uint16_t *ids, const float *vals, unsigned n);

typedef struct Sj6Rt {
    uint8_t on, note, kind;
    float vel, width, cut_hz, res, last_cut;
    float g_saw, g_pulse, g_sub, g_noise, g_sine, master, lfo_amt;
    uint32_t age;
    Sj6Osc saw, pulse, sub, sine;
    Sj6Noise noise;
    Sj6Env amp, filt;
    union {
        Sj6Ota ota;
        Sj6Lowpass lp;
        Sj6Syrup syrup;
    } flt;
} Sj6Rt;

typedef struct Sj6Engine {
    uint8_t selected;
    uint8_t resynth_on;
    uint8_t resynth_which;
    uint32_t tick;
    float control[SJ6_CTRLS];
    float lfo_phase;
    Sj6Rt voice[SJ6_POLY];
    Sj6State resynth;
} Sj6Engine;

void sj6_engine_init(Sj6Engine *e);
void sj6_engine_select(Sj6Engine *e, int on);
/* ids/values are R008 controls. Values are the resolved normalized numbers
 * the original main pass stores. Returns how many ids were out of range. */
int sj6_engine_load(Sj6Engine *e, const uint16_t *ids, const float *values, unsigned n);
void sj6_engine_note_on(Sj6Engine *e, uint8_t note, uint8_t vel);
void sj6_engine_note_off(Sj6Engine *e, uint8_t note);
/* which < 0 silences the resynthesis slot. Otherwise a stand-in index. */
void sj6_engine_resynth(Sj6Engine *e, int which);
void sj6_engine_render(Sj6Engine *e, float *left, float *right, uint32_t n, float bpm);
unsigned sj6_engine_sounding(const Sj6Engine *e);
void sj6_engine_bytes(unsigned *per_voice, unsigned *pool, unsigned *poly);
/* Source count of single-precision results for `voices` sounding pulse
 * voices on the cached lowpass, sustain, shared LFO. One pi32v2 FPU
 * result per count. Not a device measurement. */
unsigned sj6_fpu_estimate(unsigned voices);

#if X0X_SUPERJ6_BENCH
/* Read with the existing debug peek (editor.c ED_PEEK, tools/fm1_debug.py).
 * The record sits in RAM. This build does not flash it. */
typedef struct Sj6BenchRec {
    uint32_t magic, samples, cycles, voices, per_sample_q8;
} Sj6BenchRec;
extern volatile Sj6BenchRec sj6_bench;
void sj6_bench_add(uint32_t cycles, uint32_t samples, uint32_t voices);
#endif

#ifdef __cplusplus
}
#endif
#endif
