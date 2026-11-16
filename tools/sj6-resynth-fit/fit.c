#include "fit.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int best_lag(const int32_t *x, int n)
{
    int start = n / 5, len = n / 3, lag, best = 0;
    double r0 = 0, bestv = 0;
    if (len > 4096) len = 4096;
    if (start + len > n) return 0;
    for (lag = 0; lag < len; lag++) r0 += (double)x[start + lag] * x[start + lag];
    if (r0 < 1) return 0;
    for (lag = 32; lag < 500 && lag < len; lag++) {
        double s = 0;
        int i;
        for (i = 0; i < len - lag; i++) s += (double)x[start + i] * x[start + i + lag];
        if (s > bestv) { bestv = s; best = lag; }
    }
    if (bestv < 0.3 * r0) return 0;
    return best;
}

static double dot_sine(const int32_t *x, int n, uint32_t inc, uint32_t phase, int a, int b, int cosine)
{
    double s = 0;
    int i;
    uint32_t extra = cosine ? 0x40000000u : 0;
    if (b > n) b = n;
    for (i = a; i < b; i++) s += (double)x[i] * sj6_sine_at((uint32_t)i * inc + phase + extra);
    return s;
}

static double inc_energy(const int32_t *x, int n, uint32_t inc, int a, int b)
{
    return fabs(dot_sine(x, n, inc, 0, a, b, 0)) + fabs(dot_sine(x, n, inc, 0, a, b, 1));
}

static uint32_t search_inc(const int32_t *x, int n, int lag)
{
    int start, len, i, L;
    double rm = 0, r0 = 0, rp = 0, denom, delta, period;
    uint32_t center, inc, best;
    double bestv = -1;
    int a, b;
    if (lag < 17) return 0;
    start = n / 3;
    len = n / 2;
    if (len > 8192) len = 8192;
    if (start + len + lag + 1 > n) len = n - start - lag - 1;
    if (len < lag + 8) return (uint32_t)((1ull << 32) / (unsigned)lag);
    for (i = 0; i < len; i++) {
        rm += (double)x[start + i] * x[start + i + lag - 1];
        r0 += (double)x[start + i] * x[start + i + lag];
        rp += (double)x[start + i] * x[start + i + lag + 1];
    }
    denom = rm - 2 * r0 + rp;
    delta = fabs(denom) < 1e-6 ? 0 : 0.5 * (rm - rp) / denom;
    if (delta > 1) delta = 1;
    if (delta < -1) delta = -1;
    period = lag + delta;
    if (period < 8) period = lag;
    center = (uint32_t)(4294967296.0 / period + 0.5);
    a = start;
    b = start + (len > 4096 ? 4096 : len);
    best = center;
    (void)L;
    for (inc = center > 3000 ? center - 3000 : 1; inc < center + 3000; inc += 2) {
        double v = inc_energy(x, n, inc, a, b);
        if (v > bestv) { bestv = v; best = inc; }
    }
    center = best;
    bestv = -1;
    for (inc = center > 4 ? center - 4 : 1; inc < center + 4; inc++) {
        double v = inc_energy(x, n, inc, a, b);
        if (v > bestv) { bestv = v; best = inc; }
    }
    return best;
}

static uint32_t phase_of(const int32_t *x, int n, uint32_t inc)
{
    int a = n / 3, b = a + (n / 3 > 4096 ? 4096 : n / 3);
    if (b > n) b = n;
    double ss = dot_sine(x, n, inc, 0, a, b, 0);
    double sc = dot_sine(x, n, inc, 0, a, b, 1);
    double ang = atan2(sc, ss);
    if (ang < 0) ang += 2 * 3.14159265358979323846;
    return (uint32_t)(ang / (2 * 3.14159265358979323846) * 4294967296.0);
}

static int32_t env_shape(int n, int attack, int decay, int which)
{
    /* which 0: peak basis, 1: sustain basis, scaled by 32767 */
    if (attack < 1) attack = 1;
    if (decay < 1) decay = 1;
    if (which == 0) {
        if (n < attack) return (int32_t)((32767ll * n) / attack);
        if (n < attack + decay) return 32767 - (int32_t)((32767ll * (n - attack)) / decay);
        return 0;
    }
    if (n < attack) return 0;
    if (n < attack + decay) return (int32_t)((32767ll * (n - attack)) / decay);
    return 32767;
}

