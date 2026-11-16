/* Binary64 evaluation of the original double formulas, rounded once to float
 * at each store. Taylor series run in integer binary64. The reachable domains
 * are proved against libm by nodouble/exact_proof.c. */
#include "exact.h"
#include "d64.h"

static const uint64_t D1 = 0x3ff0000000000000ull;
static const uint64_t LN2 = 0x3fe62e42fefa39efull;
static const uint64_t PI = 0x400921fb54442d18ull;
static const uint64_t HALF_PI = 0x3ff921fb54442d18ull;
static const uint64_t PIO2_LO = 0x3c91a62633145c07ull;
static const uint64_t TWO_PI = 0x401921fb54442d18ull;
static const uint64_t THERMAL = 0x3efa36e2eb1c432dull;
static const uint64_t C4 = 0x4010000000000000ull;
static const uint64_t C05 = 0x3fe0000000000000ull;
static const uint64_t ROOT2 = 0x3ff6a09e667f3bcdull;
static const uint64_t C069 = 0x4051400000000000ull;
static const uint64_t C012 = 0x4028000000000000ull;
static const uint64_t C440 = 0x407b800000000000ull;
static const uint64_t C18730 = 0x3ffdf7ced916872bull;
static const uint64_t C04955 = 0x3fdfb645a1cac083ull;
static const uint64_t C06490 = 0x3fe4c49ba5e353f8ull;
static const uint64_t C09988 = 0x3feff62b6ae7d567ull;
static const uint64_t CN39364 = 0xc00f7dbf487fcb92ull;
static const uint64_t C18409 = 0x3ffd74538ef34d6aull;
static const uint64_t C09968 = 0x3fefe5c91d14e3bdull;

static uint64_t exp_small(uint64_t z)
{
    uint64_t term = D1, sum = D1;
    int k;
    for (k = 1; k <= 18; k++) {
        term = d64_div(d64_mul(term, z), d64_from_u32((uint32_t)k));
        sum = d64_add(sum, term);
    }
    return sum;
}

static uint64_t exp_d(uint64_t x)
{
    int n;
    uint64_t r;
    /* exp(-inf) is +0, as exp(-1.0/(tau*sr)) in adsr.c tau2pole (release 0). */
    if (x == 0xfff0000000000000ull) return 0;
    if (!d64_isfinite(x)) return x;
    if (d64_lt(x, d64_from_i32(-745))) return 0;
    if (d64_lt(d64_from_i32(710), x)) return 0x7ff0000000000000ull;
    n = (int)d64_rint(d64_div(x, LN2));
    r = d64_sub(x, d64_mul(d64_from_i32(n), LN2));
    return d64_ldexp(exp_small(r), n);
}

static uint64_t exp2_d(uint64_t x)
{
    int n;
    uint64_t f;
    /* exp2(-inf) is +0, as pow(0.5, +inf) in port.c (half time 0). +inf and NaN pass through. */
    if (x == 0xfff0000000000000ull) return 0;
    if (!d64_isfinite(x)) return x;
    if (d64_lt(x, d64_from_i32(-1100))) return 0;
    if (d64_lt(d64_from_i32(1024), x)) return 0x7ff0000000000000ull;
    n = (int)d64_rint(x);
    f = d64_sub(x, d64_from_i32(n));
    return d64_ldexp(exp_small(d64_mul(f, LN2)), n);
}

static uint64_t log2_d(uint64_t x)
{
    uint64_t m, u, u2, term, sum, ln;
    int exp, k;
    exp = d64_mant(x, &m);
    if (exp < -90000) return 0x7ff8000000000000ull;
    u = d64_div(d64_sub(m, D1), d64_add(m, D1));
    u2 = d64_mul(u, u);
    term = u;
    sum = u;
    for (k = 1; k <= 14; k++) {
        term = d64_mul(term, u2);
        sum = d64_add(sum, d64_div(term, d64_from_u32((uint32_t)(2 * k + 1))));
    }
    ln = d64_mul(d64_from_i32(2), sum);
    return d64_add(d64_from_i32(exp), d64_div(ln, LN2));
}

static uint64_t sin_reduced(uint64_t r)
{
    uint64_t r2 = d64_mul(r, r), term = r, sum = r;
    int k;
    for (k = 1; k <= 12; k++) {
        term = d64_div(d64_mul(term, r2), d64_from_u32((uint32_t)((2 * k) * (2 * k + 1))));
        if (k & 1) sum = d64_sub(sum, term);
        else sum = d64_add(sum, term);
    }
    return sum;
}

