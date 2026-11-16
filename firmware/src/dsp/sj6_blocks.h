/* SPDX-License-Identifier: GPL-3.0-only */
/* Envelopes, filters and conductor timing for the SuperJ6 link.
 * No libm, no heap, no function-local statics. */
#ifndef SJ6_BLOCKS_H
#define SJ6_BLOCKS_H
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif

typedef struct Sj6Seg {
    uint64_t initial, final_value, tco;
    int length;
} Sj6Seg;

typedef struct Sj6Env {
    uint64_t output, target, offset, coefficient;
    int rising, horizontal, linear, tcount, seglen;
    int cur, nseg;
    Sj6Seg seg[8];
} Sj6Env;

/* curvature: 0 linear, 1 analog, 2 linear-in-dB, 3 original OTA. */
void sj6_adsr_init(Sj6Env *e, int curvature, float rate, float attack_s, float decay_s, float sustain, float release_s);
void sj6_ahdshr_init(Sj6Env *e, int curvature, float rate, float attack_s, float hold_s, float decay_s,
                     float sustain, float release_hold_s, float release_s);
void sj6_env_start(Sj6Env *e);
void sj6_env_release(Sj6Env *e, int release_index);
float sj6_env_sample(Sj6Env *e);

typedef struct Sj6Smooth {
    uint64_t c[5], target[5], smoothing, x1, x2, y1, y2, rate, cutoff, resonance;
    int first;
} Sj6Smooth;

void sj6_smooth_init(Sj6Smooth *f, float rate);
void sj6_smooth_set(Sj6Smooth *f, float frequency, float resonance);
float sj6_smooth_process(Sj6Smooth *f, float input);

typedef struct Sj6Lowpass {
    float a0, a1, a2, b1, b2, x1, x2, y1, y2;
    float rate, last_cut, last_res;
} Sj6Lowpass;

void sj6_lowpass_init(Sj6Lowpass *f, float rate);
void sj6_lowpass_set(Sj6Lowpass *f, float cutoff_hz, float resonance);
float sj6_lowpass_process(Sj6Lowpass *f, float input);

typedef struct Sj6Ota {
    float up[8], down[8], rate, z[4], hp, hp_tan, hp_g;
    float cutoff_log, g, g_target, g_step, k, mode, mode_target, mode_step, out_gain;
    int mode_left, g_left, first, seeded, seed_ok;
} Sj6Ota;

void sj6_ota_init(Sj6Ota *f, float rate);
void sj6_ota_set(Sj6Ota *f, float cutoff, float resonance, unsigned frames);
float sj6_ota_process(Sj6Ota *f, float input);

typedef struct Sj6Syrup {
    float rate, last_cut, last_res, g, k, in_gain, out_gain, a, b, inv_a, inv_b;
    float p1, h1, p2, h2, p3, h3, p4, h4;
    int four_pole;
} Sj6Syrup;

void sj6_syrup_init(Sj6Syrup *f, float rate);
void sj6_syrup_set(Sj6Syrup *f, float cutoff, float resonance);
float sj6_syrup_process(Sj6Syrup *f, float x);

/* 0 = ok. Nonzero = the original status class (invalid / math). */
int sj6_lfo_phase(float phase, float rate, float dt, float *out);
int sj6_lfo_rate_tempo(float normalized, float tempo, float *out);
int sj6_lfo_index(float phase, int64_t *out);
int sj6_lfo_modulate(float base, float wave, float amount, unsigned waveform, float *out);
int sj6_shape_sample(unsigned kind, unsigned index, float *out);

typedef struct Sj6Beat {
    uint64_t generation, last, accumulator;
} Sj6Beat;

typedef struct Sj6Tick {
    float value;
} Sj6Tick;

/* 0 clock, 1 invalid, 2 capacity. Matches the original numeric catch-up. */
int sj6_beat_catchup(const Sj6Beat *start, uint64_t beat, uint64_t tempo, int loaded,
                     Sj6Tick *out, int capacity, int *count, Sj6Beat *result);

void sj6_blocks_keep(void);

#ifdef __cplusplus
}
#endif
#endif
