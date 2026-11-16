/* SPDX-License-Identifier: GPL-3.0-only */
#include "sj6_blocks.h"
#include "sj6_f64.hpp"
#include "sj6_sinf.h"
#include <string.h>

extern "C" float fma32_fused(float, float, float);

static float fma(float a, float b, float c) { return fma32_fused(a, b, c); }
static int finite(float x)
{
    uint32_t u;
    memcpy(&u, &x, 4);
    return ((u >> 23) & 255) != 255;
}
static float fabsf_bits(float x)
{
    uint32_t u;
    memcpy(&u, &x, 4);
    u &= 0x7fffffffu;
    memcpy(&x, &u, 4);
    return x;
}
static float ceil_pos(float x)
{
    int i = (int)x;
    float f = (float)i;
    return f < x ? f + 1.f : f;
}
static float fmin2(float a, float b) { return b < a ? b : a; }
static float fmax2(float a, float b) { return a < b ? b : a; }

static float exp2_cr(float x)
{
    using namespace sj6_math;
    return f64_to_f32(f64_exp(f64_mul(f64_from_f32(x), 0x3fe62e42fefa39efULL)));
}
static float log2_cr(float x)
{
    using namespace sj6_math;
    return f64_to_f32(f64_div(f64_log(f64_from_f32(x)), 0x3fe62e42fefa39efULL));
}
static float log10_cr(float x)
{
    using namespace sj6_math;
    return f64_to_f32(f64_div(f64_log(f64_from_f32(x)), 0x40026bb1bbb55516ULL));
}
static float pow_cr(float a, float b)
{
    using namespace sj6_math;
    return f64_to_f32(f64_pow(f64_from_f32(a), f64_from_f32(b)));
}
float sj6_tanhf(float x)
{
    using namespace sj6_math;
    uint32_t u, ax, bits;
    float out;
    memcpy(&u, &x, 4);
    ax = u & 0x7fffffffu;
    if (ax >= 0x7f800000u) {
        bits = ax == 0x7f800000u ? (u & 0x80000000u) | 0x3f800000u : 0x7fc00000u;
        memcpy(&out, &bits, 4);
        return out;
    }
    /* |x| < 2^-13: the cubic term sits below half an ulp, so tanh(x) is x. */
    if (ax < 0x39000000u) return x;
    /* |x| >= 40: binary32 tanh is exactly +/-1, and the exp series would overflow. */
    if (ax >= 0x42200000u) return (u >> 31) ? -1.f : 1.f;
    {
        uint64_t e = f64_exp(f64_mul(f64_from_f32(x), 0x4000000000000000ULL));
        return f64_to_f32(f64_div(f64_sub(e, 0x3ff0000000000000ULL), f64_add(e, 0x3ff0000000000000ULL)));
    }
}
static float tanh_cr(float x) { return sj6_tanhf(x); }
static float sqrt_cr(float x)
{
    using namespace sj6_math;
    uint32_t u, seed;
    float y;
    uint64_t g, X;
    if (!(x > 0.f)) return 0.f;
    memcpy(&u, &x, 4);
    seed = 0x5f375a86u - (u >> 1);
    memcpy(&y, &seed, 4);
    y = y * (1.5f - 0.5f * x * y * y);
    y = y * (1.5f - 0.5f * x * y * y);
    y = y * (1.5f - 0.5f * x * y * y);
    g = f64_from_f32(x * y);
    X = f64_from_f32(x);
    g = f64_mul(f64_add(g, f64_div(X, g)), 0x3fe0000000000000ULL);
    g = f64_mul(f64_add(g, f64_div(X, g)), 0x3fe0000000000000ULL);
    return f64_to_f32(g);
}
float sj6_tanf(float x)
{
    using namespace sj6_math;
    uint32_t u, ax, bits;
    float out;
    uint64_t X, s, c;
    memcpy(&u, &x, 4);
    ax = u & 0x7fffffffu;
    if (ax >= 0x7f800000u) {
        bits = 0x7fc00000u;
        memcpy(&out, &bits, 4);
        return out;
    }
    /* |x| < 2^-13: tan(x) is x in binary32. The filter arguments are larger. */
    if (ax < 0x39000000u) return x;
    X = f64_from_f32(x);
    s = f64_sin(X);
    c = f64_sin(f64_sub(0x3ff921fb54442d18ULL, X));
    return f64_to_f32(f64_div(s, c));
}
static float tan_cr(float x) { return sj6_tanf(x); }
float sj6_exp2(float x) { return exp2_cr(x); }

float sj6_ota_hp_arg(float rate)
{
    return 31.41592788696289f / (rate + rate);
}

