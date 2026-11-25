/* SPDX-License-Identifier: GPL-3.0-only */
/* Compact Synth One-style engine (written from scratch for DnBOX; judged by A/B only). */
#ifndef S1C_H
#define S1C_H
#include <stdint.h>
#include "s1u.h"
#ifdef X0X_S1C
#include "s1c_config.h"     /* device build: fixed sizes */
#endif
#if !defined(S1C_NO_SEQ) || !S1C_NO_SEQ
#include "sdbl.h"
#endif
#ifndef S1C_DLY_LEN
#define S1C_DLY_LEN 4096      /* samples per delay line, at sr / S1C_DLY_DEC */
#endif
#ifndef S1C_DLY_DEC
#define S1C_DLY_DEC 8
#endif
#ifndef S1C_RV_SCALE
#define S1C_RV_SCALE 1        /* reverb line lengths = original / S1C_RV_SCALE */
#endif
#ifndef S1C_RV_DEC
#define S1C_RV_DEC 1          /* reverb network runs at sr / S1C_RV_DEC */
#endif
#ifndef S1C_SAMPLE
#define S1C_SAMPLE int16_t    /* delay/reverb storage */
#endif
#ifndef S1C_KR
#define S1C_KR 1
#endif
#ifndef S1C_FAST
#define S1C_FAST 0            /* per-block coefficients, table sines, smoothed-id list */
#endif
#ifndef S1C_VOICES
#define S1C_VOICES 6
#endif
#ifndef S1C_BUSFX
#define S1C_BUSFX 0           /* 1: delay/reverb are the shared bus (device) */
#endif
#define S1C_WIDEN_MAX 2400
#ifndef S1C_DLN
#define S1C_DLN 6
#endif
#ifndef S1C_DLMONO
#define S1C_DLMONO 0
#endif
#ifndef S1C_RVB_DEC
#define S1C_RVB_DEC 1
#endif
#ifndef RVB_L0
#define RVB_L0 701
#define RVB_L1 829
#define RVB_L2 953
#define RVB_L3 1097
#endif
#ifndef WAP_L0
#define WAP_L0 0                  /* >0: widen by an allpass decorrelator instead of the 50 ms line */
#define WAP_L1 0
#define WAP_L2 0
#endif
#ifndef RVB_DEPTH
#define RVB_DEPTH 4
#endif
#ifndef S1C_RVB
#define S1C_RVB 0
#endif
#define S1C_RV_TOTAL ((26124 * 48 / 44 + 480) / (S1C_RV_SCALE * S1C_RV_DEC) + 8 * 5 + 64)
typedef s1u_env s1c_env;
#ifndef RVB_DEPTH
#define RVB_DEPTH 4
#endif
#define S1C_RVA(L) ((((L) + 2 * RVB_DEPTH + 4) + 15) & ~15)
#define S1C_RVA_WORDS (S1C_RVA(RVB_L0) + S1C_RVA(RVB_L1) + S1C_RVA(RVB_L2) + S1C_RVA(RVB_L3))
#ifndef S1C_HFR
#define S1C_HFR 0             /* regenerate the HF the decimated delay/reverb lose: energy envelopes + shaped noise */
#endif
#ifndef S1C_HFR_EB
#define S1C_HFR_EB 4          /* delay energy envelope: one value per this many line ticks */
#endif
typedef struct { float b0, b1, b2, a1, a2; } s1c_bq;
#ifndef S1C_DLAA
#define S1C_DLAA 0          /* >0: Butterworth anti-alias/anti-image at this percent of the decimated rate */
#endif
#ifndef S1C_HFR_FC
#define S1C_HFR_FC 0.3f       /* split, as a fraction of the decimated rate */
#endif
#ifndef S1C_HFR_ATT
#define S1C_HFR_ATT 0.01f
#endif
#ifndef S1C_HFR_REL
#define S1C_HFR_REL 0.0002f
#endif
#ifndef S1C_HFR_GMAX
#define S1C_HFR_GMAX 4.0f
#endif
#ifndef S1C_HFR_RV
#define S1C_HFR_RV 0
#endif
#ifndef S1C_HFR_RG
#define S1C_HFR_RG 1.0f
#endif
typedef struct {
    float freq, vamp, amp, fenv;
    uint32_t ph[3];           /* osc1, osc2, sub (q32 phase) */
    uint32_t cph, mph;        /* fm carrier and modulator */
    float lp[5], bq[2];       /* moog / butterworth state */
    s1c_env ea, ef;
    float gate;
    int8_t root, stage, pad[2];
#if S1C_KR > 1
    int32_t kc;                         /* control-rate countdown */
    float kd1, kd2, kff, kidx, kbal, kkey, kA, kfb, ka1, ka4, ka5;
    uint32_t ki1, ki2, kis, kif;         /* binary32-rounded phase increments */
#endif
} s1c_voice;
#ifndef S1C_EXT
#define S1C_EXT 0             /* 1: device memory model - caller-owned buffers, exact-size delay lines */
#endif
#if S1C_EXT
/* h16 lines: buf = 16-bit fixed (1/2048). 8-bit block-float lines: m = mantissas, ex = one exponent per
 * 16 writes, stg = the block being written (kept full precision until it completes). */
