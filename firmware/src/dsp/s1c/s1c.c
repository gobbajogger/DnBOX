/* SPDX-License-Identifier: GPL-3.0-only */
/* Compact Synth One-style engine, written from scratch for DnBOX from the instrument's documented
 * behaviour (control laws, envelope, voice allocation, arpeggiator, FX order). Judged by A/B only.
 * Host prototype: libm math; the device build maps these calls to fm_* helpers. */
#define SDBL_IMPL
#include "s1c.h"
#include <string.h>
#ifndef S1C_NO_SEQ
#define S1C_NO_SEQ 0
#endif
#if !S1C_NO_SEQ
#include "sdbl.h"
#endif
#if S1C_NO_SEQ
#define ARP_ON(s) 0      /* the shared firmware arp (engine_arp) plays the notes */
#else
#define ARP_ON(s) ((s)->t[ARPON] == 1.0f)
#endif
#ifdef S1C_FASTMATH
#if __has_include("../fastmath.h")
#include "../fastmath.h"  /* firmware single-precision helpers (not newlib's fastmath.h): the device links no libm */
#else
#include "fastmath.h"
#endif
#define expf fm_expf
#define exp2f fm_exp2f
#define log2f fm_log2f
#define powf fm_powf
#define sinf fm_sinf
#define cosf fm_cosf
#define tanf fm_tanf
#define sqrtf fm_sqrtf
#define fabsf fm_fabsf
#define log10f(x) (fm_log2f(x) * 0.301029996f)
#else
#include <math.h>
#define fm_floorf floorf
#endif
/* 16-bit float (1 sign, 5 exponent, 10 mantissa; tiny values flush to 0): delay storage whose
 * error follows the signal down into quiet tails, unlike fixed point */
typedef union { float f; uint32_t u; } s1c_fb;
static uint16_t h16_put(float x)
{
    s1c_fb b; b.f = x;
    uint32_t a = (b.u & 0x7FFFFFFFu) + 0x1000u;          /* round the dropped 13 bits */
    uint32_t e = a >> 23;
    if (e < 113) return 0;
    if (e > 142) a = 142u << 23 | 0x7FFFFFu;
    return (uint16_t)((b.u >> 16 & 0x8000u) | ((a >> 13) - (112u << 10)));
}
static float h16_get(uint16_t h)
{
    s1c_fb b;
    b.u = h & 0x7FFFu ? (uint32_t)(h & 0x8000u) << 16 | ((uint32_t)(h & 0x7FFFu) + (112u << 10)) << 13 : 0u;
    return b.f;
}
/* phase increment in q32 for a cycles-per-sample value (any sign/size: wrapped to [0, 1)) */
static uint32_t ph_inc(float dt) { dt -= fm_floorf(dt); return (uint32_t)(int32_t)(dt * 2147483520.0f) << 1; }

enum { I1, I2, BAL, SEMI1, SEMI2, VOL1, VOL2, SUBV, SUBOCT, SUBSQ, FMV, FMA, NOISEV, L1I, L1A, L1R,
       CUT, RES, FMIX, FENV, MONO, GLIDE, FA, FD, FS, FR, AA, AD, AS, AR, DET2, DETM, MVOL, BCD, BCSR,
       PANA, PANF, RVON, RVFB, RVHP, RVMIX, DLON, DLFB, DLT, DLMIX, L2I, L2A, L2R, LCUT, LRES, LMIX,
       LRVMIX, LDEC, LNOISE, LFM, LDET, LFENV, LPITCH, LBC, LTREM, ARPDIR, ARPINT, ARPON, ARPOCT,
       ARPRATE, ARPSEQ, ARPSTEPS, SEQP, SEQOB = SEQP + 16, SEQON = SEQOB + 16, FTYPE = SEQON + 16,
       PHMIX, PHRATE, PHFB, PHW, LEGATO, WIDEN, CMR, CRR, CWR, CMT, CRT, CWT, CMA, CRA, CWA, CMREL,
       CRREL, CWREL, CMG, CRG, CWG, DICT, DIRES, TSYNC, PB, PBMIN, PBMAX, A4, PORT, BLO, BLE, TMULT,
       TRANS, KTRK };

static const uint8_t smooth_bits[19] = {0xe7,0x5c,0xef,0xff,0xfd,0x5f,0,0,0,0,0,0,0,0,0xf0,0x02,0,0x80,0x21};
static int smoothed(unsigned i) { return (smooth_bits[i >> 3] >> (i & 7)) & 1; }
#if S1C_FAST
static uint8_t s1c_sm[64]; static unsigned s1c_nsm;   /* ids of the smoothed parameters */
#endif
#if !S1C_BUSFX && !S1C_RVB
static const uint16_t rv_len44[8] = {2473, 2767, 3217, 3557, 3907, 4127, 2143, 1933};
/* per-line random delay drift: depth (s), segment rate (Hz), seed - the Costello network's pitch movement */
static const float rv_dev[8] = {0.0010f, 0.0011f, 0.0017f, 0.0006f, 0.0010f, 0.0011f, 0.0017f, 0.0006f};
static const float rv_hz[8] = {3.100f, 3.500f, 1.110f, 3.973f, 2.341f, 1.897f, 0.891f, 3.221f};
static const uint16_t rv_seed0[8] = {1966, 29491, 22937, 9830, 20643, 22937, 29491, 14417};
static float rv_target(const s1c *s, int n)
{
    return ((float)rv_len44[n] / 44100.0f + (float)s->rv_seed[n] * rv_dev[n] / 32768.0f) * s->sr / (S1C_RV_SCALE * S1C_RV_DEC);
}
static void rv_next(s1c *s, int n)
{
    int32_t v = s->rv_seed[n];
    if (v < 0) v += 65536;
    v = (v * 15625 + 1) & 0xFFFF;
    if (v >= 32768) v -= 65536;   /* 16-bit LCG */
    s->rv_seed[n] = v;
    s->rv_cnt[n] = (int32_t)(s->sr / S1C_RV_DEC / rv_hz[n] + 0.5f);
    s->rv_inc[n] = (rv_target(s, n) - s->rv_del[n]) / (float)s->rv_cnt[n];
}

#endif
#define TWO_PI 6.28318530718f
#define Q32 4294967296.0f
#if S1C_CSM
static const int8_t s1c_smap[150] = {0,1,2,-1,-1,3,4,5,-1,-1,6,7,8,-1,9,-1,10,11,12,13,-1,14,15,16,17,18,19,20,21,22,23,24,25,-1,26,27,28,29,30,31,32,33,34,35,36,-1,37,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,38,39,40,41,-1,42,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,43,44,-1,-1,-1,-1,45};
static const uint8_t s1c_smid[46] = {0,1,2,5,6,7,10,11,12,14,16,17,18,19,21,22,23,24,25,26,27,28,29,30,31,32,34,35,36,37,38,39,40,41,42,43,44,46,116,117,118,119,121,143,144,149};
#define P(i) (s1c_smap[i] >= 0 ? s->c[s1c_smap[i]] : s->t[i])
#else
#define P(i) (s->c[i])
#endif
static float clampf(float x, float a, float b) { return x < a ? a : x > b ? b : x; }
static float note_hz(int n) { n = n < 0 ? 0 : n > 127 ? 127 : n; return 440.0f * exp2f((n - 69) / 12.0f); }
static float lfo01(const s1c *s, float sel)
{
    int k = (int)sel;
    return k == 1 ? s->l1[0] : k == 2 ? s->l0[0] : k == 3 ? 0.5f * (s->l1[0] + s->l0[0]) : 0.0f;
}
static float lfo10(const s1c *s, float sel)
{
    int k = (int)sel;
    return k == 1 ? s->l1[1] : k == 2 ? s->l0[1] : k == 3 ? 0.5f * (s->l1[1] + s->l0[1]) : 1.0f;
}

/* ---- envelope: one-pole ADSR, attack aims at 1 with tau*0.6 and turns to decay at 0.99 ---- */
enum { E_CLEAR, E_ATK, E_DEC, E_REL };
static float pole(float t, float sr) { return t <= 0.0f ? 0.0f : expf(-1.0f / (t * sr)); }
static float env_step(s1c_env *e, float gate, float atk, float dec, float sus, float rel, float sr)
{
    if (e->prev < gate && e->mode != E_DEC) { e->mode = E_ATK; e->a = pole(atk * 0.6f, sr); }
    else if (e->prev > gate) { e->mode = E_REL; e->a = pole(rel, sr); }
    e->prev = gate;
    if (e->mode == E_CLEAR) return 0.0f;
    float x = e->mode == E_ATK ? gate : gate * sus;
    e->y = (1.0f - e->a) * x + e->a * e->y;
    if (e->mode == E_ATK && e->y > 0.99f) { e->mode = E_DEC; e->a = pole(dec, sr); }
    return e->y;
}
static void voice_release(s1c *s, s1c_voice *v, int step)
{
    v->stage = 2; v->gate = 0.0f;
    if (step) {
        env_step(&v->ea, 0.0f, P(AA), P(AD), P(AS), P(AR), s->sr);
        env_step(&v->ef, 0.0f, P(FA), P(FD), P(FS), P(FR), s->sr);
    }
}
static void voice_start(s1c_voice *v, int note, int vel, float hz)
{
    float a = vel / 127.0f;
    v->freq = hz; v->vamp = a * a; v->stage = 1; v->gate = 1.0f; v->root = (int8_t)note;
#if S1C_KR > 1
    v->kc = 0;
#endif
}
static void voice_clear(s1c_voice *v) { v->gate = 0.0f; v->stage = 0; v->amp = 0.0f; v->root = -1; }