static int solve(double *A, double *b, int m)
{
    int i, j, k;
    for (k = 0; k < m; k++) {
        int piv = k;
        double d;
        for (i = k + 1; i < m; i++) if (fabs(A[i * m + k]) > fabs(A[piv * m + k])) piv = i;
        if (fabs(A[piv * m + k]) < 1e-12) return -1;
        if (piv != k) {
            for (j = k; j < m; j++) {
                double t = A[k * m + j];
                A[k * m + j] = A[piv * m + j];
                A[piv * m + j] = t;
            }
            d = b[k]; b[k] = b[piv]; b[piv] = d;
        }
        for (i = k + 1; i < m; i++) {
            double f = A[i * m + k] / A[k * m + k];
            for (j = k; j < m; j++) A[i * m + j] -= f * A[k * m + j];
            b[i] -= f * b[k];
        }
    }
    for (i = m - 1; i >= 0; i--) {
        for (j = i + 1; j < m; j++) b[i] -= A[i * m + j] * b[j];
        b[i] /= A[i * m + i];
    }
    return 0;
}

static void quantize(double g, int16_t *out)
{
    if (g > 32767) g = 32767;
    if (g < -32768) g = -32768;
    *out = (int16_t)lrint(g);
}

static void solve_gains(const int32_t *x, int n, Sj6Voice *v, int attack, int decay, int np)
{
    int m = np * 2, i, j, p, t;
    double *A = calloc((size_t)m * m, sizeof(double));
    double *b = calloc((size_t)m, sizeof(double));
    double *col = calloc((size_t)m, sizeof(double));
    if (!A || !b || !col) { free(A); free(b); free(col); return; }
    for (t = 0; t < n; t += (n > 8192 ? n / 8192 : 1)) {
        double xt = x[t];
        for (p = 0; p < np; p++) {
            int32_t s = sj6_sine_at((uint32_t)t * v->partial[p].inc + v->partial[p].phase0);
            col[p * 2] = (double)env_shape(t, attack, decay, 0) * s / 32767.0 / 32767.0;
            col[p * 2 + 1] = (double)env_shape(t, attack, decay, 1) * s / 32767.0 / 32767.0;
        }
        for (i = 0; i < m; i++) {
            b[i] += col[i] * xt;
            for (j = 0; j < m; j++) A[i * m + j] += col[i] * col[j];
        }
    }
    if (!solve(A, b, m)) {
        for (p = 0; p < np; p++) {
            quantize(b[p * 2], &v->partial[p].peak);
            quantize(b[p * 2 + 1], &v->partial[p].sustain);
            v->partial[p].attack = (uint16_t)attack;
            v->partial[p].decay = (uint16_t)decay;
        }
    }
    free(A); free(b); free(col);
}

static double render_rms(const int32_t *x, int n, const Sj6Voice *v)
{
    Sj6State st;
    int32_t *y = malloc((size_t)n * sizeof(int32_t));
    double se = 0;
    int i;
    if (!y) return 1e300;
    sj6_state_init(&st, v);
    sj6_render(v, &st, y, n, 0);
    for (i = 0; i < n; i++) {
        double d = (double)x[i] - y[i];
        se += d * d;
    }
    free(y);
    return sqrt(se / n);
}