typedef struct { int16_t *buf; int8_t *m, *ex; int32_t len, w; float prev; int32_t ap, ai, bn; int16_t stg[16]; } s1c_dline;
#else
typedef struct { S1C_SAMPLE *buf; int32_t len, w; float prev, acc, last, frac; int32_t ap, ai, bn; } s1c_dline;
#endif
typedef struct {
#ifndef S1C_CSM
#define S1C_CSM 0
#endif
#if S1C_CSM
    float c[46];              /* smoothed parameters only (s1c_smid order) */
#else
    float c[150];             /* current (smoothed where the instrument glides) */
#endif
    float t[150];             /* targets */
    float sr, port_c2;
    s1u_lfo lfo[2];           /* LFO 1, LFO 2 */
    float monof, monos, glide_c2, glide_h;
    s1c_voice v[S1C_VOICES];
    int8_t held[8][2]; uint8_t nheld, prev_held, mono_prev, play_idx;
    int8_t last[8]; uint8_t nlast, delay_history, pad[2];
#if !defined(S1C_NO_SEQ) || !S1C_NO_SEQ
    sdbl beat, beat_inc;
    float beat_rate;
    uint32_t beat_step;
#endif
    uint32_t rnd;
    /* fx */
    s1u_crush crush; s1u_phaser ph; float pan_ph;
#if defined(X0X_S1_AUTOPAN_PROBE) && X0X_S1_AUTOPAN_PROBE
#if !defined(X0X_HOST) && !X0X_S1_AUTOPAN_PORT
#error Synth One autopan requires an explicit target port
#endif
    uint8_t pan_authored;
#endif
    float dlp[2][5], hp_y[2][2], cmp[3][2][4];
#if !S1C_RVB
    float rv_fs[8], rv_g, rv_damp;
    int32_t rv_w[8], rv_len[8], rv_seed[8], rv_cnt[8];
    float rv_del[8], rv_inc[8], rv_acc[2], rv_o0[2], rv_o1[2];
    int32_t rv_k;
#endif
    float send_dly, send_rev, lfo_c2, hpk[3], dl_fc, hg[2];
#if defined(X0X_S1_FEEDBACK_PROBE) && X0X_S1_FEEDBACK_PROBE
#if !defined(X0X_HOST) && !DNBOX_TYPED_CONTROLS_NATIVE_TEST
#error Synth One direct Delay Feedback requires an explicit native trial
#endif
    uint8_t delay_feedback_direct;
#endif
#if S1C_RVB
    s1u_rvb rv;               /* lines lent by s1c_attach */
    float ra[2], r0[2], r1[2]; int32_t rk;
#endif
    int32_t wpos, wlen;
#if WAP_L0 && S1C_EXT
    s1u_widen wid;
#elif WAP_L0
    uint16_t wbuf[WAP_L0 + WAP_L1 + WAP_L2]; int32_t wap_w[3];
#elif defined S1C_WIDEN_FLOAT
    float wbuf[S1C_WIDEN_MAX];
#else
    uint16_t wbuf[S1C_WIDEN_MAX];
#endif
#if !S1C_BUSFX
    s1c_dline dl[4];
    float da[2], d0[2], d1[2];
    int32_t dk, ddec, dmono;
#if S1C_EXT
    uint8_t *pool; int32_t pool_bytes, dfmt;
#else
    S1C_SAMPLE dbuf[4][S1C_DLY_LEN];
#endif
#if S1C_DLAA
    s1c_bq aa; float zaa[4][2]; int32_t aa_dd;
#endif
#if S1C_HFR
    s1c_bq hd, hr;                       /* HF analysis/synthesis high-pass: delay, reverb */
    float hd_norm, hr_norm, za[2][2], zs[2][2], zra[2][2], zrs[2][2];
    float e_acc[2], e_bacc[3], e_out[3], e_prev[3], e_cur[3];   /* lines A (L 2T), B (R 2T), C (R T) */
    int32_t e_bn, e_w, e_len, dd_set, e_eb;
#if S1C_EXT
    uint16_t *env; int32_t e_cap;        /* 3 rings of e_cap h16 entries, caller-owned */
    float rt_a, rt_in, rt_out, rt_hfin, rvb_fbt, en[2]; uint16_t hap[130]; int32_t hapw[2][2];
#else
    float env[3][S1C_DLY_LEN / S1C_HFR_EB + 4];
    float rt_a, rt_in, rt_out, rt_hfin, rvb_fbt, en[2], hap[2][2][41]; int32_t hapw[2][2];
#endif
#endif
#if !S1C_RVB
    S1C_SAMPLE rv[S1C_RV_TOTAL];
#endif
#endif
} s1c;
/* Public parameter ids (S1 parameter order) for the effects and master: set with s1c_set in
 * physical units. Bit-crush, auto-pan, widen, phaser, delay and reverb are all reachable here. */