/* ---- note handling (6-voice poly with the instrument's stealing order, or one mono voice) ---- */
static void key_on(s1c *s, int note, int vel)
{
    float hz = note_hz(note + (int)s->t[TRANS]);
    if (s->t[MONO] > 0.0f) {
        s1c_voice *v = &s->v[0];
        s->monof = hz;
        if (s->t[LEGATO] == 0.0f) voice_release(s, v, 1);
        voice_start(v, note, vel, hz);
        return;
    }
    int idx = -1, i;
    for (i = 0; i < S1C_VOICES; i++) if (s->v[i].root == note) { idx = i; break; }
    if (idx < 0) {
        for (i = 0; i < S1C_VOICES; i++) {
            int m = (s->play_idx + i) % S1C_VOICES;
            if (s->v[m].root == -1) { idx = m; break; }
        }
        if (idx < 0) idx = (s->play_idx + 1) % S1C_VOICES;
    }
    s->play_idx = (uint8_t)idx;
    voice_start(&s->v[idx], note, vel, hz);
}
static void key_off(s1c *s, int note)
{
    int i;
    if (s->t[MONO] > 0.0f) {
        if (ARP_ON(s) || !s->nheld) { if (s->v[0].stage) voice_release(s, &s->v[0], 0); }
        else {
            s1c_voice *v = &s->v[0];
            int h = s->held[0][0], vel = s->held[0][1];
            v->root = (int8_t)h; v->vamp = (vel / 127.0f) * (vel / 127.0f);
            s->monof = v->freq = note_hz(h + (int)s->t[TRANS]);
            if (s->t[LEGATO] == 0.0f) voice_release(s, v, 1);
            v->stage = 1; v->gate = 1.0f;
        }
        return;
    }
    for (i = 0; i < S1C_VOICES; i++)
        if (s->v[i].root == note) { if (s->v[i].stage) voice_release(s, &s->v[i], 1); return; }
}
void s1c_start(s1c *s, int note, int vel)
{
    int i, n = s->nheld;
    if (note < 0 || note > 127) return;
    for (i = 0; i < n; i++) if (s->held[i][0] == note) { for (int j = i; j < n - 1; j++) { s->held[j][0] = s->held[j + 1][0]; s->held[j][1] = s->held[j + 1][1]; } n--; break; }
    if (n == 8) n--;
    for (i = n; i > 0; i--) { s->held[i][0] = s->held[i - 1][0]; s->held[i][1] = s->held[i - 1][1]; }  /* no memmove on the device */
    s->held[0][0] = (int8_t)note; s->held[0][1] = (int8_t)vel; s->nheld = (uint8_t)(n + 1);
    if (!ARP_ON(s)) key_on(s, note, vel);
}
void s1c_stop(s1c *s, int note)
{
    int i, n = s->nheld;
    for (i = 0; i < n; i++) if (s->held[i][0] == note) { for (int j = i; j < n - 1; j++) { s->held[j][0] = s->held[j + 1][0]; s->held[j][1] = s->held[j + 1][1]; } s->nheld--; break; }
    if (!ARP_ON(s)) key_off(s, note);
}
void s1c_stop_all(s1c *s)
{
    int i;
    s->nheld = 0;
    if (s->t[MONO] > 0.0f) s1c_stop(s, 60);
    else for (i = 0; i < 128; i++) s1c_stop(s, i);
}
void s1c_reset_seq(s1c *s) { s->nlast = 0; }

/* ---- arpeggiator / step sequencer ---- */
#if !S1C_NO_SEQ
static void seq_off_all(s1c *s) { while (s->nlast) key_off(s, s->last[--s->nlast]); }
static void sequencer(s1c *s)
{
    const float *p = s->t;
    int on = p[ARPON] == 1.0f, n = s->nheld;
    int first_any = !s->prev_held && n, first_none = !n && s->prev_held;
    s->prev_held = (uint8_t)n;
    if (on && (first_any || first_none)) { s->beat.m = 0; s->beat.e = 0; s->beat_step = 0; seq_off_all(s); }
    if (!on && !s->nlast) return;
    /* beat clock in (soft) double precision, like the instrument: the step lands on the same sample */
    if (p[ARPRATE] != s->beat_rate) { s->beat_rate = p[ARPRATE]; s->beat_inc = sd_fdiv(p[ARPRATE] / 60.0f, (uint32_t)s->sr); }
    s->beat = sd_add(s->beat, s->beat_inc);
    int wrapped = sd_ge(s->beat, sd_from_u(s->beat_step + 1, p[TMULT]));
    if (wrapped) s->beat_step++;
    if (!(first_any || wrapped)) return;
    seq_off_all(s);
    if (!on || !n) return;
    int step = (int)s->beat_step, i;
    if (p[ARPSEQ] == 1.0f) {
        int steps = p[ARPSTEPS] > 16 ? 16 : (int)p[ARPSTEPS];
        int k = step % steps, nn = (int)p[SEQP + k], ob = (int)p[SEQOB + k];
        nn = nn < 0 ? nn - ob * 12 : nn + ob * 12;
        if ((int)p[SEQON + k] != 1) return;
        for (i = 0; i < n && s->nlast < 8; i++) {
            int note = s->held[i][0] + nn;
            if (note >= 0 && note < 128) { key_on(s, note, s->held[i][1]); s->last[s->nlast++] = (int8_t)note; }
        }
        return;
    }
    /* arpeggio over the held notes (oldest first) and octaves: up, up-down without repeated ends, down */
    int L = ((int)p[ARPOCT] + 1) * n, dir = (int)p[ARPDIR], count = dir == 1 ? L + (L > 2 ? L - 2 : 0) : L;
    if (count <= 0) return;
    i = (int)(step % (uint32_t)count);
    int idx = dir == 2 ? L - 1 - i : i < L ? i : 2 * L - 2 - i;
    const int8_t *h = s->held[n - 1 - idx % n];
    int note = h[0] + idx / n * (int)p[ARPINT];
    if (note >= 0 && note < 128) { key_on(s, note, h[1]); s->last[s->nlast++] = (int8_t)note; }
}

/* ---- oscillators ---- */
#endif

static float blep(float t, float dt)
{
    if (t < dt) { t /= dt; return t + t - t * t - 1.0f; }
    if (t > 1.0f - dt) { t = (t - 1.0f) / dt; return t * t + t + t + 1.0f; }
    return 0.0f;
}
/* the four morph shapes: triangle, square, 1/8 pulse, saw; polyBLEP when band-limiting is on */
static float shape(int k, float t, float dt, int bl)
{
    if (k == 0) return t < 0.5f ? 4.0f * t - 1.0f : 3.0f - 4.0f * t;
    if (k == 3) return 2.0f * t - 1.0f - (bl ? blep(t, dt) : 0.0f);
    float w = k == 1 ? 0.5f : 0.125f, y = t < w ? -1.0f : 1.0f;
    if (bl) { float u = t + 1.0f - w; if (u >= 1.0f) u -= 1.0f; y += blep(u, dt) - blep(t, dt); }
    return y;
}
static float morph(uint32_t *ph, float hz, float pos, int bl, float sr)
{
    float t = (float)*ph * (1.0f / Q32), dt = hz / sr;
    if (pos > 1.0f) pos -= (int)pos;
    float f = pos * 3.0f; int k = (int)f; if (k > 2) k = 2;
    f -= k;
    float y = shape(k, t, dt, bl);
    if (f > 0.0f) y += f * (shape(k + 1, t, dt, bl) - y);
    *ph += ph_inc(dt);
    return y;
}
static float sine_q(uint32_t ph) { return sinf((float)ph * (TWO_PI / Q32)); }

/* ---- filters ---- */
#if S1C_HFR || S1C_DLAA
static void bq_lp(s1c_bq *q, float f, float sr)
{
    float c = tanf(3.14159265f * f / sr), a = 1.0f / (1.0f + 1.41421356f * c + c * c);
    q->b0 = c * c * a; q->b1 = 2.0f * c * c * a; q->b2 = c * c * a;
    q->a1 = 2.0f * (c * c - 1.0f) * a; q->a2 = (1.0f - 1.41421356f * c + c * c) * a;
}
static void bq_hp(s1c_bq *q, float f, float sr)
{
    float c = tanf(3.14159265f * f / sr), a = 1.0f / (1.0f + 1.41421356f * c + c * c);
    q->b0 = a; q->b1 = -2.0f * a; q->b2 = a;
    q->a1 = 2.0f * (c * c - 1.0f) * a; q->a2 = (1.0f - 1.41421356f * c + c * c) * a;
}
static float bq_run(const s1c_bq *q, float *z, float x)
{
    float y = q->b0 * x + z[0];
    z[0] = q->b1 * x - q->a1 * y + z[1]; z[1] = q->b2 * x - q->a2 * y;
    return y;
}
static float hfr_noise(s1c *s) { s->rnd = s->rnd * 1664525u + 1013904223u; return (float)(int32_t)s->rnd * (1.0f / 2147483648.0f); }
static float bq_norm(s1c *s, const s1c_bq *q)   /* 1 / rms of high-passed uniform noise (load time) */
{
    float z[2] = {0.0f, 0.0f}, e = 0.0f;
    for (int i = 0; i < 4096; i++) { float y = bq_run(q, z, hfr_noise(s)); if (i >= 512) e += y * y; }
    return e > 0.0f ? sqrtf(3584.0f / e) : 0.0f;
}
#endif

static float moog(float *h, float in, float cut, float res, float sr)
{
    /* four one-pole stages with resonance feedback, run twice per sample (2x oversampled) */
    float x = cut * (2.0f / sr), o = 0.0f;
    if (x > 1.0f) x = 1.0f;
    float A = 1.0f - x * (1.56874466f + x * (-1.73140822f + x * (1.55879259f + x * (-0.711601815f + x * 0.118475673f))));
    float fb = res * (3.98720139f + x * (3.68178143f - x * 3.93633599f));
    for (int k = 0; k < 2; k++) {
        float tmp = h[3], v = in - fb * 0.5f * (h[3] + h[4]);
        float z1 = v + A * (h[0] - v), z2 = z1 + A * (h[1] - z1), z3 = z2 + A * (h[2] - z2), z4 = z3 + A * (h[3] - z3);
        h[0] = z1; h[1] = z2; h[2] = z3; h[3] = z4; h[4] = tmp;
        o += z4;
    }
    return 0.5f * o;
}
static float buthp(float *y, float in, float f, float sr)
{
    float c = tanf(3.14159265f * f / sr), a1 = 1.0f / (1.0f + 1.41421356f * c + c * c);
    float a4 = 2.0f * (c * c - 1.0f) * a1, a5 = (1.0f - 1.41421356f * c + c * c) * a1;
    float t = in - a4 * y[0] - a5 * y[1], o = t * a1 - 2.0f * a1 * y[0] + a1 * y[1];
    y[1] = y[0]; y[0] = t;
    return o;
}
static float butbp(float *y, float in, float f, float bw, float sr)
{
    float c = 1.0f / tanf(3.14159265f * bw / sr), d = 2.0f * cosf(TWO_PI * f / sr), a1 = 1.0f / (1.0f + c);
    float a4 = -c * d * a1, a5 = (c - 1.0f) * a1;
    float t = in - a4 * y[0] - a5 * y[1], o = t * a1 - a1 * y[1];
    y[1] = y[0]; y[0] = t;
    return o;
}

