/* SPDX-License-Identifier: GPL-3.0-only */
/* Synth One shared units (see s1u.h). The control laws are the compact engine's; moving them here
 * changes no output (checked bit-exact against the inline versions on the device configuration). */
#include "s1u.h"
#if defined(X0X_S1_AUTOPAN_PROBE) && X0X_S1_AUTOPAN_PROBE
#if X0X_S1_AUTOPAN_PORT
#include "s1_pan_port.h"
#else
#include <math.h>
#endif
#endif
#ifdef X0X_S1C
#include "s1c_config.h"
#endif
#ifdef S1C_FASTMATH
#if __has_include("../fastmath.h")
#include "../fastmath.h"
#else
#include "fastmath.h"
#endif
#define expf fm_expf
#define exp2f fm_exp2f
#define log2f fm_log2f
#define powf fm_powf
#define sinf fm_sinf
#define cosf fm_cosf
#define fabsf fm_fabsf
#else
#include <math.h>
#endif
#ifndef RVB_DEPTH
#define RVB_DEPTH 4
#endif
#ifndef WAP_G
#define WAP_G 0.6f
#endif
#define S1U_TWO_PI 6.28318530718f

/* ---- ADSR ---- */
static float pole(float t, float sr) { return t <= 0.0f ? 0.0f : expf(-1.0f / (t * sr)); }
float s1u_env_step(s1u_env *e, float gate, float atk, float dec, float sus, float rel, float sr)
{
    if (e->prev < gate && e->mode != S1U_E_DEC) { e->mode = S1U_E_ATK; e->a = pole(atk * 0.6f, sr); }
    else if (e->prev > gate) { e->mode = S1U_E_REL; e->a = pole(rel, sr); }
    e->prev = gate;
    if (e->mode == S1U_E_CLEAR) return 0.0f;
    float x = e->mode == S1U_E_ATK ? gate : gate * sus;
    e->y = (1.0f - e->a) * x + e->a * e->y;
    if (e->mode == S1U_E_ATK && e->y > 0.99f) { e->mode = S1U_E_DEC; e->a = pole(dec, sr); }
    return e->y;
}

/* ---- table sine ---- */
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
float s1u_sin(uint32_t ph)
{
    uint32_t i = ph >> 24;
    float f = (float)(ph & 0xFFFFFFu) * (1.0f / 16777216.0f);
    float a = sin_at(i);
    return a + f * (sin_at(i + 1u) - a);
}

/* ---- LFO ---- */
void s1u_lfo_tick(s1u_lfo *l, float rate_hz, int shape, float amount, float c2, float sr)
{
    l->ph += rate_hz / sr; if (l->ph >= 1.0f) l->ph -= 1.0f;
    float ph = l->ph;
    float x = shape == 0 ? s1u_sin((uint32_t)(ph * 4294967296.0f)) :
              shape == 1 ? (ph > 0.5f ? 1.0f : -1.0f) : shape == 2 ? (ph - 0.5f) * 2.0f : (0.5f - ph) * 2.0f;
    l->sm = (1.0f - c2) * x + c2 * l->sm;
    l->o01 = 0.5f * (1.0f + l->sm) * amount; l->o10 = 1.0f - 0.5f * (1.0f - l->sm) * amount;
}

/* ---- filters ---- */
float s1u_ladder_a(float x)
{
    return 1.0f - x * (1.56874466f + x * (-1.73140822f + x * (1.55879259f + x * (-0.711601815f + x * 0.118475673f))));
}
float s1u_ladder(float *h, float in, float A, float fb)
{
    float o = 0.0f;
    for (int k = 0; k < 2; k++) {
        float tmp = h[3], v = in - fb * 0.5f * (h[3] + h[4]);
        float z1 = v + A * (h[0] - v), z2 = z1 + A * (h[1] - z1), z3 = z2 + A * (h[2] - z2), z4 = z3 + A * (h[3] - z3);
        h[0] = z1; h[1] = z2; h[2] = z3; h[3] = z4; h[4] = tmp;
        o += z4;
    }
    return 0.5f * o;
}
float s1u_hp2(float *z, float x, float a1, float a4, float a5)
{
    float t = x - a4 * z[0] - a5 * z[1], o = (t - 2.0f * z[0] + z[1]) * a1;
    z[1] = z[0]; z[0] = t;
    return o;
}