float sj6_ota_g_arg(float rate, float cutoff)
{
    float max_cut = fmin2(rate * 0.9f, 30000.f);
    float c = fmax2(fmin2(cutoff, max_cut), 3.f);
    float lc = log2_cr(c);
    return (exp2_cr(lc) * 3.1415927410125732f) / (rate + rate);
}

float sj6_syrup_tan_arg(float rate, float cut)
{
    float z;
    cut = fmin2(fmax2(cut, 12.f), rate * 0.45351473923f);
    z = (cut / rate) * 3.141592653589793f;
    return z / (1.f + z);
}

/* Constant sinusoid. Was a 2048-float BSS fill of this same f64_sin. */
#include "sj6_sine_table.inc"

static float interp_cyclic(float phase)
{
    using namespace sj6_math;
    float read, frac;
    int ri, rj;
    uint64_t mixed;
    while (phase < 0.f) phase += 1.f;
    while (phase >= 1.f) phase -= 1.f;
    read = phase * 2048.f;
    ri = (int)read;
    frac = read - (float)ri;
    rj = ri + 1;
    if (rj >= 2048) rj -= 2048;
    if (ri >= 2048) ri -= 2048;
    if (ri < 0) ri = 0;
    mixed = f64_fma(f64_sub(0x3ff0000000000000ULL, f64_from_f32(frac)), f64_from_f32(g_sine[ri]), f64_from_f32(frac * g_sine[rj]));
    return f64_to_f32(mixed);
}

static void seg_reset(Sj6Env *e, uint64_t initial, uint64_t target, uint64_t tco, int len)
{
    using namespace sj6_math;
    const uint64_t one = 0x3ff0000000000000ULL;
    uint64_t delta, ratio, span;
    e->output = len > 0 ? initial : target;
    e->target = target;
    e->horizontal = f64_cmp(target, initial) == 0;
    e->linear = f64_le(tco, 0);
    e->rising = f64_gt(target, initial);
    if (e->horizontal) {
        e->tcount = 0;
        e->seglen = len;
        return;
    }
    if (e->linear) {
        e->coefficient = len <= 0 ? f64_sub(target, e->output)
                                  : f64_div(f64_sub(target, initial), f64_from_i64(len));
        return;
    }
    if (len == 0) {
        e->coefficient = 0;
        e->offset = target;
        return;
    }
    delta = f64_sub(target, initial);
    if (delta >> 63) delta ^= 0x8000000000000000ULL;
    ratio = f64_div(f64_add(delta, tco), tco);
    e->coefficient = f64_exp(f64_neg(f64_div(f64_log(ratio), f64_from_i64(len))));
    span = f64_sub(one, e->coefficient);
    e->offset = e->rising ? f64_mul(f64_add(target, tco), span) : f64_mul(f64_sub(target, tco), span);
}

static void setup_desc(Sj6Env *e)
{
    Sj6Seg s = e->seg[e->cur];
    seg_reset(e, s.initial, s.final_value, s.tco, s.length);
}

static void setup_from(Sj6Env *e, uint64_t init)
{
    using namespace sj6_math;
    Sj6Seg s = e->seg[e->cur];
    uint64_t target = s.final_value;
    if (f64_cmp(s.initial, s.final_value) == 0) target = init;
    seg_reset(e, init, target, s.tco, s.length);
}

static int skip_empty(Sj6Env *e)
{
    while (e->cur < e->nseg && e->seg[e->cur].length == 0) e->cur++;
    if (e->cur == e->nseg) {
        e->cur = 0;
        setup_desc(e);
        return 0;
    }
    return 1;
}

static int exp_done(Sj6Env *e, float *out)
{
    using namespace sj6_math;
    int over;
    if (e->horizontal) {
        *out = f64_to_f32(e->target);
        if (e->seglen < 0) return 0;
        return ++e->tcount >= e->seglen;
    }
    if (e->linear) e->output = f64_add(e->output, e->coefficient);
    else e->output = f64_fma(e->coefficient, e->output, e->offset);
    over = e->rising ? f64_ge(e->output, e->target) : f64_le(e->output, e->target);
    if (over) e->output = e->target;
    *out = f64_to_f32(e->output);
    return over;
}

static uint64_t W(float x) { return sj6_math::f64_from_f32(x); }
static uint64_t E(uint64_t x) { return sj6_math::f64_exp(x); }
static uint64_t expm1b(uint64_t x) { return sj6_math::f64_sub(E(x), 0x3ff0000000000000ULL); }

static void env_begin(Sj6Env *e)
{
    e->cur = 0;
    setup_desc(e);
}