int sj6_fit(const int32_t *x, int n, int rate, int loop_start, int loop_end, const char *name, Sj6Voice *out)
{
    static const int attacks[] = {128, 256, 512, 1000, 1500, 2000, 3000, 4000, 6000, 8000};
    static const int decays[] = {256, 512, 1000, 2000, 4000, 6000, 8000, 12000};
    int lag, np, h, ai, di, pole, fit_n;
    uint32_t f0;
    double best, src;
    Sj6Voice trial;
    if (!x || n < 64 || !out) return -1;
    memset(out, 0, sizeof *out);
    snprintf(out->name, sizeof out->name, "%s", name ? name : "voice");
    out->noise_seed = 1u;
    fit_n = n > 16384 ? 16384 : n;
    lag = best_lag(x, n);
    f0 = search_inc(x, n, lag);
    if (loop_end > loop_start && loop_start >= 0) out->loop_len = (uint32_t)(loop_end - loop_start);
    np = 0;
    if (f0) {
        for (h = 1; h <= SJ6_PARTIALS && np < SJ6_PARTIALS; h++) {
            uint64_t inc = (uint64_t)f0 * h;
            if (inc == 0 || inc >= (1ull << 31)) break;
            out->partial[np].inc = (uint32_t)inc;
            out->partial[np].phase0 = phase_of(x, fit_n, (uint32_t)inc);
            if (out->loop_len) {
                uint32_t cycles = (uint32_t)(((uint64_t)out->partial[np].inc * out->loop_len + 0x80000000ull) >> 32);
                if (cycles) out->partial[np].inc = (uint32_t)(((uint64_t)cycles << 32) / out->loop_len);
            }
            np++;
        }
    }
    best = 1e300;
    for (ai = 0; ai < (int)(sizeof attacks / sizeof attacks[0]); ai++) {
        if (attacks[ai] >= fit_n) continue;
        for (di = 0; di < (int)(sizeof decays / sizeof decays[0]); di++) {
            double err;
            if (attacks[ai] + decays[di] >= fit_n) continue;
            trial = *out;
            solve_gains(x, fit_n, &trial, attacks[ai], decays[di], np > 0 ? np : 1);
            err = render_rms(x, fit_n, &trial);
            if (err < best) { best = err; *out = trial; }
        }
    }
    if (!np) {
        double ea = 0, es = 0;
        int i, ns = 0, na = fit_n / 5;
        if (na < 1) na = 1;
        for (i = 0; i < na; i++) ea += (double)x[i] * x[i];
        for (i = fit_n / 2; i < fit_n; i++) { es += (double)x[i] * x[i]; ns++; }
        out->noise_peak = (int16_t)sqrt(ea / na);
        out->noise_sustain = ns ? (int16_t)sqrt(es / ns) : 0;
        out->partial[0].attack = 1;
        out->partial[0].decay = 1;
    }
    best = render_rms(x, fit_n, out);
    for (pole = 0; pole <= 6; pole++) {
        trial = *out;
        trial.noise_pole = (uint16_t)pole;
        {
            Sj6State st;
            int32_t *y = malloc((size_t)fit_n * sizeof(int32_t));
            double ea = 0, es = 0;
            int i, na, ns = 0;
            if (!y) break;
            trial.noise_peak = trial.noise_sustain = 0;
            sj6_state_init(&st, &trial);
            sj6_render(&trial, &st, y, fit_n, 0);
            na = trial.partial[0].attack + trial.partial[0].decay;
            if (na < 1) na = 1;
            if (na > fit_n / 2) na = fit_n / 2;
            for (i = 0; i < na; i++) { double d = (double)x[i] - y[i]; ea += d * d; }
            for (i = fit_n / 2; i < fit_n; i++) { double d = (double)x[i] - y[i]; es += d * d; ns++; }
            free(y);
            trial.noise_peak = (int16_t)sqrt(ea / na);
            trial.noise_sustain = ns ? (int16_t)sqrt(es / ns) : 0;
        }
        {
            double err = render_rms(x, fit_n, &trial);
            Sj6Voice quiet = trial;
            quiet.noise_peak = quiet.noise_sustain = 0;
            if (render_rms(x, fit_n, &quiet) < err) { trial.noise_peak = trial.noise_sustain = 0; err = render_rms(x, fit_n, &trial); }
            if (err < best) { best = err; *out = trial; }
        }
    }
    src = 0;
    {
        int i;
        for (i = 0; i < fit_n; i++) src += (double)x[i] * x[i];
        src = sqrt(src / fit_n);
    }
    if (src > 1 && render_rms(x, fit_n, out) > src) {
        /* Keep a sounding model rather than a silent one. */
    }
    for (h = 0; h < SJ6_PARTIALS; h++) {
        if (!out->partial[h].peak && !out->partial[h].sustain) {
            out->partial[h].inc = 0;
            out->partial[h].phase0 = 0;
        }
    }
    (void)rate;
    return 0;
}