/* ---- one voice sample ---- */
static float voice_run(s1c *s, s1c_voice *v)
{
    int mono = P(MONO) > 0.0f;
    float base = mono ? s->monos : v->freq, half = 0.5f * s->sr;
    float plfo = 1.0f + lfo01(s, P(LPITCH)) * 0.0594630944f;
    float k = P(DETM) * plfo;   /* pitch bend: no wheel on DnBOX */
    float f1 = clampf(base * exp2f((int)P(SEMI1) / 12.0f) * k, 0.0f, half);
    float det = P(DET2) * (v->freq / 261.6255653f);
    if ((int)P(LDET)) det *= lfo01(s, P(LDET));
    float f2 = clampf(base * exp2f((int)P(SEMI2) / 12.0f) * k + det, 0.0f, half);
    float fs = clampf(base * k / (2.0f * (1.0f + P(SUBOCT))), 0.0f, half);
    float ff = clampf(base * k, 0.0f, half);
    float idx = P(FMA) * lfo10(s, P(LFM)); idx = clampf(idx, 0.0f, 15.0f);
    float dec = clampf(P(AD) * lfo10(s, P(LDEC)), 0.005f, 2.0f);
    float bal = clampf(P(BAL) + lfo01(s, P(LMIX)), 0.0f, 1.0f);
    float res = clampf(P(RES) * lfo10(s, P(LRES)), 0.0f, 0.98f);
    v->amp = env_step(&v->ea, v->gate, P(AA), dec, P(AS), P(AR), s->sr);
    v->fenv = env_step(&v->ef, v->gate, P(FA), P(FD), P(FS), P(FR), s->sr);
    float cut = P(CUT) * lfo10(s, P(LCUT)), em = P(FENV) * lfo10(s, P(LFENV));
    cut = clampf(cut - cut * em * (1.0f - v->fenv), 64.0f, 22050.0f);
    int bl = P(BLE) > 0.0f;
    float o1 = morph(&v->ph[0], f1, P(I1), bl, s->sr) * v->vamp * P(VOL1);
    float o2 = morph(&v->ph[1], f2, P(I2), bl, s->sr) * v->vamp * P(VOL2);
    float y = (1.0f - bal) * o1 + bal * o2;
    float sub = sine_q(v->ph[2]);
    v->ph[2] += ph_inc(fs / s->sr);
    y += P(SUBSQ) != 0.0f ? (sub > 0.0f ? P(SUBV) : -P(SUBV)) : sub * v->vamp * P(SUBV) * 3.0f;
    if (P(FMV) != 0.0f) {
        float mod = sine_q(v->mph) * ff * idx;
        v->mph += ph_inc(ff / s->sr);
        y += sine_q(v->cph) * v->vamp * P(FMV);
        v->cph += ph_inc((ff + mod) / s->sr);
    }
    if (P(NOISEV) != 0.0f) {
        s->rnd = s->rnd * 1664525u + 1013904223u;
        y += ((float)(int32_t)s->rnd * (1.0f / 2147483648.0f)) * v->vamp * P(NOISEV) * lfo10(s, P(LNOISE));
    }
    float pitch = log2f(f1 > 0.0f ? f1 : 261.0f), kt = 1.0f - clampf((pitch - 6.0f) / 5.0f, 0.0f, 1.0f);
    float kf = 1.0f - P(KTRK);
    y *= v->amp * ((1.0f - kf) * kt * kt + kf);
    int ft = (int)P(FTYPE);
    float fo = ft == 0 ? moog(v->lp, y, cut, res, s->sr)
             : ft == 1 ? butbp(v->bq, y, cut, 0.0625f * s->sr * (exp2f(clampf(1.0f - res, 0.0f, 1.0f)) - 1.0f), s->sr)
             : buthp(v->bq, y, cut, s->sr);
    return y + P(FMIX) * (fo - y);   /* filter mix glides in from dry at start-up, like the original */
}

#if S1C_KR > 1 || S1C_FAST
static const float sin_q[65] = {
    0.0f, 0.024541229f, 0.0490676761f, 0.0735645667f, 0.0980171412f, 0.122410677f, 0.146730468f, 0.170961887f,
    0.195090324f, 0.219101235f, 0.242980182f, 0.266712755f, 0.290284663f, 0.313681751f, 0.336889863f, 0.359895051f,
    0.382683426f, 0.405241311f, 0.427555084f, 0.449611336f, 0.471396744f, 0.492898196f, 0.514102757f, 0.534997642f,
    0.555570245f, 0.575808167f, 0.59569931f, 0.615231574f, 0.634393275f, 0.653172851f, 0.671558976f, 0.689540565f,
    0.707106769f, 0.724247098f, 0.740951121f, 0.757208824f, 0.773010433f, 0.78834641f, 0.803207517f, 0.817584813f,
    0.831469595f, 0.84485358f, 0.857728601f, 0.870086968f, 0.881921291f, 0.893224299f, 0.903989315f, 0.914209783f,
    0.923879504f, 0.932992816f, 0.941544056f, 0.949528158f, 0.956940353f, 0.963776052f, 0.970031261f, 0.975702107f,
    0.980785251f, 0.985277653f, 0.989176512f, 0.992479563f, 0.99518472f, 0.997290432f, 0.99879545f, 0.999698818f,
    1.0f
};
static float sin_at(uint32_t i)                 /* sin(i * 2 pi / 256) for i in 0..256, from a quarter wave */
{
    uint32_t k = i & 127u;
    float v = sin_q[k <= 64u ? k : 128u - k];
    return (i & 128u) ? -v : v;
}
static float sine_t(uint32_t ph)
{
    uint32_t i = ph >> 24;
    float f = (float)(ph & 0xFFFFFFu) * (1.0f / 16777216.0f);
    float a = sin_at(i);
    return a + f * (sin_at(i + 1u) - a);
}
#endif
#if S1C_KR > 1
/* Cheap voice: pitch, LFO, key-tracking and filter coefficients at control rate (every S1C_KR
 * samples), sines from a 256-point table, the filter envelope still per sample in the cutoff. */
static float voice_run_kr(s1c *s, s1c_voice *v)
{
    int mono = P(MONO) > 0.0f, ft = (int)P(FTYPE);
    v->amp = env_step(&v->ea, v->gate, P(AA), clampf(P(AD) * lfo10(s, P(LDEC)), 0.005f, 2.0f), P(AS), P(AR), s->sr);
    v->fenv = env_step(&v->ef, v->gate, P(FA), P(FD), P(FS), P(FR), s->sr);
    if (--v->kc <= 0) {
        v->kc = S1C_KR;
        float base = mono ? s->monos : v->freq, half = 0.5f * s->sr;
        float plfo = 1.0f + lfo01(s, P(LPITCH)) * 0.0594630944f, k = P(DETM) * plfo;
        v->kf1 = clampf(base * exp2f((int)P(SEMI1) / 12.0f) * k, 0.0f, half);
        float det = P(DET2) * (v->freq / 261.6255653f);
        if ((int)P(LDET)) det *= lfo01(s, P(LDET));
        v->kf2 = clampf(base * exp2f((int)P(SEMI2) / 12.0f) * k + det, 0.0f, half);
        v->kfs = clampf(base * k / (2.0f * (1.0f + P(SUBOCT))), 0.0f, half);
        v->kff = clampf(base * k, 0.0f, half);
        v->kidx = clampf(P(FMA) * lfo10(s, P(LFM)), 0.0f, 15.0f);
        v->kbal = clampf(P(BAL) + lfo01(s, P(LMIX)), 0.0f, 1.0f);
        v->kres = clampf(P(RES) * lfo10(s, P(LRES)), 0.0f, 0.98f);
        float pitch = log2f(v->kf1 > 0.0f ? v->kf1 : 261.0f), kt = 1.0f - clampf((pitch - 6.0f) / 5.0f, 0.0f, 1.0f);
        float kf = 1.0f - P(KTRK);
        v->kkey = (1.0f - kf) * kt * kt + kf;
        v->kcut = P(CUT) * lfo10(s, P(LCUT)); v->kem = P(FENV) * lfo10(s, P(LFENV));
        float cut = clampf(v->kcut - v->kcut * v->kem * (1.0f - v->fenv), 64.0f, 22050.0f);
        if (ft == 0) {
            float x = cut * (2.0f / s->sr); if (x > 1.0f) x = 1.0f;
            v->kA = 1.0f - x * (1.56874466f + x * (-1.73140822f + x * (1.55879259f + x * (-0.711601815f + x * 0.118475673f))));
            v->kfb = v->kres * (3.98720139f + x * (3.68178143f - x * 3.93633599f));
        } else if (ft == 1) {
            float bw = 0.0625f * s->sr * (exp2f(clampf(1.0f - v->kres, 0.0f, 1.0f)) - 1.0f);
            float cc = 1.0f / tanf(3.14159265f * bw / s->sr), d = 2.0f * cosf(TWO_PI * cut / s->sr), a1 = 1.0f / (1.0f + cc);
            v->ka1 = a1; v->ka4 = -cc * d * a1; v->ka5 = (cc - 1.0f) * a1;
        } else {
            float cc = tanf(3.14159265f * cut / s->sr), a1 = 1.0f / (1.0f + 1.41421356f * cc + cc * cc);
            v->ka1 = a1; v->ka4 = 2.0f * (cc * cc - 1.0f) * a1; v->ka5 = (1.0f - 1.41421356f * cc + cc * cc) * a1;
        }
    }
    int bl = P(BLE) > 0.0f;
    float o1 = morph(&v->ph[0], v->kf1, P(I1), bl, s->sr) * v->vamp * P(VOL1);
    float o2 = morph(&v->ph[1], v->kf2, P(I2), bl, s->sr) * v->vamp * P(VOL2);
    float y = (1.0f - v->kbal) * o1 + v->kbal * o2;
    float sub = sine_t(v->ph[2]);
    v->ph[2] += ph_inc(v->kfs / s->sr);
    y += P(SUBSQ) != 0.0f ? (sub > 0.0f ? P(SUBV) : -P(SUBV)) : sub * v->vamp * P(SUBV) * 3.0f;
    if (P(FMV) != 0.0f) {
        float mod = sine_t(v->mph) * v->kff * v->kidx;
        v->mph += ph_inc(v->kff / s->sr);
        y += sine_t(v->cph) * v->vamp * P(FMV);
        v->cph += ph_inc((v->kff + mod) / s->sr);
    }
    if (P(NOISEV) != 0.0f) {
        s->rnd = s->rnd * 1664525u + 1013904223u;
        y += ((float)(int32_t)s->rnd * (1.0f / 2147483648.0f)) * v->vamp * P(NOISEV) * lfo10(s, P(LNOISE));
    }
    y *= v->amp * v->kkey;
    float fo;
    if (ft == 0) {
        float *hh = v->lp, A = v->kA, fb = v->kfb, o = 0.0f;
        for (int k2 = 0; k2 < 2; k2++) {
            float tmp = hh[3], vv = y - fb * 0.5f * (hh[3] + hh[4]);
            float z1 = vv + A * (hh[0] - vv), z2 = z1 + A * (hh[1] - z1), z3 = z2 + A * (hh[2] - z2), z4 = z3 + A * (hh[3] - z3);
            hh[0] = z1; hh[1] = z2; hh[2] = z3; hh[3] = z4; hh[4] = tmp; o += z4;
        }
        fo = 0.5f * o;
    } else {
        float t = y - v->ka4 * v->bq[0] - v->ka5 * v->bq[1];
        fo = ft == 1 ? t * v->ka1 - v->ka1 * v->bq[1] : t * v->ka1 - 2.0f * v->ka1 * v->bq[0] + v->ka1 * v->bq[1];
        v->bq[1] = v->bq[0]; v->bq[0] = t;
    }
    return y + P(FMIX) * (fo - y);
}
#define voice_run voice_run_kr
#endif
/* ---- FX ---- */
typedef struct { float s1, s2, s3, s4, thr; } s1c_cc;
static void comp_coef(s1c_cc *k, float ratio, float thr, float atk, float rel, float sr)
{
    k->s1 = atk > 0.0f ? expf(-2.0f / (sr * atk)) : 0.0f; k->s3 = atk > 0.0f ? expf(-1.0f / (sr * atk)) : 0.0f;
    k->s4 = rel > 0.0f ? expf(-1.0f / (sr * rel)) : 0.0f; k->s2 = (1.0f - k->s1) * (1.0f / ratio - 1.0f);
    k->thr = thr;
}
#if S1C_FAST
static int comp_upd;     /* set on every 4th frame: detector-to-dB and gain law at quarter rate */
static float comp(float *st, float x, const s1c_cc *k)
{
    float ax = fabsf(x), a = st[0] > ax ? k->s4 : k->s3;
    st[0] = st[0] * a + (1.0f - a) * ax;
    if (comp_upd) {
        float over = st[0] > 0.0f ? 6.02059991f * log2f(st[0]) - k->thr : 0.0f;
        st[2] = over > 0.0f ? over : 0.0f;
        st[3] = st[1] != 0.0f ? exp2f(0.166096405f * st[1]) : 1.0f;
    }
    st[1] = k->s1 * st[1] + k->s2 * st[2];
    return x * st[3];
}
#else
static float comp(float *st, float x, const s1c_cc *k)
{
    float ax = fabsf(x), a = st[0] > ax ? k->s4 : k->s3;
    st[0] = st[0] * a + (1.0f - a) * ax;
    float over = st[0] > 0.0f ? 6.02059991f * log2f(st[0]) - k->thr : 0.0f;
    st[1] = k->s1 * st[1] + k->s2 * (over > 0.0f ? over : 0.0f);
    return st[1] != 0.0f ? x * exp2f(0.166096405f * st[1]) : x;
}
#endif
/* four notch allpass stages with feedback, vibrato (wet-only) mode, notch sweep 100..800 Hz */
static float phaser_ch(float r[4][3], float *fb, float in, float lfo, float width, float fbk, float sr)
{
    float w = 0.5f * (TWO_PI * 800.0f - TWO_PI * 100.0f) * (1.0f - lfo) + TWO_PI * 100.0f;
    float g = expf(-3.14159f * width / sr), f4 = g * g, f5 = -2.0f * g, m = 1.5f, t[4];
    int k;
    for (k = 0; k < 4; k++) { t[k] = r[k][1] * cosf(m * w / sr); m *= 1.5f; }
    float r40 = -((f5 * t[0] + f4 * r[0][2]) - (in + fbk * *fb));
    float r30 = f5 * (t[0] - t[1]) + r[0][2] + f4 * (r40 - r[1][2]);
    float r20 = f5 * (t[1] - t[2]) + r[1][2] + f4 * (r30 - r[2][2]);
    float r10 = f5 * (t[2] - t[3]) + r[2][2] + f4 * (r20 - r[3][2]);
    float out = f4 * r10 + f5 * t[3] + r[3][2];
    float nw[4] = {r40, r30, r20, r10};
    for (k = 0; k < 4; k++) { r[k][2] = r[k][1]; r[k][1] = nw[k]; }
    *fb = out;
    return out;
}
#if !S1C_BUSFX
#ifdef S1C_OPT_A
#ifndef S1C_DLB
#define S1C_DLB 2.0f
#endif
/* option A storage formats, simulated on the host: 8-bit float (1/4/3) delay, 16-bit float reverb */
static float q8(float x)
{
    s1c_fb b; b.f = x;
    uint32_t a = (b.u & 0x7FFFFFFFu) + 0x80000u, e = a >> 23;   /* keep 3 mantissa bits */
    if (e < 127 - 14) return 0.0f;
    if (e > 127 + 2) a = (127u + 2u) << 23 | 0x700000u;
    b.u = (b.u & 0x80000000u) | (a & 0xFFF00000u);
    return b.f;
}
#ifdef S1C_DL8
#define DL_Q(x) q8(x)
#else
#define DL_Q(x) h16_get(h16_put(x))
#endif
#define RV_Q(x) h16_get(h16_put(x))
#else
#define DL_Q(x) (x)
#define RV_Q(x) (x)
#endif
static float dl_read(const s1c_dline *d, float del) /* del in line samples, linear interpolation */
{
    float pos = (float)d->w - del;
    while (pos < 0.0f) pos += (float)d->len;
    int i = (int)pos; float f = pos - (float)i;
    int j = i + 1 == d->len ? 0 : i + 1;
    return (float)d->buf[i] * (1.0f / 2048.0f) * (1.0f - f) + (float)d->buf[j] * (1.0f / 2048.0f) * f;
}
#ifdef S1C_ADPCM
int s1c_dlfmt = 1;               /* host sim: 0 = 16-bit float, 1 = IMA ADPCM, 2 = 8-bit block float (16) */
static const float s1c_dlbytes[3] = {2.0f, 0.547f, 1.0625f};
/* IMA ADPCM, 4 bits per sample, one encoder per line; the device decoder reads the same
 * reconstruction sequentially from 64-sample keyframed blocks, so the host stores it directly. */