static uint64_t cos_reduced(uint64_t r)
{
    uint64_t r2 = d64_mul(r, r), term = D1, sum = D1;
    int k;
    for (k = 1; k <= 12; k++) {
        term = d64_div(d64_mul(term, r2), d64_from_u32((uint32_t)((2 * k - 1) * (2 * k))));
        if (k & 1) sum = d64_sub(sum, term);
        else sum = d64_add(sum, term);
    }
    return sum;
}

static void sincos_d(uint64_t x, uint64_t *s, uint64_t *c)
{
    int q = (int)d64_rint(d64_div(x, HALF_PI));
    uint64_t n = d64_from_i32(q);
    uint64_t r = d64_sub(d64_sub(x, d64_mul(n, HALF_PI)), d64_mul(n, PIO2_LO));
    uint64_t sr = sin_reduced(r), cr = cos_reduced(r);
    switch (q & 3) {
    case 0: *s = sr; *c = cr; break;
    case 1: *s = cr; *c = d64_neg(sr); break;
    case 2: *s = d64_neg(sr); *c = d64_neg(cr); break;
    default: *s = d64_neg(cr); *c = sr; break;
    }
}

static uint64_t sin_d(uint64_t x)
{
    uint64_t s, c;
    sincos_d(x, &s, &c);
    return s;
}

static uint64_t cos_d(uint64_t x)
{
    uint64_t s, c;
    sincos_d(x, &s, &c);
    return c;
}

static uint64_t tan_d(uint64_t x)
{
    return d64_div(sin_d(x), cos_d(x));
}

static uint64_t tanh_d(uint64_t x)
{
    int neg = d64_lt(x, 0);
    uint64_t ax = neg ? d64_neg(x) : x;
    uint64_t e, t;
    if (!d64_lt(ax, C4)) return neg ? d64_neg(D1) : D1;
    e = exp_d(d64_mul(d64_from_i32(2), ax));
    t = d64_sub(D1, d64_div(d64_from_i32(2), d64_add(e, D1)));
    return neg ? d64_neg(t) : t;
}

static uint64_t my_tanh(uint64_t x)
{
    int neg;
    uint64_t ax;
    if (d64_lt(x, 0)) { neg = 1; ax = d64_neg(x); }
    else { neg = 0; ax = x; }
    if (!d64_lt(ax, C4)) return neg ? d64_neg(D1) : D1;
    if (d64_lt(ax, C05)) return x;
    return neg ? d64_neg(tanh_d(ax)) : tanh_d(ax);
}

float s1_exp2_f64(float x) { return d64_to_float(exp2_d(d64_from_float(x))); }
float s1_exp_f64(float x) { return d64_to_float(exp_d(d64_from_float(x))); }
float s1_log2_f64(float x) { return d64_to_float(log2_d(d64_from_float(x))); }
float s1_sin_f64(float x) { return d64_to_float(sin_d(d64_from_float(x))); }
float s1_cos_f64(float x) { return d64_to_float(cos_d(d64_from_float(x))); }
float s1_tan_f64(float x) { return d64_to_float(tan_d(d64_from_float(x))); }
float s1_tanh_f64(float x) { return d64_to_float(tanh_d(d64_from_float(x))); }
float s1_pow_f64(float x, float y)
{
    if (!(x > 0.f)) return d64_to_float(0x7ff8000000000000ull);
    return d64_to_float(exp2_d(d64_mul(d64_from_float(y), log2_d(d64_from_float(x)))));
}

float s1_midi_cps_f64(float nn)
{
    uint64_t x = d64_div(d64_sub(d64_from_float(nn), C069), C012);
    return d64_to_float(d64_mul(exp2_d(x), C440));
}

float s1_adsr_pole(float tau, float sr)
{
    uint64_t prod = d64_from_float(tau * sr);
    return d64_to_float(exp_d(d64_neg(d64_div(D1, prod))));
}

float s1_round_mul(float a, uint64_t b_bits)
{
    return d64_to_float(d64_mul(d64_from_float(a), b_bits));
}

int s1_f_gt_d(float a, uint64_t b_bits)
{
    return d64_lt(b_bits, d64_from_float(a));
}

float s1_port_onedsr(float sr)
{
    return d64_to_float(d64_div(D1, d64_from_float(sr)));
}

void s1_port_coeffs(float onedsr, float htime, float *c1, float *c2)
{
    uint64_t y = d64_from_float(onedsr / htime);
    uint64_t c = exp2_d(d64_neg(y));
    *c2 = d64_to_float(c);
    *c1 = d64_to_float(d64_sub(D1, d64_from_float(*c2)));
}