void sj6_adsr_init(Sj6Env *e, int curvature, float rate, float attack_s, float decay_s, float sustain, float release_s)
{
    using namespace sj6_math;
    const uint64_t z = 0, one = W(1.f);
    uint64_t sus = W(sustain);
    int silence = (int)(0.01f * rate);
    int attack = (int)(attack_s * rate);
    int decay = (int)(decay_s * rate);
    int release = (int)(release_s * rate);
    memset(e, 0, sizeof *e);
    e->nseg = 6;
    e->seg[0] = Sj6Seg{z, z, z, -1};
    e->seg[1] = Sj6Seg{one, z, z, silence};
    if (curvature == 3) {
        uint64_t reciprocal = f64_div(one, expm1b(0x3ff8000000000000ULL));
        e->seg[2] = Sj6Seg{z, one, f64_div(one, expm1b(one)), attack};
        e->seg[3] = Sj6Seg{one, sus, f64_mul(f64_sub(one, sus), reciprocal), decay};
        e->seg[4] = Sj6Seg{sus, sus, z, -1};
        e->seg[5] = Sj6Seg{sus, z, f64_mul(sus, reciprocal), release};
    } else if (curvature == 1) {
        e->seg[2] = Sj6Seg{z, one, E(0xbff8000000000000ULL), attack};
        e->seg[3] = Sj6Seg{one, sus, E(0xc013cccccccccccdULL), decay};
        e->seg[4] = Sj6Seg{sus, sus, z, -1};
        e->seg[5] = Sj6Seg{sus, z, E(0xc013cccccccccccdULL), release};
    } else if (curvature == 2) {
        e->seg[2] = Sj6Seg{z, one, 0x3fefffeb074a771dULL, attack};
        e->seg[3] = Sj6Seg{one, sus, E(0xc02619999999999aULL), decay};
        e->seg[4] = Sj6Seg{sus, sus, z, -1};
        e->seg[5] = Sj6Seg{sus, z, E(0xc02619999999999aULL), release};
    } else {
        e->seg[2] = Sj6Seg{z, one, z, attack};
        e->seg[3] = Sj6Seg{one, sus, z, decay};
        e->seg[4] = Sj6Seg{sus, sus, z, -1};
        e->seg[5] = Sj6Seg{sus, z, z, release};
    }
    env_begin(e);
}

void sj6_ahdshr_init(Sj6Env *e, int curvature, float rate, float attack_s, float hold_s, float decay_s,
                     float sustain, float release_hold_s, float release_s)
{
    const uint64_t z = 0, one = W(1.f);
    uint64_t sus = W(sustain);
    int silence = (int)(0.01f * rate);
    memset(e, 0, sizeof *e);
    e->nseg = 8;
    e->seg[0] = Sj6Seg{z, z, z, -1};
    e->seg[1] = Sj6Seg{one, z, z, silence};
    e->seg[3] = Sj6Seg{one, one, z, (int)(hold_s * rate)};
    e->seg[5] = Sj6Seg{sus, sus, z, -1};
    e->seg[6] = Sj6Seg{sus, sus, z, (int)(release_hold_s * rate)};
    if (curvature == 1) {
        e->seg[2] = Sj6Seg{z, one, E(0xbff8000000000000ULL), (int)(attack_s * rate)};
        e->seg[4] = Sj6Seg{one, sus, E(0xc013cccccccccccdULL), (int)(decay_s * rate)};
        e->seg[7] = Sj6Seg{sus, z, E(0xc013cccccccccccdULL), (int)(release_s * rate)};
    } else if (curvature == 2) {
        e->seg[2] = Sj6Seg{z, one, 0x3fefffeb074a771dULL, (int)(attack_s * rate)};
        e->seg[4] = Sj6Seg{one, sus, E(0xc02619999999999aULL), (int)(decay_s * rate)};
        e->seg[7] = Sj6Seg{sus, z, E(0xc02619999999999aULL), (int)(release_s * rate)};
    } else {
        e->seg[2] = Sj6Seg{z, one, z, (int)(attack_s * rate)};
        e->seg[4] = Sj6Seg{one, sus, z, (int)(decay_s * rate)};
        e->seg[7] = Sj6Seg{sus, z, z, (int)(release_s * rate)};
    }
    env_begin(e);
}

void sj6_env_start(Sj6Env *e)
{
    e->cur = 2;
    if (skip_empty(e)) setup_from(e, e->seg[e->cur].initial);
}

void sj6_env_release(Sj6Env *e, int release_index)
{
    uint64_t here = W(sj6_math::f64_to_f32(e->horizontal ? e->target : e->output));
    if (e->nseg == 8) {
        e->seg[6].initial = here;
        e->seg[6].final_value = here;
        e->seg[7].initial = here;
        e->cur = 6;
    } else {
        e->seg[release_index].initial = here;
        e->cur = release_index;
    }
    if (skip_empty(e)) setup_from(e, e->output);
}

