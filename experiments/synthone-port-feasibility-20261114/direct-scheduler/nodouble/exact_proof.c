/* Host proof that the no-double formulas store the same float as the original
 * double expressions. Build with the host compiler and libm. Not part of the
 * firmware link.
 *
 *   cc -O2 -ffp-contract=off -I. exact_proof.c exact.c d64.c -lm
 *
 * The stride checks call the integer-binary64 implementation. The midpoint
 * scan uses libm and an absolute error bound: a result farther than the bound
 * from a binary32 midpoint rounds the same way for every approximation inside
 * that bound. The series remainder after the terms in exact.c is far below
 * the bound on these domains; each binary64 step adds at most half an ulp. */
#include "exact.h"
#include <math.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

static int differ(float a, float b) { return memcmp(&a, &b, 4) != 0; }

static int near_midpoint(double truth, double bound)
{
    float lo = (float)truth;
    if (truth == (double)lo) return 0;
    float hi = nextafterf(lo, truth > (double)lo ? INFINITY : -INFINITY);
    double mid = 0.5 * ((double)lo + (double)hi);
    double dist = truth - mid;
    if (dist < 0) dist = -dist;
    return dist < bound;
}

static void finish(const char *name, uint64_t n, uint64_t bad, uint64_t close)
{
    printf("{\"check\":\"%s\",\"samples\":%llu,\"mismatches\":%llu,\"close_midpoints\":%llu}\n",
           name, (unsigned long long)n, (unsigned long long)bad, (unsigned long long)close);
    if (bad) exit(1);
}

static void exp2_domain(void)
{
    uint64_t n = 0, bad = 0, close = 0;
    /* Every float in [-32, 32]. The series plus reduction error is < 1e-12. */
    for (uint32_t u = 0; u <= 0x42000000u; u++) {
        float x; memcpy(&x, &u, 4);
        if (x > 32.f) break;
        double truth = exp2((double)x);
        n += 2;
        if (near_midpoint(truth, 1e-12)) {
            close++;
            if (differ(s1_exp2_f64(x), (float)truth)) bad++;
        } else if ((u & 127u) == 0 && differ(s1_exp2_f64(x), (float)truth)) bad++;
        float y = -x;
        double t2 = exp2((double)y);
        if (near_midpoint(t2, 1e-12)) {
            close++;
            if (differ(s1_exp2_f64(y), (float)t2)) bad++;
        } else if ((u & 127u) == 0 && differ(s1_exp2_f64(y), (float)t2)) bad++;
    }
    finish("exp2[-32,32]", n, bad, close);
}

static void log2_domain(void)
{
    uint64_t n = 0, bad = 0, close = 0;
    for (uint32_t u = 0x00800000u; u <= 0x477fe000u; u += 16u) {
        float x; memcpy(&x, &u, 4);
        if (!(x > 0.f) || x > 48000.f) continue;
        double truth = log2((double)x);
        int close_one = near_midpoint(truth, 1e-10);
        if (close_one) close++;
        if (close_one || (u & 255u) == 0) {
            if (differ(s1_log2_f64(x), (float)truth)) bad++;
        }
        n++;
    }
    finish("log2(0,48000]", n, bad, close);
}

static void lfo_domain(void)
{
    uint64_t n = 0, bad = 0;
    const double pi = 3.14159265358979323846;
    for (uint32_t u = 0; u <= 0x3f800000u; u += 32u) {
        float p; memcpy(&p, &u, 4);
        if (p > 1.f) break;
        float got = s1_lfo_sine(p);
        float want = (float)sin(((double)p) * pi * 2.f);
        if (differ(got, want)) bad++;
        n++;
    }
    finish("lfo phase[0,1] stride 32", n, bad, 0);
}

static void midi_domain(void)
{
    uint64_t n = 0, bad = 0;
    for (uint32_t u = 0; u < 0x43000000u; u += 64u) {
        float nn; memcpy(&nn, &u, 4);
        if (nn >= 128.f) break;
        float got = s1_midi_cps_f64(nn);
        float want = (float)(pow(2.0, ((double)nn - 69.0) / 12.0) * 440.0);
        if (differ(got, want)) bad++;
        n++;
    }
    finish("midi nn[0,128) stride 64", n, bad, 0);
}