static const int16_t ima_step[89] = {7,8,9,10,11,12,13,14,16,17,19,21,23,25,28,31,34,37,41,45,50,55,60,66,73,80,88,97,107,118,130,
 143,157,173,190,209,230,253,279,307,337,371,408,449,494,544,598,658,724,796,876,963,1060,1166,1282,1411,1552,1707,1878,2066,
 2272,2499,2749,3024,3327,3660,4026,4428,4871,5358,5894,6484,7132,7845,8630,9493,10442,11487,12635,13899,15289,16818,18500,
 20350,22385,24623,27086,29794,32767};
static const int8_t ima_idx[8] = {-1,-1,-1,-1,2,4,6,8};
static float adpcm_q(s1c_dline *d, float x)
{
    float v = x * 2048.0f; int s = v > 32767.0f ? 32767 : v < -32768.0f ? -32768 : (int)v;
    int step = ima_step[d->ai], diff = s - d->ap, code = 0, dq = step >> 3;
    if (diff < 0) { code = 8; diff = -diff; }
    if (diff >= step) { code |= 4; diff -= step; dq += step; }
    if (diff >= step >> 1) { code |= 2; diff -= step >> 1; dq += step >> 1; }
    if (diff >= step >> 2) { code |= 1; dq += step >> 2; }
    d->ap += code & 8 ? -dq : dq;
    if (d->ap > 32767) d->ap = 32767; else if (d->ap < -32768) d->ap = -32768;
    d->ai += ima_idx[code & 7]; if (d->ai < 0) d->ai = 0; else if (d->ai > 88) d->ai = 88;
    return (float)d->ap * (1.0f / 2048.0f);
}
static float dl_store(s1c_dline *d, float x)
{
    if (s1c_dlfmt == 1) return adpcm_q(d, x);
    if (s1c_dlfmt == 0) return h16_get(h16_put(x));
    /* block float: when a 16-sample block completes, requantise it to 8-bit mantissas of its peak */
    if (++d->bn == 16) {
        int i, j = d->w; float pk = fabsf(x);
        for (i = 1; i < 16; i++) { j = j ? j - 1 : d->len - 1; float a = fabsf((float)d->buf[j] * (1.0f / 2048.0f)); if (a > pk) pk = a; }
        d->bn = 0;
        if (pk > 0.0f) {
            float sc = exp2f(__builtin_ceilf(log2f(pk))) / 127.0f; j = d->w;
            for (i = 1; i < 16; i++) { j = j ? j - 1 : d->len - 1; d->buf[j] = (S1C_SAMPLE)(__builtin_roundf((float)d->buf[j] * (1.0f / 2048.0f) / sc) * sc * 2048.0f); }
            x = __builtin_roundf(x / sc) * sc;
        }
    }
    return x;
}
#endif
static float dl_tick(s1c_dline *d, float in, float del, float fb)
{
#ifdef S1C_ADPCM
    float v = dl_store(d, in + d->prev * fb) * 2048.0f;
#else
    float v = DL_Q(in + d->prev * fb) * 2048.0f;
#endif
    d->buf[d->w] = (S1C_SAMPLE)(v > 32767.0f ? 32767.0f : v < -32768.0f ? -32768.0f : v);
    float o = dl_read(d, del);
    if (++d->w == d->len) d->w = 0;
    d->prev = o;
    return o;
}
/* decimated lines: the feedback tap stays at the true period; the output tap is read
 * earlier by the decimation latency (box input + linear output = 1.5 ticks - 0.5 samples) */
static float dl_tick_c(s1c_dline *d, float in, float del, float fb, float c)
{
#ifdef S1C_ADPCM
    float v = dl_store(d, in + d->prev * fb) * 2048.0f;
#else
    float v = DL_Q(in + d->prev * fb) * 2048.0f;
#endif
    d->buf[d->w] = (S1C_SAMPLE)(v > 32767.0f ? 32767.0f : v < -32768.0f ? -32768.0f : v);
    d->prev = dl_read(d, del);
    float o = dl_read(d, del - c > 0.0f ? del - c : 0.0f);
    if (++d->w == d->len) d->w = 0;
    return o;
}

#if S1C_EXT && S1C_DL_H16
/* Device memory model (smallest): exact-size half-float lines carved from a caller-owned pool. */
static float xl_read(const s1c_dline *d, float del)
{
    float pos = (float)d->w - del;
    while (pos < 0.0f) pos += (float)d->len;
    int i = (int)pos; float f = pos - (float)i;
    int j = i + 1 == d->len ? 0 : i + 1;
    const uint16_t *b = (const uint16_t *)d->buf;
    return h16_get(b[i]) * (1.0f - f) + h16_get(b[j]) * f;
}
static float xl_tick(s1c_dline *d, float in, float del, float fb)
{
    ((uint16_t *)d->buf)[d->w] = h16_put(in + d->prev * fb);
    float o = xl_read(d, del);
    if (++d->w == d->len) d->w = 0;
    d->prev = o;
    return o;
}
#define DL_TICK(d, in, del, fb) xl_tick(d, in, del, fb)
#elif S1C_EXT
/* Device memory model: exact-size lines carved from a caller-owned pool. h16 lines keep the host's
 * 16-bit fixed storage; 8-bit block-float lines keep a mantissa per sample and an exponent per 16
 * writes, the block being written stays full precision in stg (same values the host sim produces). */