float sj6_env_sample(Sj6Env *e)
{
    float out = 0.f;
    if (exp_done(e, &out)) {
        e->cur++;
        if (e->cur >= e->nseg) e->cur = 0;
        setup_desc(e);
    }
    return out;
}

void sj6_smooth_init(Sj6Smooth *f, float sample_rate)
{
    using namespace sj6_math;
    int i;
    memset(f, 0, sizeof *f);
    f->rate = f64_from_f32(sample_rate);
    f->smoothing = f64_sub(0x3ff0000000000000ULL, f64_exp(f64_div(0xc0578fdb9effea46ULL, f->rate)));
    f->first = 1;
    f->cutoff = 0x40d3880000000000ULL;
    f->resonance = 0x3fb999999999999aULL;
    sj6_smooth_set(f, f64_to_f32(f->cutoff), f64_to_f32(f->resonance));
    for (i = 0; i < 5; i++) f->c[i] = f->target[i];
    f->first = 1;
}

void sj6_smooth_set(Sj6Smooth *f, float frequency, float resonance)
{
    using namespace sj6_math;
    const uint64_t one = 0x3ff0000000000000ULL;
    uint64_t freq, res, normalized, k, u, v, w, z, out[5];
    float phase;
    freq = f64_from_f32(frequency);
    res = f64_from_f32(resonance);
    if (f64_lt(freq, 0x4028000000000000ULL)) freq = 0x4028000000000000ULL;
    if (f64_lt(res, 0)) res = 0;
    if (f64_gt(res, 0x4034000000000000ULL)) res = 0x4034000000000000ULL;
    f->cutoff = freq;
    f->resonance = res;
    normalized = f64_div(f64_add(freq, freq), f->rate);
    if (f64_gt(normalized, 0x3fefae147ae147aeULL)) normalized = 0x3fefae147ae147aeULL;
    phase = f64_to_f32(f64_mul(0x3fe0000000000000ULL, normalized));
    k = f64_mul(f64_pow(res, 0x3ffe8f5c28f5c28fULL), f64_from_f32(interp_cyclic(phase)));
    u = f64_mul(0x3fe0000000000000ULL, f64_div(f64_sub(one, k), f64_add(one, k)));
    v = f64_add(0x3fe0000000000000ULL, u);
    w = f64_mul(v, f64_from_f32(interp_cyclic(phase + 0.25f)));
    z = f64_mul(f64_sub(v, w), 0x3fd0000000000000ULL);
    out[0] = f64_add(z, z);
    out[1] = f64_mul(z, 0x4010000000000000ULL);
    out[2] = out[0];
    out[3] = f64_mul(0xc000000000000000ULL, w);
    out[4] = f64_add(u, u);
    for (int i = 0; i < 5; i++) f->target[i] = out[i];
}

float sj6_smooth_process(Sj6Smooth *f, float input)
{
    using namespace sj6_math;
    uint64_t in, out;
    float result;
    int i;
    if (f->first) {
        for (i = 0; i < 5; i++) f->c[i] = f->target[i];
        f->first = 0;
    }
    for (i = 0; i < 5; i++) f->c[i] = f64_fma(f->smoothing, f64_sub(f->target[i], f->c[i]), f->c[i]);
    in = f64_from_f32(input);
    out = f64_fma(f64_neg(f->c[4]), f->y2, f64_fma(f64_neg(f->c[3]), f->y1, f64_fma(f->c[2], f->x2, f64_fma(f->c[0], in, f64_mul(f->c[1], f->x1)))));
    result = f64_to_f32(out);
    f->x2 = f->x1;
    f->x1 = in;
    f->y2 = f->y1;
    f->y1 = f64_from_f32(result);
    return result;
}

void sj6_lowpass_init(Sj6Lowpass *f, float rate)
{
    memset(f, 0, sizeof *f);
    f->rate = rate;
    f->last_cut = f->last_res = -1.f;
}

void sj6_lowpass_set(Sj6Lowpass *f, float cutoff_hz, float resonance)
{
    float cutoff, k, c1, c2, c3;
    if (cutoff_hz == f->last_cut && resonance == f->last_res) return;
    if (cutoff_hz < 12.f) cutoff_hz = 12.f;
    if (resonance < 0.1f) resonance = 0.1f;
    if (resonance > 10.f) resonance = 10.f;
    cutoff = 2.f * cutoff_hz / f->rate;
    if (cutoff > 0.99f) cutoff = 0.99f;
    f->last_cut = cutoff_hz;
    f->last_res = resonance;
    k = pow_cr(resonance, 1.91f) * interp_cyclic(0.5f * cutoff);
    c1 = 0.5f * (1.f - k) / (1.f + k);
    c2 = (0.5f + c1) * interp_cyclic(0.5f * cutoff + 0.25f);
    c3 = (0.5f + c1 - c2) * 0.25f;
    f->a0 = 2.f * c3;
    f->a1 = 2.f * 2.f * c3;
    f->a2 = 2.f * c3;
    f->b1 = 2.f * -c2;
    f->b2 = 2.f * c1;
}

