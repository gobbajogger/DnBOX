/* SPDX-License-Identifier: GPL-3.0-only */
#include "band_morph.h"
#include "fastmath.h"

enum shape { TRIANGLE, SQUARE, PULSE, SAW };

static unsigned harmonic_budget(float increment)
{
    static const float upper_hz[] = {
        8.1787834f, 16.357567f, 32.715134f, 65.430267f, 131.25f, 262.5f,
        525.f, 1050.f, 2205.f, 4410.f, 11025.f, 22050.f
    };
    float hz = increment * 44100.f;
    unsigned band = 0;
    while (band < 11 && hz > upper_hz[band]) ++band;
    return (unsigned)(22050.f / upper_hz[band]);
}

/* Si(y) = integral of sin(t)/t from 0 to y. */
static float sine_integral(float y, float sine, float cosine)
{
    if (y <= 4.f) {
        float yy = y * y;
        float p = -1.f / 19615115520000.f;
        p = 1.f / 80951270400.f + yy * p;
        p = -1.f / 439084800.f + yy * p;
        p = 1.f / 3265920.f + yy * p;
        p = -1.f / 35280.f + yy * p;
        p = 1.f / 600.f + yy * p;
        p = -1.f / 18.f + yy * p;
        return y * (1.f + yy * p);
    }
    /* The sine and cosine amplitudes of pi/2 - Si(y) have Laplace
     * integrals. Combining their four Gauss-Laguerre terms gives these
     * rational polynomials in y^-2 with integral coefficients. */
    float inv = 1.f / y;
    float z = inv * inv;
    float den = 1.f + z * (112.f + z * (2160.f + z * (5760.f + 576.f * z)));
    float f = 1.f + z * (110.f + z * (1960.f + 3408.f * z));
    float g = 1.f + z * (106.f + z * (1608.f + 1200.f * z));
    return .25f * FM_TWO_PI - inv * (f * cosine + inv * g * sine) / den;
}

/* Evaluate sum(sin(k*x)/k), or sum(cos(k*x)/k^2), for 1 <= k <= n. */
static float fourier_sum(unsigned n, float turns, int cosine_series)
{
    float phase = turns - fm_floorf(turns + .5f);
    if (n <= 8) {
        float result = 0.f;
        for (unsigned k = 1; k <= n; ++k) {
            float h = (float)k;
            if (cosine_series) result += fm_sin_turns(h * phase + .25f) / (h * h);
            else result += fm_sin_turns(h * phase) / h;
        }
        return result;
    }
    float abs_phase = fm_fabsf(phase);
    float x = FM_TWO_PI * abs_phase;
    float d = (float)n + .5f;
    float inv = 1.f / d;
    float angle = d * abs_phase;
    float sine = fm_sin_turns(angle);
    float cosine = fm_sin_turns(angle + .25f);
    float si = sine_integral(d * x, sine, cosine);

    /* Integrating the Dirichlet kernel leaves the smooth correction
     * B(x) = 1/(2*sin(x/2)) - 1/x. Approximate B/x on [0,pi] as a
     * polynomial in (x/pi)^2. b1 is its product-rule derivative B'(x). */
    float z = 4.f * abs_phase * abs_phase;
    float b = x * (.04166666f + z * (.01199508f + z * (.00311124f +
              z * (.00082877f + z * (.00012203f + z * .00010993f)))));
    float b1 = .04166666f + z * (.03598524f + z * (.0155562f +
               z * (.00580139f + z * (.00109827f + z * .00120923f))));
    if (!cosine_series) {
        float sum = si - .5f * x - b * cosine * inv + b1 * sine * inv * inv;
        return phase < 0.f ? -sum : sum;
    }

    /* A second integral gives the cosine series. H_n^(2) at phase zero is
     * evaluated by Euler-Maclaurin about d = n+1/2, through d^-3. */
    float inv2 = inv * inv;
    float h2 = 1.64493406685f - inv + inv * inv2 / 12.f;
    return h2 - x * si - (cosine - 1.f) * inv + .25f * x * x
        + b * sine * inv2 + 2.f * (b1 * cosine - 1.f / 24.f) * inv2 * inv;
}

static float waveform(unsigned shape, float phase, unsigned n, float width)
{
    const float invpi = .3183098861837907f;
    if (shape == TRIANGLE) {
        if (n > 201) n = 201;
        return -4.f * invpi * invpi *
            (fourier_sum(n, phase, 1) - fourier_sum(n, phase + .5f, 1));
    }
    float saw = -2.f * invpi * fourier_sum(n, phase, 0);
    if (shape == SAW) return saw;
    if (shape == SQUARE) return saw + 2.f * invpi * fourier_sum(n, phase + .5f, 0);
    return 1.f - 2.f * width + saw + 2.f * invpi * fourier_sum(n, phase - width, 0);
}

static float morph_sample(unsigned n, float phase, float morph, float width)
{
    float position = 3.f * morph;
    unsigned lower = (unsigned)position;
    float fraction = position - (float)lower;
    float a = waveform(lower, phase, n, width);
    if (fraction == 0.f || lower == SAW) return a;
    float b = waveform(lower + 1, phase, n, width);
    return a + fraction * (b - a);
}

float s1_band_morph(float phase, float increment, float morph, float width)
{
    unsigned n = harmonic_budget(increment);
    if (n <= 2048) return morph_sample(n, phase, morph, width);

    /* Only in the lowest positive band do harmonics cross the Nyquist bin
     * of the 4096-point phase grid. Evaluate its two neighbors to retain
     * their folded energy and the observed inter-sample interpolation. */
    float scaled = phase * 4096.f;
    unsigned index = (unsigned)scaled;
    float a = morph_sample(n, (float)index / 4096.f, morph, width);
    float b = morph_sample(n, (float)((index + 1) & 4095) / 4096.f, morph, width);
    return a + (scaled - (float)index) * (b - a);
}

/* The unbandlimited bank's pulse edges and ramp seam lie on the 4096-point
 * grid. Only their neighboring interval needs interpolation; all other
 * segments are linear or constant. */
static float raw_wave(unsigned shape, float phase, unsigned index, float frac)
{
    if (shape == TRIANGLE) return 1.f - 4.f * fm_fabsf(phase - .5f);
    if (shape == SAW) {
        float ramp = 2.f * phase - 1.f;
        return index == 4095 ? ramp - 2.f * frac : ramp;
    }
    unsigned edge = shape == SQUARE ? 2048 : 512;
    if (index == edge - 1) return -1.f + 2.f * frac;
    if (index == 4095) return 1.f - 2.f * frac;
    return index < edge ? -1.f : 1.f;
}

float s1_raw_morph(float phase, float morph)
{
    float scaled = 4096.f * phase;
    unsigned index = (unsigned)scaled;
    float frac = scaled - (float)index;
    float position = 3.f * morph;
    unsigned lower = (unsigned)position;
    float blend = position - (float)lower;
    float a = raw_wave(lower, phase, index, frac);
    if (blend == 0.f || lower == SAW) return a;
    return a + blend * (raw_wave(lower + 1, phase, index, frac) - a);
}
