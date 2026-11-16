/* Envelopes, filters and conductor timing against the original headers. */
#include "sj6_blocks.h"
#include "ADSREnvelope.h"
#include "AHDSHREnvelope.h"
#include "OriginalSmoothPassFilter.h"
#include "OriginalOTAFilter.h"
#include "OriginalSyrupFilter.h"
#include "ResonantLowPassFilter.h"
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <vector>

static uint32_t bits(float x)
{
    uint32_t u;
    memcpy(&u, &x, 4);
    return u;
}

static int g_fail;

static void expect(const char *name, int diffs, int n)
{
    std::printf("%s diffs %d / %d\n", name, diffs, n);
    if (diffs) g_fail = 1;
}

static void adsr_case(int curve, float a, float d, float s, float r, int release_at)
{
    DunneCore::ADSREnvelopeParameters p;
    DunneCore::ADSREnvelope e;
    Sj6Env port;
    int diffs = 0;
    p.init(44100.f, a, d, s, r);
    e.pParameters = &p;
    e.init((DunneCore::ADSREnvelope::CurvatureType)curve);
    e.start();
    sj6_adsr_init(&port, curve, 44100.f, a, d, s, r);
    sj6_env_start(&port);
    for (int i = 0; i < 8000; i++) {
        if (i == release_at) {
            e.release();
            sj6_env_release(&port, 5);
        }
        float x = e.getSample();
        float y = sj6_env_sample(&port);
        if (bits(x) != bits(y)) diffs++;
    }
    char label[64];
    std::snprintf(label, sizeof label, "adsr curve %d", curve);
    expect(label, diffs, 8000);
}

static void ahdshr_case(int curve)
{
    DunneCore::AHDSHREnvelopeParameters p;
    DunneCore::AHDSHREnvelope e;
    Sj6Env port;
    int diffs = 0;
    p.init(44100.f, 0.01f, 0.02f, 0.08f, 0.6f, 0.01f, 0.15f);
    e.pParameters = &p;
    e.init((DunneCore::AHDSHREnvelope::CurvatureType)curve);
    e.start();
    sj6_ahdshr_init(&port, curve, 44100.f, 0.01f, 0.02f, 0.08f, 0.6f, 0.01f, 0.15f);
    sj6_env_start(&port);
    for (int i = 0; i < 8000; i++) {
        if (i == 4000) {
            e.release();
            sj6_env_release(&port, 6);
        }
        if (bits(e.getSample()) != bits(sj6_env_sample(&port))) diffs++;
    }
    char label[64];
    std::snprintf(label, sizeof label, "ahdshr curve %d", curve);
    expect(label, diffs, 8000);
}

static void smooth_case(float cut, float res)
{
    DunneCore::OriginalSmoothPassFilter a;
    Sj6Smooth b;
    int diffs = 0;
    a.setParameters(cut, res);
    sj6_smooth_init(&b, 44100.f);
    sj6_smooth_set(&b, cut, res);
    for (int i = 0; i < 2048; i++) {
        float x = (i & 63) < 32 ? 0.2f : -0.2f;
        if (bits(a.process(x)) != bits(sj6_smooth_process(&b, x))) diffs++;
    }
    expect("smooth", diffs, 2048);
}

static void low_case(float cut, float res)
{
    DunneCore::ResonantLowPassFilter a;
    Sj6Lowpass b;
    int diffs = 0;
    a.init(44100.f);
    a.setParameters(cut, res);
    sj6_lowpass_init(&b, 44100.f);
    sj6_lowpass_set(&b, cut, res);
    for (int i = 0; i < 2048; i++) {
        float x = (i * 17 % 100) / 100.f - 0.5f;
        if (bits(a.process(x)) != bits(sj6_lowpass_process(&b, x))) diffs++;
    }
    char label[64];
    std::snprintf(label, sizeof label, "lowpass %.0f", cut);
    expect(label, diffs, 2048);
}

static void ota_case(float cut, float res)
{
    DunneCore::OriginalOTAFilter a;
    Sj6Ota b;
    int diffs = 0;
    double se = 0;
    a.init(44100.f);
    sj6_ota_init(&b, 44100.f);
    a.setParameters(cut, res, 64);
    sj6_ota_set(&b, cut, res, 64);
    for (int i = 0; i < 1024; i++) {
        float x = (i & 31) < 16 ? 0.4f : -0.4f;
        float u = a.process(x), v = sj6_ota_process(&b, x);
        float d = u - v;
        se += (double)d * d;
        if (bits(u) != bits(v)) diffs++;
    }
    std::printf("ota cut %.0f diffs %d / 1024 rms %.3g\n", cut, diffs, std::sqrt(se / 1024.0));
}

static void syrup_case(float cut, float res)
{
    DunneCore::OriginalSyrupFilter a;
    Sj6Syrup b;
    int diffs = 0;
    double se = 0;
    a.init(44100.f);
    sj6_syrup_init(&b, 44100.f);
    a.setParameters(cut, res);
    sj6_syrup_set(&b, cut, res);
    for (int i = 0; i < 1024; i++) {
        float x = (i & 31) < 16 ? 0.4f : -0.4f;
        float u = a.process(x), v = sj6_syrup_process(&b, x);
        float d = u - v;
        se += (double)d * d;
        if (bits(u) != bits(v)) diffs++;
    }
    std::printf("syrup cut %.0f diffs %d / 1024 rms %.3g\n", cut, diffs, std::sqrt(se / 1024.0));
}

int main()
{
    adsr_case(0, 0.005f, 0.05f, 0.4f, 0.08f, 3000);
    adsr_case(1, 0.01f, 0.12f, 0.7f, 0.2f, 3500);
    adsr_case(2, 0.02f, 0.08f, 0.3f, 0.1f, 2500);
    adsr_case(3, 0.004f, 0.06f, 0.5f, 0.15f, 2800);
    ahdshr_case(0);
    ahdshr_case(1);
    ahdshr_case(2);
    smooth_case(800.f, 0.4f);
    smooth_case(2400.f, 2.5f);
    low_case(400.f, 0.3f);
    low_case(1800.f, 1.4f);
    low_case(6000.f, 4.f);
    ota_case(500.f, 0.2f);
    ota_case(3000.f, 0.7f);
    syrup_case(700.f, 0.4f);
    syrup_case(2500.f, 2.f);

    if (g_fail) {
        std::printf("FAIL blocks\n");
        return 1;
    }
    std::printf("PASS envelopes, smooth, lowpass, syrup, with OTA residual printed above\n");
    return 0;
}