float sj6_lowpass_process(Sj6Lowpass *f, float input)
{
    float y = (float)fma(-f->b2, f->y2, fma(-f->b1, f->y1, fma(f->a2, f->x2, fma(f->a0, input, f->a1 * f->x1))));
    f->x2 = f->x1;
    f->x1 = input;
    f->y2 = f->y1;
    f->y1 = y;
    return y;
}

static float ota_curve(float q)
{
    if (q <= 0.5f) {
        q /= 0.9f;
        return (q * q) * fma(q, -6.920000076293945f, 10.920000076293945f);
    }
    if (q < 0.75f) {
        q = (q + -0.5f) * 4.f;
        return fma(q, fma(q, fma(q, 0.7831621170043945f, -1.407510757446289f), 1.5905351638793945f), 2.1838135719299316f);
    }
    if (q < 0.9f) {
        q = (q + -0.75f) / 0.15000000596046448f;
        return fma(q, fma(q, fma(q, -0.8449998497962952f, 1.0199997425079346f), 0.675000011920929f), 3.1500000953674316f);
    }
    return fma(q + -0.9f, 1.2000000476837158f, 4.f);
}

void sj6_ota_init(Sj6Ota *f, float rate)
{
    memset(f, 0, sizeof *f);
    f->rate = rate;
    f->hp_tan = tan_cr(sj6_ota_hp_arg(rate));
    f->hp_g = f->hp_tan / (1.f + f->hp_tan);
    f->seed_ok = 1;
}

void sj6_ota_set(Sj6Ota *f, float cutoff, float normalized_res, unsigned frames)
{
    float max_cut, c, lc, q, t, w, s;
    f->out_gain = 1.f;
    if (f->first) {
        if (f->mode_target != 1.f) {
            f->mode_left = (int)fmax2(1.f, ceil_pos(f->rate * 0.005f));
            f->mode_target = 1.f;
            f->mode_step = (f->mode_target - f->mode) / (float)f->mode_left;
        }
    } else {
        f->mode = f->mode_target = 1.f;
        f->mode_left = 0;
    }
    max_cut = fmin2(f->rate * 0.9f, 30000.f);
    c = fmax2(fmin2(cutoff, max_cut), 3.f);
    lc = log2_cr(c);
    q = ota_curve(fmax2(fmin2(normalized_res, 1.f), 0.f));
    t = fmin2(fmax2((c / f->rate + -0.45f) / 0.05f, 0.f), 1.f);
    w = fmax2(q + -3.9f, 0.f) * t;
    s = fma(t, -2.f, 3.f);
    q = fma(-(t * w), s, q);
    f->k = q;
    if (!(q > 4.f)) f->seeded = 0;
    if (f->first) {
        if (lc == f->cutoff_log) return;
        f->cutoff_log = lc;
        f->g_target = tan_cr(sj6_ota_g_arg(f->rate, c));
        f->g_left = (int)fmax2(1.f, (float)frames);
        f->g_step = (f->g_target - f->g) / (float)f->g_left;
    } else {
        f->cutoff_log = lc;
        f->g = f->g_target = tan_cr(sj6_ota_g_arg(f->rate, c));
        f->first = 1;
    }
}

static float ota_tick(Sj6Ota *s, float input, float G, float G4, float inv)
{
    float d = (input - s->hp) * s->hp_g;
    float low = s->hp + d;
    float acc = 0.f, power = 1.f, a = 1.f - G, x, c, t, v, factor, in, product = 1.f, affine = 0.f;
    float provisional[4], coeff[4], offset[4], feedback, correction;
    int i;
    s->hp = d + low;
    for (i = 3; i >= 0; --i) {
        float ti = a * power;
        acc = fma(ti, s->z[i], acc);
        power = G * power;
    }
    x = (input - low) * 0.025f;
    c = fma(s->k, 0.3f, 1.f);
    t = G4 * c;
    t = fma(t, x, acc);
    t = -(inv * t);
    t = s->k * t;
    t = fma(c, x, t);
    for (i = 0; i < 4; i++) {
        float old = a * s->z[i];
        t = fma(G, t, old);
        provisional[i] = t;
    }
    v = fma(x, 0.3f, -provisional[3]);
    v = tanh_cr(v * 8.f);
    factor = s->k * 0.125f;
    in = fma(factor, v, x);
    for (i = 0; i < 4; i++) {
        float prev = provisional[i];
        float th = tanh_cr(in - prev);
        float n = fma(-th, th, 1.f);
        float recip, b;
        n = s->g * n;
        recip = 1.f / (n + 1.f);
        coeff[i] = n * recip;
        b = fma(-s->g, th, prev - s->z[i]);
        offset[i] = -(b * recip);
        product = product * coeff[i];
        affine = fma(coeff[i], affine, offset[i]);
        in = prev;
    }
    feedback = fma(-v, v, 1.f);
    feedback = -(s->k * feedback);
    correction = (feedback * affine) / fma(-feedback, product, 1.f);
    for (i = 0; i < 4; i++) {
        correction = fma(coeff[i], correction, offset[i]);
        provisional[i] += correction;
        s->z[i] = fma(provisional[i], 2.f, -s->z[i]);
    }
    if (s->mode == 1.f) return provisional[3] * s->out_gain;
    if (s->mode == 0.f) return provisional[1];
    return fma(s->mode, provisional[3] * s->out_gain - provisional[1], provisional[1]);
}

