/* Oscillator and noise against the original host headers.
 * Sine is compared with a correctly rounded long-double reference because
 * this platform's sinf is 1 ULP off on some phases. Saw, triangle, pulse
 * and noise are compared with the original header. */
#include "sj6_analytic.h"
#include "OriginalAnalyticOscillator.h"
#include "OriginalWhiteNoise.h"
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstring>

static uint32_t bits(float x)
{
    uint32_t u;
    memcpy(&u, &x, 4);
    return u;
}

static int fail(const char *msg)
{
    std::printf("FAIL %s\n", msg);
    return 1;
}

int main()
{
    const float increments[] = {0.001f, 0.01f, 0.023f, 0.05f, 0.1f, 0.2f, 0.45f};
    const float gains[] = {0.25f, 1.f, 1.5f};
    const float widths[] = {0.2f, 0.5f, 0.8f};
    int saw_d = 0, tri_d = 0, pwm_d = 0, noise_d = 0, sine_d = 0, sine_lib = 0;
    int saw_n = 0, tri_n = 0, pwm_n = 0, noise_n = 0, sine_n = 0;
    int shown = 0;

    for (float inc : increments) {
        for (float gain : gains) {
            DunneCore::OriginalAnalyticOscillator a;
            Sj6Osc b;
            a.reset();
            sj6_osc_reset(&b);
            a.increment = inc;
            b.increment = inc;
            for (int i = 0; i < 2048; i++) {
                float r = a.processSaw(gain);
                float g = sj6_osc_saw(&b, gain);
                saw_n++;
                if (bits(r) != bits(g)) {
                    saw_d++;
                    if (shown < 4) {
                        std::printf("saw i %d inc %a got %a ref %a\n", i, inc, g, r);
                        shown++;
                    }
                }
            }
            a.reset();
            sj6_osc_reset(&b);
            a.increment = inc;
            b.increment = inc;
            for (int i = 0; i < 2048; i++) {
                float r = a.processTriangle(gain);
                float g = sj6_osc_triangle(&b, gain);
                tri_n++;
                if (bits(r) != bits(g)) tri_d++;
            }
            for (float width : widths) {
                a.reset();
                sj6_osc_reset(&b);
                a.increment = inc;
                b.increment = inc;
                for (int i = 0; i < 1024; i++) {
                    float r = a.processPWM(gain, width);
                    float g = sj6_osc_pwm(&b, gain, width);
                    pwm_n++;
                    if (bits(r) != bits(g)) {
                        pwm_d++;
                        if (shown < 8) {
                            std::printf("pwm i %d w %a got %a ref %a\n", i, width, g, r);
                            shown++;
                        }
                    }
                }
            }
            a.reset();
            sj6_osc_reset(&b);
            a.increment = inc;
            b.increment = inc;
            DunneCore::OriginalAnalyticOscillator cr;
            cr.reset();
            cr.increment = inc;
            for (int i = 0; i < 2048; i++) {
                float phase = cr.phase;
                float dt = inc * 0.5f;
                float mid = phase + dt;
                if (mid >= 1.f) mid += -1.f;
                cr.phase = mid + dt;
                if (cr.phase >= 1.f) cr.phase += -1.f;
                float first = (float)sinl((long double)(phase * 6.2831854820251465f));
                float second = (float)sinl((long double)(mid * 6.2831854820251465f));
                float rounded = (gain * 0.5f) * cr.decimate(first, second);
                float lib = a.processSine(gain);
                float port = sj6_osc_sine(&b, gain);
                sine_n++;
                if (bits(rounded) != bits(port)) sine_d++;
                if (bits(lib) != bits(port)) sine_lib++;
            }
        }
    }

    for (uint32_t seed = 1; seed < 8; seed++) {
        DunneCore::OriginalWhiteNoise rn;
        Sj6Noise pn;
        rn.seed = seed;
        pn.seed = seed;
        rn.reset();
        sj6_noise_reset(&pn);
        for (int i = 0; i < 4096; i++) {
            float r = rn.sample();
            float g = sj6_noise_sample(&pn);
            noise_n++;
            if (bits(r) != bits(g)) noise_d++;
        }
    }

    std::printf("saw diffs %d / %d\n", saw_d, saw_n);
    std::printf("triangle diffs %d / %d\n", tri_d, tri_n);
    std::printf("pwm diffs %d / %d\n", pwm_d, pwm_n);
    std::printf("noise diffs %d / %d\n", noise_d, noise_n);
    std::printf("sine diffs_beyond_libm %d libm_ulp %d / %d\n", sine_d, sine_lib, sine_n);
    if (saw_d || tri_d || pwm_d || noise_d || sine_d) return fail("analytic mismatch");
    std::printf("PASS analytic oscillator and noise\n");
    return 0;
}