/* ---- compressor ---- */
void s1u_comp_coef(s1u_cc *k, float ratio, float thr, float atk, float rel, float sr)
{
    k->s1 = atk > 0.0f ? expf(-2.0f / (sr * atk)) : 0.0f; k->s3 = atk > 0.0f ? expf(-1.0f / (sr * atk)) : 0.0f;
    k->s4 = rel > 0.0f ? expf(-1.0f / (sr * rel)) : 0.0f; k->s2 = (1.0f - k->s1) * (1.0f / ratio - 1.0f);
    k->thr = thr;
}
float s1u_comp(float *st, float x, const s1u_cc *k, int upd)
{
    float ax = fabsf(x), a = st[0] > ax ? k->s4 : k->s3;
    st[0] = st[0] * a + (1.0f - a) * ax;
    if (upd) {
        float over = st[0] > 0.0f ? 6.02059991f * log2f(st[0]) - k->thr : 0.0f;
        st[2] = over > 0.0f ? over : 0.0f;
        st[3] = st[1] != 0.0f ? exp2f(0.166096405f * st[1]) : 1.0f;
    }
    st[1] = k->s1 * st[1] + k->s2 * st[2];
    return x * st[3];
}

/* ---- phaser ---- */
static float phaser_ch(float r[4][3], float *fb, float in, float lfo, float width, float fbk, float sr)
{
    float w = 0.5f * (S1U_TWO_PI * 800.0f - S1U_TWO_PI * 100.0f) * (1.0f - lfo) + S1U_TWO_PI * 100.0f;
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
void s1u_phaser_run(s1u_phaser *p, float *l, float *r, float pm, float rate, float width, float fbk, float sr)
{
    if (pm == 0.0f) return;
    float sl = sinf(p->ph), cl = cosf(p->ph);
    p->ph += 0.10472f * rate / sr;
    if (p->ph > S1U_TWO_PI) p->ph -= S1U_TWO_PI;
    float pl = phaser_ch(p->rec[0], &p->out[0], *l, sl, width, fbk, sr);
    float pr = phaser_ch(p->rec[1], &p->out[1], *r, cl, width, fbk, sr);
    *l = (1.0f - pm) * *l + pm * pl; *r = (1.0f - pm) * *r + pm * pr;
}

/* ---- bit-crush ---- */
void s1u_crush_clock(s1u_crush *c, float hz, float sr)
{
    c->inc = sr / hz; if (c->inc < 1.0f) c->inc = 1.0f;
}
float s1u_crush_run(s1u_crush *c, float y)
{
    if (c->idx <= c->n) { c->v = y; c->idx += c->inc - c->n; c->n = 0.0f; } else y = c->v;
    c->n += 1.0f;
    return y;
}

/* ---- auto-pan ---- */
void s1u_autopan(float *ph, float y, float rate, float amount, float sr, float *l, float *r)
{
    float pan = s1u_sin((uint32_t)(*ph * 4294967296.0f)) * amount;
    *ph += rate / sr; if (*ph >= 1.0f) *ph -= 1.0f;
    uint32_t pa = (uint32_t)((1.0f + pan) * 0.125f * 4294967296.0f);   /* angle / 2pi */
    *l = y * s1u_sin(pa + 0x40000000u); *r = y * s1u_sin(pa);
}

#if defined(X0X_S1_AUTOPAN_PROBE) && X0X_S1_AUTOPAN_PROBE
/* Same mono->stereo seam and equal-power law; store original Q24 phase
 * exactly in the existing float lane. Never reset phase at control edits. */
void s1u_autopan_authored(float *ph, float y, float rate, float amount, float sr, float *l, float *r)
{
    const uint32_t mask = 0xFFFFFFu;
    uint32_t a = (uint32_t)(*ph * 16777216.f) & mask;
    float pan = s1u_sin(a << 8) * amount;
    uint32_t inc;
#if X0X_S1_AUTOPAN_PORT
    float phase_scale = sr==44100.f ? 0x1.7c6f8c0000000p+8f : 16777216.f / sr;
    inc = s1_pan_q24_increment(rate,phase_scale);
#else
    inc = (uint32_t)lrintf(rate * (float)(16777216.0 / sr));
#endif
    *ph = (float)((a + inc) & mask) * (1.f / 16777216.f);
    uint32_t pa = (uint32_t)((1.f + pan) * 0.125f * 4294967296.f);
    *l = y*s1u_sin(pa + 0x40000000u);
    *r = y*s1u_sin(pa);
}
#endif

/* ---- half floats ---- */
typedef union { float f; uint32_t u; } s1u_fb;
uint16_t s1u_h16_put(float x)
{
    s1u_fb b; b.f = x;
    uint32_t a = (b.u & 0x7FFFFFFFu) + 0x1000u;          /* round the dropped 13 bits */
    uint32_t e = a >> 23;
    if (e < 113) return 0;
    if (e > 142) a = 142u << 23 | 0x7FFFFFu;
    return (uint16_t)((b.u >> 16 & 0x8000u) | ((a >> 13) - (112u << 10)));
}
float s1u_h16_get(uint16_t h)
{
    s1u_fb b;
    b.u = h & 0x7FFFu ? (uint32_t)(h & 0x8000u) << 16 | ((uint32_t)(h & 0x7FFFu) + (112u << 10)) << 13 : 0u;
    return b.f;
}

/* ---- widen ---- */
float s1u_widen_run(s1u_widen *wd, float r, float amount)
{
    float d0 = r; uint16_t *wb = wd->buf;
    for (int q = 0; q < 3; q++) {            /* Schroeder allpass, g = WAP_G */
        int w = wd->w[q]; float d = s1u_h16_get(wb[w]), v = d0 + WAP_G * d;
        wb[w] = s1u_h16_put(v); d0 = d - WAP_G * v;
        wd->w[q] = w + 1 == wd->len[q] ? 0 : w + 1; wb += wd->len[q];
    }
    return r + amount * (d0 - r);
}

/* ---- reverb ---- */
void s1u_rvb_init(s1u_rvb *r, const uint16_t len48[4], const uint16_t off[5], float sr)
{
    for (int i = 0; i < 4; i++) r->ln[i] = (int32_t)(len48[i] * sr / 48000.0f);
    r->off = off; r->fb = -1.0f;
    r->fit[0] = r->fit[1] = r->fit[2] = 1.0f;
}
void s1u_rvb_attach(s1u_rvb *r, void *mem)
{
    unsigned words = r->off[4];
    r->m = (int8_t *)mem; r->e = r->m + words; r->st = (int16_t *)(void *)(r->e + words / 16);
    uint8_t *p = (uint8_t *)mem;
    for (unsigned i = 0; i < words + words / 16 + 128; i++) p[i] = 0;
    for (int n = 0; n < 4; n++) { r->bn[n] = 0; r->w[n] = 0; r->lp[n] = 0.0f; }
}
/* 8-bit block-float lines: one exponent per 16 writes, the block being written kept at 16 bits */
static float rb_get(const s1u_rvb *r, int n, int i, int w)
{
    int idx = r->off[n] + i;
    if ((i >> 4) == (w >> 4) && (i & 15) < r->bn[n]) return (float)r->st[n * 16 + (i & 15)] * (1.0f / 2048.0f);
    int e = r->e[idx >> 4];
    if (e == -128) return 0.0f;
    return (float)r->m[idx] * (exp2f((float)e) / 127.0f);
}
static void rb_put(s1u_rvb *r, int n, int w, float x)
{
    int16_t *st = r->st + n * 16;
    float v = x * 2048.0f;
    st[w & 15] = (int16_t)(v > 32767.0f ? 32767.0f : v < -32768.0f ? -32768.0f : v);
    if (++r->bn[n] == 16) {
        int base = r->off[n] + (w & ~15), i; float pk = 0.0f;
        r->bn[n] = 0;
        for (i = 0; i < 16; i++) { float a = fabsf((float)st[i] * (1.0f / 2048.0f)); if (a > pk) pk = a; }
        if (pk > 0.0f) {
            float e = __builtin_ceilf(log2f(pk)), sc = exp2f(e) / 127.0f;
            for (i = 0; i < 16; i++) r->m[base + i] = (int8_t)__builtin_roundf((float)st[i] * (1.0f / 2048.0f) / sc);
            r->e[base >> 4] = (int8_t)e;
        } else r->e[base >> 4] = -128;
    }
}
void s1u_rvb_run(s1u_rvb *r, float fb, float il, float ir, float *ol, float *or_, float sr)
{
    float y[4];
    int n;
    if (fb != r->fb) {                    /* per-line gains when feedback changes */
        float lref = 0.068390f * sr, G = 0.70710678f;
        r->fb = fb;
        for (n = 0; n < 4; n++) {
            float q = (float)r->ln[n] / lref;
            r->g[n] = fb > 0.0f ? powf(fb, q / r->fit[1]) : 0.0f;
            float gn = powf(G, q * r->fit[2]);
            r->d[n] = (1.0f - gn) / (1.0f + gn);
        }
    }
    for (n = 0; n < 4; n++) {
        int len = r->off[n + 1] - r->off[n], w = r->w[n];
#if S1C_RVB_NOMOD   /* fixed taps */
        int ip = w - r->ln[n] - RVB_DEPTH - 1; if (ip < 0) ip += len;
        float v = rb_get(r, n, ip, w);
#else               /* taps swept by a slow triangle, cubic interpolation */
        r->ph[n] += (0.6f + 0.37f * (float)n) / sr; if (r->ph[n] >= 1.0f) r->ph[n] -= 1.0f;
        float tri = r->ph[n] < 0.5f ? 4.0f * r->ph[n] - 1.0f : 3.0f - 4.0f * r->ph[n];
        float rp = (float)w - (float)r->ln[n] - (float)RVB_DEPTH * (1.0f + tri) - 1.0f;
        if (rp < 0.0f) rp += (float)len;
        int i1 = (int)rp; float fr = rp - (float)i1;
        int i0 = i1 ? i1 - 1 : len - 1, i2 = i1 + 1 == len ? 0 : i1 + 1, i3 = i2 + 1 == len ? 0 : i2 + 1;
        float a0 = rb_get(r, n, i0, w), a1 = rb_get(r, n, i1, w), a2 = rb_get(r, n, i2, w), a3 = rb_get(r, n, i3, w);
        float c1 = 0.5f * (a2 - a0), c2 = a0 - 2.5f * a1 + 2.0f * a2 - 0.5f * a3, c3 = 0.5f * (a3 - a0) + 1.5f * (a1 - a2);
        float v = ((c3 * fr + c2) * fr + c1) * fr + a1;
#endif
        v *= r->g[n];
        r->lp[n] = v + r->d[n] * (r->lp[n] - v);
        y[n] = r->lp[n];
    }
    /* Householder-style orthogonal mix (H4 / 2) back into the lines */
    float h0 = 0.5f * (y[0] + y[1] + y[2] + y[3]), h1 = 0.5f * (y[0] - y[1] + y[2] - y[3]);
    float h2 = 0.5f * (y[0] + y[1] - y[2] - y[3]), h3 = 0.5f * (y[0] - y[1] - y[2] + y[3]);
    float x[4] = {h0 + il, h1 + ir, h2 + il, h3 + ir};
    for (n = 0; n < 4; n++) {
        int len = r->off[n + 1] - r->off[n], w = r->w[n];
        rb_put(r, n, w, x[n]);
        r->w[n] = w + 1 == len ? 0 : w + 1;
    }
    *ol = (y[0] + y[2]) * 0.35f * 2.0f * r->fit[0];
    *or_ = (y[1] + y[3]) * 0.35f * 2.0f * r->fit[0];
}