static float xl_smp(const s1c_dline *d, int i, int fmt)
{
    if (fmt != 2) return (float)d->buf[i];
    int b = i >> 4;
    if (b == (d->w >> 4) && (i & 15) < d->bn) return (float)d->stg[i & 15];
    int e = d->ex[b];
    if (e == -128) return 0.0f;
    float v = (float)d->m[i] * (exp2f((float)e) / 127.0f) * 2048.0f;
    return (float)(int16_t)(v > 32767.0f ? 32767.0f : v < -32768.0f ? -32768.0f : v);
}
static float xl_read(const s1c_dline *d, float del, int fmt)
{
    float pos = (float)d->w - del;
    while (pos < 0.0f) pos += (float)d->len;
    int i = (int)pos; float f = pos - (float)i;
    int j = i + 1 == d->len ? 0 : i + 1;
    return xl_smp(d, i, fmt) * (1.0f / 2048.0f) * (1.0f - f) + xl_smp(d, j, fmt) * (1.0f / 2048.0f) * f;
}
static float xl_tick(s1c_dline *d, float in, float del, float fb, int fmt)
{
    float x = in + d->prev * fb;
    if (fmt == 2) {
        int p = d->w;
        if (++d->bn == 16) {
            int base = p & ~15, i; float pk = fabsf(x);
            for (i = 0; i < 15; i++) { float a = fabsf((float)d->stg[i] * (1.0f / 2048.0f)); if (a > pk) pk = a; }
            d->bn = 0;
            if (pk > 0.0f) {
                float e = __builtin_ceilf(log2f(pk)), sc = exp2f(e) / 127.0f;
                for (i = 0; i < 15; i++) d->m[base + i] = (int8_t)__builtin_roundf((float)d->stg[i] * (1.0f / 2048.0f) / sc);
                d->m[p] = (int8_t)__builtin_roundf(x / sc);
                d->ex[base >> 4] = (int8_t)e;
            } else {
                for (i = 0; i < 16; i++) d->m[base + i] = 0;
                d->ex[base >> 4] = -128;
            }
        } else {
            float v = x * 2048.0f;
            d->stg[d->bn - 1] = (int16_t)(v > 32767.0f ? 32767.0f : v < -32768.0f ? -32768.0f : v);
        }
    } else {
        float v = h16_get(h16_put(x)) * 2048.0f;
        d->buf[d->w] = (int16_t)(v > 32767.0f ? 32767.0f : v < -32768.0f ? -32768.0f : v);
    }
    float o = xl_read(d, del, fmt);
    if (++d->w == d->len) d->w = 0;
    d->prev = o;
    return o;
}
#define DL_TICK(d, in, del, fb) xl_tick(d, in, del, fb, s->dfmt)
#else
#define DL_TICK(d, in, del, fb) dl_tick(d, in, del, fb)
#endif
#if !S1C_RVB
static void reverb(s1c *s, float il, float ir, float *ol, float *or_)
{
    float fbk = P(RVFB), jp = 0.0f, al = 0.0f, ar = 0.0f;
    int n;
    for (n = 0; n < 8; n++) jp += s->rv_fs[n];
    jp *= 0.25f;
    S1C_SAMPLE *b = s->rv;
    for (n = 0; n < 8; n++) {
        int len = s->rv_len[n], w = s->rv_w[n];
        float x = ((n & 1) ? jp + ir : jp + il) - s->rv_fs[n];
        float v = x * 4096.0f;
        b[w] = (S1C_SAMPLE)(v > 32767.0f ? 32767.0f : v < -32768.0f ? -32768.0f : v);
        if (++w == len) w = 0;
        s->rv_w[n] = w;
        /* fractional read behind the write head, linear interpolation */
        float rp = (float)w - s->rv_del[n];
        if (rp < 0.0f) rp += (float)len;
        int i0 = (int)rp; float fr = rp - (float)i0;
        int i1 = i0 + 1 == len ? 0 : i0 + 1;
        float rd = ((float)b[i0] + fr * (float)(b[i1] - b[i0])) * (1.0f / 4096.0f);
        s->rv_del[n] += s->rv_inc[n];
        if (--s->rv_cnt[n] <= 0) rv_next(s, n);
        /* shorter lines: same decay per second (gain ^ (1/scale)), same damping per second */
        float g = s->rv_g * rd;
        float y = (s->rv_fs[n] - g) * s->rv_damp + g;
        s->rv_fs[n] = y;
        if (n & 1) ar += y; else al += y;
        b += len;
    }
    (void)fbk;
    *ol = al * 0.35f; *or_ = ar * 0.35f;
}
#endif

#endif
#if S1C_RVB
/* Option B reverb: 4-line feedback delay network in 16-bit floats (about 7 KB at 48 kHz).
 * Decay per second and HF loss per second follow the original 8-line network's feedback and
 * damping; output gain, decay scale and colour are fitted per program. */
static const uint16_t rvb_len48[4] = {RVB_L0, RVB_L1, RVB_L2, RVB_L3};

float s1c_rvb_fit[3] = {1.0f, 1.0f, 1.0f};
#if S1C_EXT
#define FITV(s) ((s)->rfit)
#else
#define FITV(s) s1c_rvb_fit
#endif
#ifdef S1C_RVB_MU
/* 8-bit mu-law-style companding (sign + 7-bit log magnitude, 1/8 full scale = 16) */
static uint16_t mu_put(float x)
{
    float a = fabsf(x) * (1.0f / 16.0f); if (a > 1.0f) a = 1.0f;
    int q = (int)(127.0f * log2f(1.0f + 255.0f * a) * 0.125f + 0.5f);
    return (uint16_t)(x < 0.0f ? 128 + q : q);
}
static float mu_get(uint16_t c)
{
    float m = (exp2f((float)(c & 127) * (8.0f / 127.0f)) - 1.0f) * (16.0f / 255.0f);
    return c & 128 ? -m : m;
}
#define RVB_PUT mu_put
#define RVB_GET mu_get
#else
#define RVB_PUT h16_put
#define RVB_GET h16_get
#endif
#ifndef WAP_G
#define WAP_G 0.6f
#endif
#ifndef RVB_DEPTH
#define RVB_DEPTH 4
#endif   /* gain, decay-time scale, damping scale (host fit) */

#ifdef S1C_RVB_BFP
/* 8-bit block-float reverb lines: one exponent per 16 writes, the block being written kept at
 * 16-bit precision until it completes (the same scheme as the delay lines). */
static const uint16_t rva_off[5] = {0, S1C_RVA(RVB_L0), S1C_RVA(RVB_L0) + S1C_RVA(RVB_L1),
    S1C_RVA(RVB_L0) + S1C_RVA(RVB_L1) + S1C_RVA(RVB_L2), S1C_RVA_WORDS};
static float rb_get(const s1c *s, int n, int i, int w)
{
    int idx = rva_off[n] + i;
    if ((i >> 4) == (w >> 4) && (i & 15) < s->rvbn[n]) return (float)s->rvs[n * 16 + (i & 15)] * (1.0f / 2048.0f);
    int e = s->rve[idx >> 4];
    if (e == -128) return 0.0f;
    return (float)s->rvm[idx] * (exp2f((float)e) / 127.0f);
}
static void rb_put(s1c *s, int n, int w, float x)
{
    int16_t *st = s->rvs + n * 16;
    float v = x * 2048.0f;
    st[w & 15] = (int16_t)(v > 32767.0f ? 32767.0f : v < -32768.0f ? -32768.0f : v);
    if (++s->rvbn[n] == 16) {
        int base = rva_off[n] + (w & ~15), i; float pk = 0.0f;
        s->rvbn[n] = 0;
        for (i = 0; i < 16; i++) { float a = fabsf((float)st[i] * (1.0f / 2048.0f)); if (a > pk) pk = a; }
        if (pk > 0.0f) {
            float e = __builtin_ceilf(log2f(pk)), sc = exp2f(e) / 127.0f;
            for (i = 0; i < 16; i++) s->rvm[base + i] = (int8_t)__builtin_roundf((float)st[i] * (1.0f / 2048.0f) / sc);
            s->rve[base >> 4] = (int8_t)e;
        } else s->rve[base >> 4] = -128;
    }
}
#endif
static void rvb_compact(s1c *s, float il, float ir, float *ol, float *or_)
{
#if S1C_FAST
    float y[4], fb = s->t[RVFB];
#else
    float y[4], fb = P(RVFB);
#endif
#if defined S1C_RVB_BFP
#define RLEN(n) (rva_off[(n) + 1] - rva_off[n])
#define RGET(i) rb_get(s, n, i, w)
#else
#if S1C_EXT && defined S1C_RVB_MU
    uint8_t *b = s->rvb;
#else
    uint16_t *b = s->rvb;
#endif
#define RLEN(n) (s->rvb_ln[n] + 2 * RVB_DEPTH + 4)
#define RGET(i) RVB_GET(b[i])
#endif
    int n;
    if (fb != s->rvb_fb) {                    /* per-line gains when feedback changes */
        float lref = 0.068390f * s->sr / (float)S1C_RVB_DEC, G = 0.70710678f;
        s->rvb_fb = fb;
        for (n = 0; n < 4; n++) {
            float r = (float)s->rvb_ln[n] / lref;
            s->rvb_g[n] = fb > 0.0f ? powf(fb, r / FITV(s)[1]) : 0.0f;
            float gn = powf(G, r * FITV(s)[2]);
            s->rvb_d[n] = (1.0f - gn) / (1.0f + gn);
        }
    }
    for (n = 0; n < 4; n++) {
        int len = RLEN(n), w = s->rvb_w[n];
        s->rvb_ph[n] += (0.6f + 0.37f * (float)n) * (float)S1C_RVB_DEC / s->sr; if (s->rvb_ph[n] >= 1.0f) s->rvb_ph[n] -= 1.0f;
        float tri = s->rvb_ph[n] < 0.5f ? 4.0f * s->rvb_ph[n] - 1.0f : 3.0f - 4.0f * s->rvb_ph[n];
        float rp = (float)w - (float)s->rvb_ln[n] - (float)RVB_DEPTH * (1.0f + tri) - 1.0f;
        if (rp < 0.0f) rp += (float)len;
        int i1 = (int)rp; float fr = rp - (float)i1;
        int i0 = i1 ? i1 - 1 : len - 1, i2 = i1 + 1 == len ? 0 : i1 + 1, i3 = i2 + 1 == len ? 0 : i2 + 1;
        float a0 = RGET(i0), a1 = RGET(i1), a2 = RGET(i2), a3 = RGET(i3);
        float c1 = 0.5f * (a2 - a0), c2 = a0 - 2.5f * a1 + 2.0f * a2 - 0.5f * a3, c3 = 0.5f * (a3 - a0) + 1.5f * (a1 - a2);
        float v = ((c3 * fr + c2) * fr + c1) * fr + a1;
        v *= s->rvb_g[n];
        s->rvb_lp[n] = v + s->rvb_d[n] * (s->rvb_lp[n] - v);
        y[n] = s->rvb_lp[n];
#if !defined S1C_RVB_BFP
        b += len;
#endif
    }
    /* Householder-style orthogonal mix (H4 / 2) back into the lines */
    float h0 = 0.5f * (y[0] + y[1] + y[2] + y[3]), h1 = 0.5f * (y[0] - y[1] + y[2] - y[3]);
    float h2 = 0.5f * (y[0] + y[1] - y[2] - y[3]), h3 = 0.5f * (y[0] - y[1] - y[2] + y[3]);
    float x[4] = {h0 + il, h1 + ir, h2 + il, h3 + ir};
#if defined S1C_RVB_BFP
    for (n = 0; n < 4; n++) {
        int len = RLEN(n), w = s->rvb_w[n];
        rb_put(s, n, w, x[n]);
        s->rvb_w[n] = w + 1 == len ? 0 : w + 1;
    }
#else
    b = s->rvb;
    for (n = 0; n < 4; n++) {
        int len = RLEN(n), w = s->rvb_w[n];
        b[w] = RVB_PUT(x[n]);
        s->rvb_w[n] = w + 1 == len ? 0 : w + 1;
        b += len;
    }
#endif
#undef RLEN
#undef RGET
    *ol = (y[0] + y[2]) * 0.35f * 2.0f * FITV(s)[0];
    *or_ = (y[1] + y[3]) * 0.35f * 2.0f * FITV(s)[0];
}
#endif
static void param_dirty(s1c *s)
{
#if S1C_BUSFX || S1C_RVB
    (void)s;
#else
    float fb = P(RVFB);
    s->rv_g = fb > 0.0f ? powf(fb, 1.0f / (float)S1C_RV_SCALE) : 0.0f;
#endif
}