float s1_lfo_sine(float phase)
{
    uint64_t t = d64_mul(d64_mul(d64_from_float(phase), PI), d64_from_float(2.f));
    return d64_to_float(sin_d(t));
}

void s1_butbp_consts(float sr, float *pidsr, float *tpidsr)
{
    uint64_t s = d64_from_float(sr);
    *pidsr = d64_to_float(d64_div(PI, s));
    if (tpidsr) *tpidsr = d64_to_float(d64_div(TWO_PI, s));
}

void s1_butbp_coeffs(float pidsr, float tpidsr, float bw, float fr, float *a)
{
    uint64_t arg_c = d64_from_float(pidsr * bw);
    uint64_t arg_d = d64_from_float(tpidsr * fr);
    float c = d64_to_float(d64_div(D1, tan_d(arg_c)));
    float d = d64_to_float(d64_mul(d64_from_i32(2), cos_d(arg_d)));
    float a1 = d64_to_float(d64_div(D1, d64_add(D1, d64_from_float(c))));
    a[1] = a1;
    a[2] = 0.f;
    a[3] = -a1;
    a[4] = -c * d * a1;
    a[5] = d64_to_float(d64_mul(d64_sub(d64_from_float(c), D1), d64_from_float(a1)));
}

void s1_buthp_coeffs(float pidsr, float freq, float *a)
{
    float c = d64_to_float(tan_d(d64_from_float(pidsr * freq)));
    uint64_t cc = d64_from_float(c * c);
    uint64_t rc = d64_mul(ROOT2, d64_from_float(c));
    float a1 = d64_to_float(d64_div(D1, d64_add(d64_add(D1, rc), cc)));
    a[1] = a1;
    a[2] = -(a1 + a1);
    a[3] = a1;
    a[4] = d64_to_float(d64_mul(d64_mul(d64_from_i32(2), d64_sub(cc, D1)), d64_from_float(a1)));
    a[5] = d64_to_float(d64_mul(d64_add(d64_sub(D1, rc), cc), d64_from_float(a1)));
}

void s1_moog_coeffs(float freq, float sr, float *acr_out, float *tune_out)
{
    float fc = freq / sr;
    float f = d64_to_float(d64_mul(C05, d64_from_float(fc)));
    float fc2 = fc * fc;
    float fc3 = fc2 * fc;
    uint64_t fcr_d = d64_add(d64_sub(d64_add(d64_mul(C18730, d64_from_float(fc3)),
                                             d64_mul(C04955, d64_from_float(fc2))),
                                     d64_mul(C06490, d64_from_float(fc))),
                            C09988);
    float fcr = d64_to_float(fcr_d);
    uint64_t acr = d64_add(d64_add(d64_mul(CN39364, d64_from_float(fc2)),
                                   d64_mul(C18409, d64_from_float(fc))),
                          C09968);
    uint64_t ang = d64_mul(d64_mul(TWO_PI, d64_from_float(f)), d64_from_float(fcr));
    uint64_t tune = d64_div(d64_sub(D1, exp_d(d64_neg(ang))), THERMAL);
    *acr_out = d64_to_float(acr);
    *tune_out = d64_to_float(tune);
}

float s1_moog_res4(float res, float acr)
{
    return d64_to_float(d64_mul(d64_mul(C4, d64_from_float(res)), d64_from_float(acr)));
}

float s1_moog_half(float a, float b)
{
    return d64_to_float(d64_mul(d64_from_float(a + b), C05));
}

float s1_moog_tanh_store(float x)
{
    return d64_to_float(my_tanh(d64_mul(d64_from_float(x), THERMAL)));
}

float s1_moog_direct(float delay, float tune, float input, float stag)
{
    uint64_t th = my_tanh(d64_mul(d64_from_float(input), THERMAL));
    uint64_t y = d64_add(d64_from_float(delay),
                         d64_mul(d64_from_float(tune), d64_sub(th, d64_from_float(stag))));
    return d64_to_float(y);
}

float s1_moog_stored(float delay, float tune, float stored, float other)
{
    uint64_t y = d64_add(d64_from_float(delay),
                         d64_mul(d64_from_float(tune),
                                 d64_sub(d64_from_float(stored), d64_from_float(other))));
    return d64_to_float(y);
}

float s1_moog_tail(float delay, float tune, float stored, float input)
{
    uint64_t th = my_tanh(d64_mul(d64_from_float(input), THERMAL));
    uint64_t y = d64_add(d64_from_float(delay),
                         d64_mul(d64_from_float(tune), d64_sub(d64_from_float(stored), th)));
    return d64_to_float(y);
}