static void port_adsr(void)
{
    uint64_t n = 0, bad = 0;
    float rates[3] = {44100.f, 48000.f, 96000.f};
    for (int r = 0; r < 3; r++) {
        float one = s1_port_onedsr(rates[r]);
        float want_one = (float)(1.0 / (double)rates[r]);
        if (differ(one, want_one)) bad++;
        n++;
        for (uint32_t u = 0x358637bdu; u <= 0x3f7d70a4u; u += 64u) {
            float ht; memcpy(&ht, &u, 4);
            if (ht < 1e-6f || ht > 0.99f) continue;
            float c1, c2; s1_port_coeffs(one, ht, &c1, &c2);
            float wc2 = (float)pow(0.5, (double)(want_one / ht));
            float wc1 = (float)(1.0 - (double)wc2);
            if (differ(c1, wc1) || differ(c2, wc2)) bad++;
            float tau; memcpy(&tau, &u, 4);
            if (tau >= 0.0005f && tau <= 2.f) {
                float g = s1_adsr_pole(tau, rates[r]);
                float w = (float)exp(-1.0 / (double)(tau * rates[r]));
                if (differ(g, w)) bad++;
                n++;
            }
            n++;
        }
    }
    finish("port+adsr three rates", n, bad, 0);
}

static void filters(void)
{
    uint64_t n = 0, bad = 0;
    float rates[3] = {44100.f, 48000.f, 96000.f};
    const double root2 = 1.4142135623730950488;
    const double pi = 3.14159265358979323846;
    for (int r = 0; r < 3; r++) {
        float pid, tpid; s1_butbp_consts(rates[r], &pid, &tpid);
        float wp = (float)(pi / (double)rates[r]);
        float wt = (float)((2.0 * pi) / (double)rates[r]);
        if (differ(pid, wp) || differ(tpid, wt)) bad++;
        for (uint32_t u = 0x42800000u; u <= 0x46ac2000u; u += 128u) {
            float f; memcpy(&f, &u, 4);
            if (f < 64.f || f > 22050.f) continue;
            float a[6];
            s1_butbp_coeffs(pid, tpid, f, f, a);
            float c = (float)(1.0 / tan((double)(pid * f)));
            float d = (float)(2.0 * cos((double)(tpid * f)));
            float a1 = (float)(1.0 / (1.0 + (double)c));
            float a4 = -c * d * a1;
            float a5 = (float)(((double)c - 1.0) * (double)a1);
            if (differ(a[1], a1) || differ(a[4], a4) || differ(a[5], a5)) bad++;
            s1_buthp_coeffs(pid, f, a);
            c = (float)tan((double)(pid * f));
            float cc = c * c;
            a1 = (float)(1.0 / (1.0 + root2 * (double)c + (double)cc));
            a4 = (float)(2.0 * ((double)cc - 1.0) * (double)a1);
            a5 = (float)((1.0 - root2 * (double)c + (double)cc) * (double)a1);
            if (differ(a[1], a1) || differ(a[4], a4) || differ(a[5], a5)) bad++;
            float ac, tu; s1_moog_coeffs(f, rates[r], &ac, &tu);
            float fc = f / rates[r];
            float ff = (float)(0.5 * (double)fc);
            float fc2 = fc * fc, fc3 = fc2 * fc;
            float fcr = (float)(1.8730 * (double)fc3 + 0.4955 * (double)fc2 - 0.6490 * (double)fc + 0.9988);
            float acr = (float)(-3.9364 * (double)fc2 + 1.8409 * (double)fc + 0.9968);
            float tune = (float)((1.0 - exp(-((2 * pi) * (double)ff * (double)fcr))) / 0.000025);
            if (differ(ac, acr) || differ(tu, tune)) bad++;
            n++;
        }
    }
    finish("butbp+buthp+moog three rates stride 128", n, bad, 0);
}

int main(void)
{
    exp2_domain();
    log2_domain();
    lfo_domain();
    midi_domain();
    port_adsr();
    filters();
    return 0;
}