static void ota_half(float *st, float input, float *a, float *b)
{
    float aa = fma(input - st[2], 0.079866424202919f, st[0]);
    float ab = fma(aa - st[3], 0.5453536510467529f, st[1]);
    float c = fma(input - st[6], 0.28382933139801025f, st[4]);
    float d = fma(c - st[7], 0.8344119191169739f, st[5]);
    st[0] = input;
    st[1] = aa;
    st[2] = aa;
    st[3] = ab;
    st[4] = input;
    st[5] = c;
    st[6] = c;
    st[7] = d;
    *a = ab;
    *b = d;
}

float sj6_ota_process(Sj6Ota *s, float input)
{
    float a, b, G, G4, inv, x, y, aa, ab, ba, bb;
    if (s->mode_left > 0) {
        s->mode += s->mode_step;
        if (--s->mode_left == 0) s->mode = s->mode_target;
    }
    if (s->g_left > 0) {
        s->g += s->g_step;
        if (--s->g_left == 0) s->g = s->g_target;
    }
    if (s->k > 4.f && !s->seeded) {
        if (s->seed_ok) {
            float sa = sqrt_cr(s->k + -4.f) * 0.125f;
            float sb = sa + sa;
            float t = tanh_cr(sb) * s->g;
            float tt;
            s->z[0] = fma(sa, 2.f, t);
            s->z[1] = sb;
            tt = tanh_cr(sa);
            s->z[2] = fma(-s->g, tt, sa);
            s->z[3] = -(s->g * tt);
        } else s->z[0] += 0.00009999999747378752f;
        s->seeded = 1;
    }
    ota_half(s->up, input, &a, &b);
    G = s->g / (s->g + 1.f);
    G4 = G * G;
    G4 = G * G4;
    G4 = G * G4;
    inv = 1.f / fma(s->k, G4, 1.f);
    s->seed_ok = 0;
    x = ota_tick(s, a, G, G4, inv);
    y = ota_tick(s, b, G, G4, inv);
    aa = fma(y - s->down[2], 0.079866424202919f, s->down[0]);
    ab = fma(aa - s->down[3], 0.5453536510467529f, s->down[1]);
    ba = fma(x - s->down[6], 0.28382933139801025f, s->down[4]);
    bb = fma(ba - s->down[7], 0.8344119191169739f, s->down[5]);
    s->down[0] = y;
    s->down[1] = aa;
    s->down[2] = aa;
    s->down[3] = ab;
    s->down[4] = x;
    s->down[5] = ba;
    s->down[6] = ba;
    s->down[7] = bb;
    return ((ab + bb) * 0.5f) * 40.f;
}

void sj6_syrup_init(Sj6Syrup *f, float rate)
{
    memset(f, 0, sizeof *f);
    f->rate = rate;
    f->last_cut = 12.f;
    f->last_res = 0.1f;
    f->in_gain = 1.f;
    f->out_gain = 1.f;
    f->a = -1.f;
    f->b = 1.f;
    f->inv_a = 1.f;
    f->inv_b = 1.f;
}