void s1c_set(s1c *s, unsigned id, float v)
{
    if (id >= 150) return;
    s->t[id] = v;
#if !S1C_CSM
    if (!smoothed(id)) s->c[id] = v;
#endif
    if (id == PORT) s->port_c2 = exp2f(-1.0f / (s->sr * v));   /* 0.5^(1 / (sr v)) */
}
void s1c_init(s1c *s, float sr, const float init[150])
{
    int i;
#if !S1C_BUSFX
    int n, off = 0;
#endif
    memset(s, 0, sizeof *s);
#if S1C_FAST
    s1c_nsm = 0; for (i = 0; i < 150; i++) if (smoothed((unsigned)i)) s1c_sm[s1c_nsm++] = (uint8_t)i;
#endif
    s->sr = sr; s->rnd = 0x1234567u;
#if S1C_CSM
    for (i = 0; i < 150; i++) s->t[i] = init ? init[i] : 0.0f;   /* NULL: the caller sets every target */
#else
    for (i = 0; i < 150; i++) { s->t[i] = init[i]; s->c[i] = smoothed((unsigned)i) ? 0.0f : init[i]; }
#endif
    s->port_c2 = init ? powf(0.5f, 1.0f / (sr * init[PORT])) : 0.0f;
    for (i = 0; i < S1C_VOICES; i++) s->v[i].root = -1;
#if !S1C_BUSFX
#if S1C_EXT
    memcpy(s->rfit, s1c_rvb_fit, sizeof s->rfit);
    s->dfmt = s1c_dlfmt == 1 ? 2 : s1c_dlfmt;      /* device lines: h16 or 8-bit block float */
#else
    for (i = 0; i < 4; i++) { s->dl[i].buf = s->dbuf[i]; s->dl[i].len = S1C_DLY_LEN; }
#endif
#ifdef S1C_OPT_A
    s->ddec = 0;
#else
    s->ddec = 1;
#endif
#if !S1C_RVB
    for (n = 0; n < 8; n++) {
        s->rv_len[n] = (int32_t)(((float)rv_len44[n] / 44100.0f + rv_dev[n] * 1.125f) * sr / (S1C_RV_SCALE * S1C_RV_DEC)) + 4;
        off += s->rv_len[n];
        s->rv_seed[n] = rv_seed0[n];
        s->rv_del[n] = rv_target(s, n);
        rv_next(s, n);
    }
    (void)off;
    float d = 2.0f - cosf(3.14159265f), damp = d - sqrtf(d * d - 1.0f);
    s->rv_damp = powf(damp, 1.0f / (float)S1C_RV_SCALE);
#else
    (void)n; (void)off;
#endif
#endif
#if S1C_RVB
    for (i = 0; i < 4; i++) s->rvb_ln[i] = (int32_t)(rvb_len48[i] * sr / 48000.0f);  /* line lengths given at sr / S1C_RVB_DEC */
    s->rvb_fb = -1.0f;
#if defined S1C_RVB_BFP && !S1C_EXT
    s->rvm = s->rvm_s; s->rve = s->rve_s; s->rvs = s->rvs_s;
#endif
#endif
#if S1C_KR > 1
#endif
    s->mono_prev = 0;
    s->lfo_c2 = powf(0.5f, 1.0f / (0.0053125f * sr));
    s->monof = 261.625579833984375f;   /* the mono glide aims at middle C before any note */
    s->wlen = (int32_t)(0.05f * sr);
    if (s->wlen > S1C_WIDEN_MAX) s->wlen = S1C_WIDEN_MAX;
}

#if S1C_EXT
#define ENV_PUT(q, i, v) (s->env[(q) * s->e_cap + (i)] = h16_put(v))
#define ENV_GET(q, i) h16_get(s->env[(q) * s->e_cap + (i)])
#ifndef S1C_EXT_EB
#define S1C_EXT_EB 16
#endif
/* Carve the delay lines and HF envelope rings from the pool: the lowest decimation whose exact line
 * lengths (2T, 2T unless mono, T; +3, block-aligned) and rings fit. Clears every line. */
static void dly_partition(s1c *s)
{
#if S1C_DL_H16
    const int fmt = 0; int nl, dd, i;
#else
    int fmt = s->dfmt, nl, dd, i;
#endif
    float dt = s->t[DLT] > P(DLT) ? s->t[DLT] : P(DLT);
    float Tt = dt * s->sr * s->t[DLON];
    s->dmono = s->t[PANA] == 0.0f && s->t[PHMIX] == 0.0f;
    nl = s->dmono ? 2 : 3;
    for (dd = 1; dd < 4096; dd++) {
        float T = Tt / (float)dd;
        int la = ((int)(2.0f * T) + 3 + 15) & ~15, lc = ((int)T + 3 + 15) & ~15;
#if S1C_HFR
        int el = (int)(2.0f * T) / S1C_EXT_EB + 6;
#else
        int el = 0;
#endif
        int samples = la * (nl - 1) + lc;
        int bytes = (fmt == 2 ? samples + samples / 16 : 2 * samples) + 3 * el * 2;
        if (bytes <= s->pool_bytes || dd == 4095) {
            uint8_t *p = s->pool; int lens[4] = {la, nl == 3 ? la : 0, 0, lc};
            memset(s->pool, 0, (size_t)s->pool_bytes);
#if S1C_HFR
            s->env = (uint16_t *)p; s->e_cap = el; s->e_len = el; p += 3 * el * 2;
#endif
            for (i = 0; i < 4; i++) {
                s1c_dline *d = &s->dl[i];
                memset(d, 0, sizeof *d);
                d->len = lens[i] ? lens[i] : 16;
                if (!lens[i]) continue;
                if (fmt == 2) { d->m = (int8_t *)p; p += lens[i]; d->ex = (int8_t *)p; p += lens[i] / 16; }
                else { d->buf = (int16_t *)(void *)p; p += 2 * lens[i]; }
            }
            break;
        }
    }
    s->ddec = dd; s->dk = 0; s->da[0] = s->da[1] = s->d0[0] = s->d0[1] = s->d1[0] = s->d1[1] = 0.0f;
#if S1C_HFR
    s->dd_set = 0; s->e_w = s->e_bn = 0;
    for (i = 0; i < 3; i++) s->e_bacc[i] = s->e_out[i] = 0.0f;
#endif
}
void s1c_attach(s1c *s, void *rvb, uint16_t *wbuf, uint8_t *pool, int pool_bytes)
{
#if defined S1C_RVB_BFP
    s->rvm = (int8_t *)rvb; s->rve = s->rvm + S1C_RVA_WORDS; s->rvs = (int16_t *)(void *)(s->rve + S1C_RVA_WORDS / 16);
#else
    s->rvb = rvb;
#endif
    s->wbuf = wbuf; s->pool = pool; s->pool_bytes = pool_bytes & ~3;
    memset(rvb, 0, S1C_RVB_BYTES); memset(wbuf, 0, S1C_WAP_WORDS * 2);
    s->ddec = 0;
}
#else
#define ENV_PUT(q, i, v) (s->env[q][i] = (v))
#define ENV_GET(q, i) s->env[q][i]
#endif