enum {
    S1C_P_MVOL = 32, S1C_P_BCD = 33, S1C_P_BCSR = 34,                 /* volume, crush depth, crush clock Hz */
    S1C_P_PANA = 35, S1C_P_PANF = 36,                                 /* auto-pan amount 0..1, rate Hz */
    S1C_P_RVON = 37, S1C_P_RVFB = 38, S1C_P_RVHP = 39, S1C_P_RVMIX = 40,
    S1C_P_DLON = 41, S1C_P_DLFB = 42, S1C_P_DLT = 43, S1C_P_DLMIX = 44,
    S1C_P_PHMIX = 116, S1C_P_PHRATE = 117, S1C_P_PHFB = 118, S1C_P_PHW = 119,
    S1C_P_WIDEN = 121
};
void s1c_init(s1c *s, float sr, const float init[150]);
void s1c_set(s1c *s, unsigned id, float v);
/* Explicit panel bypass keeps processing the delay pool behind the mix. */
void s1c_set_delay_gate(s1c *s, float gate, int manual);
void s1c_start(s1c *s, int note, int vel);
void s1c_stop(s1c *s, int note);
void s1c_stop_all(s1c *s);
void s1c_reset_seq(s1c *s);
void s1c_render(s1c *s, float *l, float *r, unsigned n);
#if S1C_EXT
#define S1C_RVB_WORDS (RVB_L0 + RVB_L1 + RVB_L2 + RVB_L3 + 4 * (2 * RVB_DEPTH + 4))
#define S1C_WAP_WORDS (WAP_L0 + WAP_L1 + WAP_L2)
#if defined S1C_RVB_BFP
#define S1C_RVB_BYTES (S1C_RVA_WORDS + S1C_RVA_WORDS / 16 + 128)
#elif defined S1C_RVB_MU
#define S1C_RVB_BYTES S1C_RVB_WORDS
#else
#define S1C_RVB_BYTES (2 * S1C_RVB_WORDS)
#endif
/* after s1c_init: reverb lines, widen allpasses, and the pool the delay lines and HF rings are carved from */
void s1c_attach(s1c *s, void *rvb, uint16_t *wbuf, uint8_t *pool, int pool_bytes);
#endif
#endif