void sj6_syrup_set(Sj6Syrup *f, float cut, float res)
{
    float z, q, kr, drive, twice, limit, gg, c, d;
    if (!finite(cut)) cut = f->last_cut;
    if (!finite(res)) res = f->last_res;
    cut = fmin2(fmax2(cut, 12.f), f->rate * 0.45351473923f);
    res = fmin2(fmax2(res, 0.1f), 10.f);
    f->last_cut = cut;
    f->last_res = res;
    z = (cut / f->rate) * 3.141592653589793f;
    f->g = tan_cr(sj6_syrup_tan_arg(f->rate, cut));
    q = (-20.f * log10_cr(res)) / 19.f;
    q = fmin2(fmax2(q, 0.f), 1.f);
    kr = pow_cr(q, 0.45f) * 2.05f;
    drive = fma(q, 2.f, 1.f);
    f->in_gain = 1.f / drive;
    f->out_gain = (pow_cr(drive, 0.75f) * 1.35f) / sqrt_cr(drive * f->in_gain);
    twice = f->g + f->g;
    limit = fmax2(1.f, fma(twice, 0.09f, 0.97f));
    f->k = kr / limit;
    gg = f->g * f->g;
    f->a = (twice - gg) + -1.f;
    f->b = 1.f - f->g;
    c = (gg - f->g) + 1.f;
    f->inv_a = fabsf_bits(c) > 1e-6f ? 1.f / c : (c < 0.f ? -1e6f : 1e6f);
    d = fma(f->k, gg - f->g, 1.f);
    f->inv_b = fabsf_bits(d) > 1e-6f ? 1.f / d : (d < 0.f ? -1e6f : 1e6f);
}

float sj6_syrup_process(Sj6Syrup *s, float x)
{
    float v, d, feedback, av, vv, n, den, sat, dx, y;
    if (s->four_pole) {
        v = s->inv_a * (x - fma(s->a, s->h1, s->b * s->h2));
        d = s->g * (v - s->h1);
        s->p1 = s->h1 + d;
        s->h1 = d + s->p1;
        d = s->g * (s->p1 - s->h2);
        s->p2 = s->h2 + d;
        s->h2 = d + s->p2;
        x = s->p2;
    }
    feedback = fma(s->a, s->h3, s->b * s->h4);
    v = s->inv_b * fma(s->in_gain, x, -(feedback * s->k));
    av = fabsf_bits(v);
    vv = v * v;
    n = v * fma(vv, fma(av, 0.8212266564369202f, 0.8932298421859741f), fma(av, 2.455507516860962f, 2.455507516860962f));
    den = fma(vv + 2.445066452026367f, fabsf_bits(fma(v * 0.8146427273750305f, av, v)), 2.445066452026367f);
    sat = n / den;
    dx = s->g * (sat - s->h3);
    s->p3 = s->h3 + dx;
    s->h3 = dx + s->p3;
    dx = s->g * (s->p3 - s->h4);
    s->p4 = s->h4 + dx;
    s->h4 = dx + s->p4;
    y = s->out_gain * s->p4;
    if (!finite(y)) {
        s->p1 = s->h1 = s->p2 = s->h2 = s->p3 = s->h3 = s->p4 = s->h4 = 0.f;
        return 0.f;
    }
    return y;
}

static int bad_finite(float x) { return !finite(x); }

static const float k_lfo_factors[18] = {
    64.f, 48.f, 32.f, 24.f, 16.f, 12.f, 8.f, 6.f, 4.f, 3.f, 2.f, 1.5f, 1.f, 0.5f, 0.3333f, 0.25f, 0.1667f, 0.125f
};

int sj6_lfo_phase(float phase, float rate, float dt, float *out)
{
    float next;
    if (bad_finite(phase) || bad_finite(rate) || rate < 0.f || bad_finite(dt) || dt < 0.f) return 1;
    next = phase + (float)rate * dt;
    if (!finite(next)) return 2;
    if (next >= 1.f) next -= 1.f;
    *out = next;
    return 0;
}

int sj6_lfo_rate_tempo(float normalized, float tempo, float *out)
{
    volatile float scaled, value;
    unsigned i;
    if (!finite(normalized) || normalized < 0.f || normalized > 1.f || !finite(tempo) || tempo < 0.f) return 1;
    scaled = (1.f - normalized) * 17.f;
    i = (unsigned)(float(scaled) + 0.5f);
    if (i >= 18) return 1;
    value = ((tempo / 60.f) * k_lfo_factors[i]) * 0.25f;
    if (!finite(value)) return 2;
    *out = value;
    return 0;
}

int sj6_lfo_index(float phase, int64_t *out)
{
    float x;
    if (!finite(phase)) return 1;
    x = phase * 256.f;
    if (!finite(x)) return 1;
    *out = (int64_t)x % 256;
    return 0;
}

int sj6_shape_sample(unsigned kind, unsigned i, float *out)
{
    volatile float v;
    if (i >= 256) return 1;
    if (kind == 1) v = i < 128 ? ((4.f / 256.f) * (float)i) - 1.f : ((4.f / 256.f) * -(float)i) + 3.f;
    else if (kind == 2) v = i < 128 ? -1.f : 1.f;
    else if (kind == 3) v = ((float)(i + i) / 256.f) - 1.f;
    else if (kind == 4) v = 1.f - ((float)(i + i) / 256.f);
    else return 3;
    *out = v;
    return 0;
}