void s1c_render(s1c *s, float *outl, float *outr, unsigned n)
{
    unsigned f, i;
    int mono = s->t[MONO] > 0.0f;
    if (mono) { if (s->v[0].stage == 2 && s->v[0].amp < 0.01f) voice_clear(&s->v[0]); }
    else for (i = 0; i < S1C_VOICES; i++) if (s->v[i].stage == 2 && s->v[i].amp < 0.01f) voice_clear(&s->v[i]);
    float c2 = s->port_c2, c1 = 1.0f - c2;
    param_dirty(s);
    s1c_cc cm, cr, cw;   /* compressor parameters are not smoothed: coefficients once per block */
    comp_coef(&cm, P(CMR), P(CMT), P(CMA), P(CMREL), s->sr);
#if !S1C_BUSFX
#if !S1C_NO_RVCOMP
    comp_coef(&cr, P(CRR), P(CRT), P(CRA), P(CRREL), s->sr);
    comp_coef(&cw, P(CWR), P(CWT), P(CWA), P(CWREL), s->sr);
#endif
#endif
    (void)cr; (void)cw;
    for (f = 0; f < n; f++) {
#if S1C_FAST
        comp_upd = (f & 3) == 0;
#if S1C_CSM
        for (i = 0; i < (int)sizeof s1c_smid; i++) s->c[i] = c1 * s->t[s1c_smid[i]] + c2 * s->c[i];
#else
        for (i = 0; i < s1c_nsm; i++) { unsigned q = s1c_sm[i]; s->c[q] = c1 * s->t[q] + c2 * s->c[q]; }
#endif
#else
        for (i = 0; i < 150; i++) if (smoothed(i)) s->c[i] = c1 * s->t[i] + c2 * s->c[i];
#endif
        float gh = P(GLIDE);
        if (gh != s->glide_h) { s->glide_h = gh; s->glide_c2 = gh > 0.0f ? powf(0.5f, 1.0f / (s->sr * gh)) : 0.0f; }
        s->monos = (1.0f - s->glide_c2) * s->monof + s->glide_c2 * s->monos;
        if (P(MONO) != (float)s->mono_prev) {
            s->mono_prev = (uint8_t)P(MONO);
            for (i = 0; i < S1C_VOICES; i++) voice_clear(&s->v[i]);
            s->nlast = 0; s->prev_held = 0;
#if !S1C_BUSFX
#if S1C_EXT
            s->ddec = 0;     /* re-partition (clears the lines) */
#else
            for (i = 0; i < 4; i++) { memset(s->dl[i].buf, 0, sizeof s->dbuf[0]); s->dl[i].prev = 0.0f; }
#endif
#endif
        }
        /* LFOs */
        float x;
        s->l1ph += P(L1R) / s->sr; if (s->l1ph >= 1.0f) s->l1ph -= 1.0f;
        s->l2ph += P(L2R) / s->sr; if (s->l2ph >= 1.0f) s->l2ph -= 1.0f;
        float lc2 = s->lfo_c2;
        for (i = 0; i < 2; i++) {
            float ph = i ? s->l2ph : s->l1ph; int w = (int)(i ? P(L2I) : P(L1I));
#if S1C_FAST
            x = w == 0 ? sine_t((uint32_t)(ph * 4294967296.0f)) :
#else
            x = w == 0 ? sinf(ph * TWO_PI) :
#endif
                w == 1 ? (ph > 0.5f ? 1.0f : -1.0f) : w == 2 ? (ph - 0.5f) * 2.0f : (0.5f - ph) * 2.0f;
            float *sm = i ? &s->l2s : &s->l1s;
            *sm = (1.0f - lc2) * x + lc2 * *sm;
            float a = i ? P(L2A) : P(L1A), *o = i ? s->l0 : s->l1;
            o[0] = 0.5f * (1.0f + *sm) * a; o[1] = 1.0f - 0.5f * (1.0f - *sm) * a;
        }
#if !S1C_NO_SEQ
        sequencer(s);
#endif
        float y = 0.0f;
        if (P(MONO) > 0.0f) { if (s->v[0].root != -1 && s->v[0].stage) y = voice_run(s, &s->v[0]); }
        else for (i = 0; i < S1C_VOICES; i++) if (s->v[i].root != -1 && s->v[i].stage) y += voice_run(s, &s->v[i]);

        /* bitcrush (sample-and-hold), tremolo */
#if S1C_FAST
        if ((f & 7) == 0) {
            float bsr = log2f(P(BCSR)) + 4.0f * lfo01(s, P(LBC));
            bsr = clampf(exp2f(bsr), 2048.0f, 48000.0f);
            s->bc_inc = s->sr / bsr; if (s->bc_inc < 1.0f) s->bc_inc = 1.0f;
        }
        float inc = s->bc_inc;
#else
        float bsr = log2f(P(BCSR)) + 4.0f * lfo01(s, P(LBC));
        bsr = clampf(exp2f(bsr), 2048.0f, 48000.0f);
        float inc = s->sr / bsr; if (inc < 1.0f) inc = 1.0f;
#endif
        if (s->bc_idx <= s->bc_n) { s->bc_v = y; s->bc_idx += inc - s->bc_n; s->bc_n = 0.0f; } else y = s->bc_v;
        s->bc_n += 1.0f;
        y *= lfo10(s, P(LTREM));
        /* autopan, equal power */
#if S1C_FAST
        float pan = sine_t((uint32_t)(s->pan_ph * 4294967296.0f)) * P(PANA);
        s->pan_ph += P(PANF) / s->sr; if (s->pan_ph >= 1.0f) s->pan_ph -= 1.0f;
        uint32_t pa = (uint32_t)((1.0f + pan) * 0.125f * 4294967296.0f);   /* angle / 2pi */
        float L = y * sine_t(pa + 0x40000000u), R = y * sine_t(pa);
#else
        float pan = sinf(s->pan_ph * TWO_PI) * P(PANA);
        s->pan_ph += P(PANF) / s->sr; if (s->pan_ph >= 1.0f) s->pan_ph -= 1.0f;
        float ang = (1.0f + pan) * 0.785398163f, L = y * cosf(ang), R = y * sinf(ang);
#endif
        /* phaser */
        float pm = P(PHMIX);
        if (pm != 0.0f) {
            float sl = sinf(s->ph_ph), cl = cosf(s->ph_ph);
            s->ph_ph += 0.10472f * P(PHRATE) / s->sr;
            if (s->ph_ph > TWO_PI) s->ph_ph -= TWO_PI;
            float pl = phaser_ch(s->ph_rec[0], &s->ph_out[0][0], L, sl, P(PHW), P(PHFB), s->sr);
            float pr = phaser_ch(s->ph_rec[1], &s->ph_out[1][0], R, cl, P(PHW), P(PHFB), s->sr);
            L = (1.0f - pm) * L + pm * pl; R = (1.0f - pm) * R + pm * pr;
        }
#if !S1C_BUSFX
        /* ping-pong delay with tracking low-pass on its input */
        float dL = L, dR = R;
        if (P(FTYPE) == 0.0f) {
#if S1C_FAST
            if ((f & 7) == 0) {
                float pv = clampf(log2f(P(CUT)), 10.0f, 14.4284f);
                float pc = 1.0f + (pv - 10.0f) * (1.0f / 4.4284f) * (P(DICT) - 1.0f);
                s->dl_fc = clampf(P(CUT) * pc, 64.0f, 22050.0f);
            }
            float fc = s->dl_fc;
#else
            float pv = clampf(log2f(P(CUT)), 10.0f, 14.4284f);
            float pc = 1.0f + (pv - 10.0f) * (1.0f / 4.4284f) * (P(DICT) - 1.0f);
            float fc = clampf(P(CUT) * pc, 64.0f, 22050.0f);
#endif
            dL = moog(s->dlp[0], L, fc, 0.0f, s->sr); dR = moog(s->dlp[1], R, fc, 0.0f, s->sr);
        }
#ifdef S1C_OPT_A
#if S1C_EXT
        if (s->ddec == 0) dly_partition(s);
        if (0) {
#else
        if (s->ddec == 0) {   /* fit 6T line samples (8-bit) into what the reverb leaves of 158,400 B */
#endif
#ifndef S1C_DLCAP
#define S1C_DLCAP (158400.0f - 2.0f * 28500.0f)
#endif
            float cap = S1C_DLCAP;
            #ifdef S1C_ADPCM
            #if S1C_DLMONO     /* no autopan and no phaser: L and R delay inputs are identical, line B == line A */
            s->dmono = s->t[PANA] == 0.0f && s->t[PHMIX] == 0.0f;
            float need = (s->dmono ? 3.0f : (float)S1C_DLN) * s->t[DLT] * s->sr * s->t[DLON] * s1c_dlbytes[s1c_dlfmt];
#else
            float need = S1C_DLN * s->t[DLT] * s->sr * s->t[DLON] * s1c_dlbytes[s1c_dlfmt];
#endif
#else
            float need = 6.0f * s->t[DLT] * s->sr * s->t[DLON] * S1C_DLB;
#endif
            int want = (int)(need / cap) + 1; if (want > 256) want = 256;
            s->ddec = want;
        }
#endif
        int dd = s->ddec;
#ifndef S1C_DL_FBK
#define S1C_DL_FBK 0.0f
#endif
#ifndef S1C_DL_FBMAX
#define S1C_DL_FBMAX 0.97f
#endif
#ifndef S1C_DL_WET
#define S1C_DL_WET 1.0f
#endif
        /* compact lines lose level per repeat (decimation, interpolation): feedback and wet trim, by ear */
        float T = P(DLT) * s->sr / (float)dd, fbk = P(DLFB) * (1.0f + S1C_DL_FBK * (float)(dd > 1)), oL, oRR;
        if (fbk > S1C_DL_FBMAX) fbk = S1C_DL_FBMAX;
        if (dd > 1) {
        /* lines run at sr / S1C_DLY_DEC: box-filtered input, linearly interpolated output */
#if S1C_DLAA
        if (s->aa_dd != dd) { s->aa_dd = dd; bq_lp(&s->aa, S1C_DLAA * 0.01f * s->sr / (float)dd, s->sr); }
        dL = bq_run(&s->aa, s->zaa[0], dL); dR = bq_run(&s->aa, s->zaa[1], dR);
#endif
        s->da[0] += dL; s->da[1] += dR;
#if S1C_HFR
        if (s->dd_set != dd) { s->dd_set = dd; bq_hp(&s->hd, S1C_HFR_FC * s->sr / (float)dd, s->sr); s->hd_norm = bq_norm(s, &s->hd);
#if S1C_EXT
                               s->e_eb = S1C_EXT_EB;
#else
                               s->e_eb = S1C_HFR_EB;
#endif
#if defined S1C_ADPCM && !S1C_EXT
                               s->e_eb *= s1c_dlfmt == 1 ? 4 : s1c_dlfmt == 2 ? 2 : 1;   /* ~same ring bytes for every format */
#endif
#if S1C_EXT
                               ; }
#else
                               s->e_len = S1C_DLY_LEN / s->e_eb + 4; }
#endif
        float hA = bq_run(&s->hd, s->za[0], dL), hB = bq_run(&s->hd, s->za[1], dR);
        s->e_acc[0] += hA * hA; s->e_acc[1] += hB * hB;
#endif
        if (++s->dk >= dd) {
            float iL = s->da[0] / (float)dd, iR = s->da[1] / (float)dd;
            s->dk = 0; s->da[0] = s->da[1] = 0.0f; s->d0[0] = s->d1[0]; s->d0[1] = s->d1[1];
#ifdef S1C_DLCOMP
            float lc = 1.5f - 0.5f / (float)dd;
            s->d1[0] = dl_tick_c(&s->dl[0], iL, 2.0f * T, fbk, lc);
            float r = dl_tick_c(&s->dl[1], iR, 2.0f * T, fbk, 0.0f), f2 = dl_tick_c(&s->dl[2], iR, T, fbk, lc);
            s->d1[1] = dl_tick_c(&s->dl[3], r, T, fbk, lc) + f2;
#else
            s->d1[0] = DL_TICK(&s->dl[0], iL, 2.0f * T, fbk);
#if S1C_DLN == 5      /* C_T(R) + C_T(C_2T(R)) == C_T(R + C_2T(R)): one T line instead of two */
#if S1C_DLMONO
            float r = s->dmono ? s->d1[0] : DL_TICK(&s->dl[1], iR, 2.0f * T, fbk);
#else
            float r = dl_tick(&s->dl[1], iR, 2.0f * T, fbk);
#endif
            s->d1[1] = DL_TICK(&s->dl[3], iR + r, T, fbk);
#else
            float r = dl_tick(&s->dl[1], iR, 2.0f * T, fbk), f2 = dl_tick(&s->dl[2], iR, T, fbk);
            s->d1[1] = dl_tick(&s->dl[3], r, T, fbk) + f2;
#endif
#endif
        }
        { float u = (float)s->dk / (float)dd;
          oL = s->d0[0] + (s->d1[0] - s->d0[0]) * u; oRR = s->d0[1] + (s->d1[1] - s->d0[1]) * u; }
#if S1C_DLAA
        oL = bq_run(&s->aa, s->zaa[2], oL); oRR = bq_run(&s->aa, s->zaa[3], oRR);
#endif
#if S1C_HFR
        if (s->dk == 0) {    /* one line tick just ran: HF energy through the same comb structure, in energy */
            float inv = 1.0f / (float)dd, f2 = fbk * fbk, ein[3];
            ein[0] = s->e_acc[0] * inv; ein[1] = s->e_acc[1] * inv; ein[2] = ein[1] + s->e_out[1];
            s->e_acc[0] = s->e_acc[1] = 0.0f;
            for (int q = 0; q < 3; q++) s->e_bacc[q] += ein[q] + f2 * s->e_out[q];
            if (++s->e_bn == s->e_eb) {
                for (int q = 0; q < 3; q++) { ENV_PUT(q, s->e_w, s->e_bacc[q] / (float)s->e_eb); s->e_bacc[q] = 0.0f; }
                s->e_bn = 0; if (++s->e_w == s->e_len) s->e_w = 0;
            }
            float now = (float)s->e_w + (float)s->e_bn / (float)s->e_eb - 0.5f, last = (float)s->e_w - 1.0f;
            for (int q = 0; q < 3; q++) {
                float p = now - (q < 2 ? 2.0f * T : T) / (float)s->e_eb;
                if (p > last) p = last;          /* never the block still being filled */
                while (p < 0.0f) p += (float)s->e_len;
                int i0 = (int)p; float fr = p - (float)i0; int i1 = i0 + 1 == s->e_len ? 0 : i0 + 1;
                { float v0 = ENV_GET(q, i0), v1 = ENV_GET(q, i1); s->e_out[q] = v0 + fr * (v1 - v0); }
            }
        }
#if S1C_HFR == 2
        /* excitation: the input's own HF now (right spectrum), decorrelated by short allpasses,
         * scaled to the delayed HF energy over the input's current HF energy */
        {   float hx[2] = {hA, hB};
            for (int q = 0; q < 2; q++) {
                float e = hx[q] * hx[q], *en = &s->en[q];
                *en += (e > *en ? S1C_HFR_ATT : S1C_HFR_REL) * (e - *en);
                float v = hx[q];
                for (int a = 0; a < 2; a++) {
                    static const int16_t al[2][2] = {{23, 41}, {29, 37}};
#if S1C_EXT
                    static const uint8_t ao[2][2] = {{0, 23}, {64, 93}};
                    uint16_t *buf = s->hap + ao[q][a]; int w = s->hapw[q][a];
                    float d = h16_get(buf[w]), u = v + 0.6f * d; buf[w] = h16_put(u); v = d - 0.6f * u;
#else
                    float *buf = s->hap[q][a]; int w = s->hapw[q][a];
                    float d = buf[w], u = v + 0.6f * d; buf[w] = u; v = d - 0.6f * u;
#endif
                    s->hapw[q][a] = w + 1 == al[q][a] ? 0 : w + 1;
                }
                float g;
                if ((f & 7) == 0) {
                    g = *en > 1e-14f ? sqrtf(s->e_out[q ? 2 : 0] / *en) : 0.0f;
                    if (g > S1C_HFR_GMAX) g = S1C_HFR_GMAX;
                    s->hg[q] = g;
                } else g = s->hg[q];
                if (q) oRR += g * v; else oL += g * v;
            }
        }
#else
        oL += sqrtf(s->e_out[0]) * s->hd_norm * bq_run(&s->hd, s->zs[0], hfr_noise(s));
        oRR += sqrtf(s->e_out[2]) * s->hd_norm * bq_run(&s->hd, s->zs[1], hfr_noise(s));
#endif
#endif
        } else {
        oL = DL_TICK(&s->dl[0], dL, 2.0f * T, fbk);
        #if S1C_DLMONO
        float oR = s->dmono ? oL : DL_TICK(&s->dl[1], dR, 2.0f * T, fbk);
#else
        float oR = dl_tick(&s->dl[1], dR, 2.0f * T, fbk);
#endif
#if S1C_DLN == 5
        oRR = DL_TICK(&s->dl[3], dR + oR, T, fbk);
#else
        float oF = dl_tick(&s->dl[2], dR, T, fbk);
        oRR = dl_tick(&s->dl[3], oR, T, fbk) + oF;
#endif
        }
        float dm = P(DLMIX) * P(DLON);
        if (dd > 1) { oL *= S1C_DL_WET; oRR *= S1C_DL_WET; }
        float mL = (1.0f - dm) * L + dm * oL, mR = (1.0f - dm) * R + dm * oRR;
        /* reverb: high-pass, pre-gain, input compressor, network, wet compressor */
#if S1C_FAST
        if ((f & 7) == 0) {
            float cc = tanf(3.14159265f * P(RVHP) / s->sr), a1 = 1.0f / (1.0f + 1.41421356f * cc + cc * cc);
            s->hpk[0] = a1; s->hpk[1] = 2.0f * (cc * cc - 1.0f) * a1; s->hpk[2] = (1.0f - 1.41421356f * cc + cc * cc) * a1;
        }
        float hl, hr;
        { float *yy = s->hp_y[0], t = mL - s->hpk[1] * yy[0] - s->hpk[2] * yy[1]; hl = (t - 2.0f * yy[0] + yy[1]) * s->hpk[0] * 2.0f; yy[1] = yy[0]; yy[0] = t; }
        { float *yy = s->hp_y[1], t = mR - s->hpk[1] * yy[0] - s->hpk[2] * yy[1]; hr = (t - 2.0f * yy[0] + yy[1]) * s->hpk[0] * 2.0f; yy[1] = yy[0]; yy[0] = t; }
#else
        float hl = buthp(s->hp_y[0], mL, P(RVHP), s->sr) * 2.0f, hr = buthp(s->hp_y[1], mR, P(RVHP), s->sr) * 2.0f;
#endif
#if !S1C_NO_RVCOMP
        hl = comp(s->cmp[1][0], hl, &cr) * P(CRG); hr = comp(s->cmp[1][1], hr, &cr) * P(CRG);
#endif
        float wl, wr;
#if S1C_RVB
#if S1C_RVB_DEC > 1
        s->ra[0] += hl; s->ra[1] += hr;
        if (++s->rk >= S1C_RVB_DEC) {
            s->rk = 0; s->r0[0] = s->r1[0]; s->r0[1] = s->r1[1];
            rvb_compact(s, s->ra[0] * (1.0f / S1C_RVB_DEC), s->ra[1] * (1.0f / S1C_RVB_DEC), &s->r1[0], &s->r1[1]);
            s->ra[0] = s->ra[1] = 0.0f;
        }
        { float u = (float)s->rk * (1.0f / S1C_RVB_DEC);
          wl = s->r0[0] + (s->r1[0] - s->r0[0]) * u; wr = s->r0[1] + (s->r1[1] - s->r0[1]) * u; }
#else
        rvb_compact(s, hl, hr, &wl, &wr);
#endif
#else
        reverb(s, hl, hr, &wl, &wr);
#endif
#if S1C_HFR && S1C_RVB_DEC > 1 && S1C_HFR_RV
        if (s->rvb_fbt != s->t[RVFB] || s->hr_norm == 0.0f) {
            float fb = s->t[RVFB]; s->rvb_fbt = fb;
            s->rt_a = fb > 0.0f ? powf(fb, 2.0f / (FITV(s)[1] * 0.068390f * s->sr)) : 0.0f;
            bq_hp(&s->hr, S1C_HFR_FC * s->sr / (float)S1C_RVB_DEC, s->sr); s->hr_norm = bq_norm(s, &s->hr);
        }
        { float a = s->rt_a, b = 1.0f - a;
          float xl = bq_run(&s->hr, s->zra[0], hl), xr = bq_run(&s->hr, s->zra[1], hr);
          float ehf = 0.5f * (xl * xl + xr * xr), ein = 0.5f * (hl * hl + hr * hr) - ehf, eo = 0.5f * (wl * wl + wr * wr);
          s->rt_in = a * s->rt_in + b * (ein > 0.0f ? ein : 0.0f); s->rt_out = a * s->rt_out + b * eo; s->rt_hfin = a * s->rt_hfin + b * ehf;
          float g = s->rt_in > 1e-20f ? sqrtf(s->rt_out / s->rt_in * s->rt_hfin) * s->hr_norm * S1C_HFR_RG : 0.0f;
          wl += g * bq_run(&s->hr, s->zrs[0], hfr_noise(s)); wr += g * bq_run(&s->hr, s->zrs[1], hfr_noise(s)); }
#endif
#if !S1C_NO_RVCOMP
        wl = comp(s->cmp[2][0], wl, &cw) * P(CWG); wr = comp(s->cmp[2][1], wr, &cw) * P(CWG);
#endif
        float rm = P(RVMIX) * P(RVON) * lfo10(s, P(LRVMIX));
        L = ((1.0f - rm) * mL + rm * wl) * 2.0f * P(MVOL);
        R = ((1.0f - rm) * mR + rm * wr) * 2.0f * P(MVOL);
#else
        /* device: delay and reverb are the shared bus; the part glue reads these send levels */
        s->send_dly = P(DLMIX) * P(DLON);
        s->send_rev = P(RVMIX) * P(RVON) * lfo10(s, P(LRVMIX));
        L *= 2.0f * P(MVOL); R *= 2.0f * P(MVOL);
#endif
        L = comp(s->cmp[0][0], L, &cm) * P(CMG); R = comp(s->cmp[0][1], R, &cm) * P(CMG);
        /* widen: the right channel crossfades to itself 50 ms late */
        {
#if WAP_L0
            static const int16_t wl[3] = {WAP_L0, WAP_L1, WAP_L2};
            float wd = R; uint16_t *wb = s->wbuf;
            for (int q = 0; q < 3; q++) {            /* Schroeder allpass, g = WAP_G */
                int w = s->wap_w[q]; float d = h16_get(wb[w]), v = wd + WAP_G * d;
                wb[w] = h16_put(v); wd = d - WAP_G * v;
                s->wap_w[q] = w + 1 == wl[q] ? 0 : w + 1; wb += wl[q];
            }
#elif defined S1C_WIDEN_FLOAT
            float wd = s->wbuf[s->wpos]; s->wbuf[s->wpos] = R;
#else
            float wd = h16_get(s->wbuf[s->wpos]); s->wbuf[s->wpos] = h16_put(R);
#endif
#if !WAP_L0
            if (++s->wpos >= s->wlen) s->wpos = 0;
#endif
            R += P(WIDEN) * (wd - R);
        }
        outl[f] = L; outr[f] = R;
    }
}
