/* SPDX-License-Identifier: GPL-3.0-only */
/* Synth One shared units: engine-agnostic building blocks with their state in small structs and
 * their parameters as arguments (no access to an engine struct). Used by the compact Synth One
 * engine (s1c) and available to SuperJ6 for the modules the two instruments share
 * (see /workspace/synths/SHARED-CORE.md). Written from scratch for DnBOX. */
#pragma once
#include <stdint.h>

/* ---- ADSR: one-pole segments; attack aims at 1 with tau*0.6 and turns to decay at 0.99 ---- */
typedef struct { float y, a, prev; uint8_t mode, pad[3]; } s1u_env;
enum { S1U_E_CLEAR, S1U_E_ATK, S1U_E_DEC, S1U_E_REL };
float s1u_env_step(s1u_env *e, float gate, float atk, float dec, float sus, float rel, float sr);

/* ---- LFO: sine / square / saw up / saw down, smoothed, unipolar outputs ----
 * o01 = 0..amount (adds), o10 = 1-amount..1 (scales). */
typedef struct { float ph, sm, o01, o10; } s1u_lfo;
void s1u_lfo_tick(s1u_lfo *l, float step, int shape, float amount, float smooth_c2);

/* ---- table sine: ph is a 32-bit phase (one turn = 2^32) ---- */
float s1u_sin(uint32_t ph);

/* ---- filters ---- */
float s1u_ladder_a(float x);                                   /* x = cut * 2 / sr (<= 1) -> ladder coefficient */
float s1u_ladder(float *h /* [5] */, float in, float A, float fb);  /* 4-pole ladder, 2x oversampled */
float s1u_hp2(float *z /* [2] */, float x, float a1, float a4, float a5);  /* biquad high-pass core */

/* ---- compressor: per-block coefficients, per-sample run; upd: refresh the gain law (every 4th sample) ---- */
typedef struct { float s1, s2, s3, s4, thr; } s1u_cc;
void s1u_comp_coef(s1u_cc *k, float ratio, float thr_db, float atk_s, float rel_s, float sr);
float s1u_comp(float *st /* [4] */, float x, const s1u_cc *k, int upd);

/* ---- phaser: four notch allpasses with feedback, notch sweep 100..800 Hz, stereo LFO (sin/cos) ---- */
typedef struct { float ph, rec[2][4][3], out[2]; } s1u_phaser;
void s1u_phaser_run(s1u_phaser *p, float *l, float *r, float mix, float rate, float width_hz, float feedback, float sr);

/* ---- bit-crush: sample-and-hold at clock_hz (2048..48000), set the clock at block rate ---- */
typedef struct { float idx, n, v, inc; } s1u_crush;
void s1u_crush_clock(s1u_crush *c, float clock_hz, float sr);
float s1u_crush_run(s1u_crush *c, float y);

/* ---- auto-pan: equal power, rate in Hz, amount 0..1 ---- */
void s1u_autopan(float *ph, float y, float rate, float amount, float sr, float *l, float *r);

/* ---- widen: the right channel through three h16 Schroeder allpasses, crossfaded by amount ---- */
typedef struct { uint16_t *buf; const int16_t *len; int32_t w[3]; } s1u_widen;
float s1u_widen_run(s1u_widen *w, float r, float amount);

/* ---- half floats (storage only) ---- */
uint16_t s1u_h16_put(float x);
float s1u_h16_get(uint16_t h);

/* ---- reverb: 4-line feedback delay network, 8-bit block-float lines, Householder mix ----
 * feedback 0..1 (decay), fit = {output gain, decay-time scale, damping scale}. Line lengths are
 * given at 48 kHz and scaled to sr. Memory: s1u_rvb_bytes() from the caller (no allocation). */
typedef struct {
    int8_t *m, *e; int16_t *st; uint8_t bn[4];
    const uint16_t *off;      /* 5 block-aligned offsets into m (lines 0..3, end) */
    int32_t ln[4], w[4];
    float g[4], d[4], lp[4], ph[4], fb, fit[3];
} s1u_rvb;
void s1u_rvb_init(s1u_rvb *r, const uint16_t len48[4], const uint16_t off[5], float sr);
void s1u_rvb_attach(s1u_rvb *r, void *mem);   /* off[4] + off[4] / 16 + 128 bytes, zeroed */
void s1u_rvb_run(s1u_rvb *r, float feedback, float il, float ir, float *ol, float *or_, float sr);