int sj6_lfo_modulate(float base, float wave, float amount, unsigned waveform, float *out)
{
    float delta, value;
    if (!finite(base) || !finite(wave) || wave < -1.f || wave > 1.f || !finite(amount) || amount < 0.f || amount > 1.f || waveform > 5) return 1;
    delta = waveform == 1 ? amount * ((wave + 1.f) * 0.5f) : waveform == 2 ? amount * ((wave - 1.f) * 0.5f) : (wave * amount) * 0.5f;
    value = delta + (float)base;
    if (value < 0.f) value = 0.f;
    if (value > 1.f) value = 1.f;
    *out = value;
    return 0;
}

int sj6_beat_catchup(const Sj6Beat *start, uint64_t beat, uint64_t tempo, int loaded, Sj6Tick *out, int capacity, int *count, Sj6Beat *result)
{
    using namespace sj6_math;
    const uint64_t kZero = 0, kOne = 0x3ff0000000000000ULL, kSixty = 0x404e000000000000ULL;
    const uint64_t kStep = 0x3f8999999999999aULL, kStepNeg = 0xbf8999999999999aULL, kGap = 0x3fc999999999999aULL;
    Sj6Beat next;
    int used = 0;
    if (!start || !result || !count || capacity < 0) return 1;
    if (!f64_finite(start->last) || !f64_finite(start->accumulator) || f64_lt(start->accumulator, kZero) || f64_ge(start->accumulator, kStep) || !f64_finite(beat) || !f64_finite(tempo))
        return 1;
    next = *start;
    next.last = beat;
    if (!loaded || f64_lt(beat, start->last)) next.accumulator = kZero;
    else {
        uint64_t elapsed = f64_div(f64_mul(f64_sub(beat, start->last), kSixty), f64_lt(tempo, kOne) ? kOne : tempo);
        if (f64_finite(elapsed) && f64_gt(elapsed, kZero)) {
            if (f64_gt(elapsed, kGap)) next.accumulator = kZero;
            else {
                next.accumulator = f64_add(next.accumulator, elapsed);
                while (f64_ge(next.accumulator, kStep)) {
                    if (used == capacity) return 2;
                    if (used == 17) return 1;
                    if (out) out[used].value = 0.0125f;
                    used++;
                    next.accumulator = f64_add(next.accumulator, kStepNeg);
                }
            }
        }
    }
    *result = next;
    *count = used;
    return 0;
}

static Sj6Env g_keep_env;
static Sj6Smooth g_keep_smooth;
static Sj6Lowpass g_keep_low;
static Sj6Ota g_keep_ota;
static Sj6Syrup g_keep_syrup;
static Sj6Beat g_keep_beat, g_keep_beat_out;
static Sj6Tick g_keep_ticks[4];

void sj6_blocks_keep(void)
{
    Sj6Env *env = &g_keep_env;
    Sj6Smooth *smooth = &g_keep_smooth;
    Sj6Lowpass *low = &g_keep_low;
    Sj6Ota *ota = &g_keep_ota;
    Sj6Syrup *syrup = &g_keep_syrup;
    Sj6Beat *beat = &g_keep_beat;
    Sj6Beat *outb = &g_keep_beat_out;
    Sj6Tick *ticks = g_keep_ticks;
    float y, phase = 0.f;
    int n = 0;
    int64_t ix = 0;
    sj6_adsr_init(env, 1, 44100.f, 0.01f, 0.1f, 0.5f, 0.2f);
    sj6_env_start(env);
    y = sj6_env_sample(env);
    sj6_smooth_init(smooth, 44100.f);
    y += sj6_smooth_process(smooth, 0.1f);
    sj6_lowpass_init(low, 44100.f);
    sj6_lowpass_set(low, 1000.f, 0.5f);
    y += sj6_lowpass_process(low, 0.1f);
    sj6_ota_init(ota, 44100.f);
    sj6_ota_set(ota, 1000.f, 0.2f, 1);
    y += sj6_ota_process(ota, 0.1f);
    sj6_syrup_init(syrup, 44100.f);
    sj6_syrup_set(syrup, 1000.f, 0.5f);
    y += sj6_syrup_process(syrup, 0.1f);
    sj6_lfo_phase(0.1f, 1.f, 0.01f, &phase);
    sj6_lfo_rate_tempo(0.5f, 120.f, &phase);
    sj6_lfo_index(0.2f, &ix);
    sj6_lfo_modulate(0.5f, 0.25f, 0.5f, 0, &phase);
    sj6_shape_sample(1, 10, &phase);
    beat->generation = 1;
    beat->last = sj6_math::f64_from_f32(0.f);
    beat->accumulator = 0;
    sj6_beat_catchup(beat, sj6_math::f64_from_f32(0.05f), sj6_math::f64_from_f32(120.f), 1, ticks, 4, &n, outb);
    (void)y;
    (void)ix;
}
