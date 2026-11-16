// Original-compatible behavioral implementation; qualified scope in README.md.
// MIT sampler integration preserves AudioKit/Dunne source credits in LICENSE.
#pragma once
#include <cstdint>
#include "FunctionTable.h"
#include "f64_bits.hpp"
namespace DunneCore {
struct OriginalSmoothPassFilter {
    std::uint64_t c[5]{}, target[5]{}, smoothing{}, x1{}, x2{}, y1{}, y2{};
    bool first{true};
    std::uint64_t rate{0x40e5880000000000ULL}, cutoff{0x40d3880000000000ULL}, resonance{0x3fb999999999999aULL};
    OriginalSmoothPassFilter() { init(44100.f); }
    static FunctionTable& sine() {
        static FunctionTable t;
        if (t.waveTable.empty()) { t.init(2048); t.sinusoid(); }
        return t;
    }
    void calculate(std::uint64_t frequency, std::uint64_t res, std::uint64_t* out) {
        using namespace original_conductor_v1;
        const std::uint64_t one = 0x3ff0000000000000ULL;
        if (f64_lt(frequency, 0x4028000000000000ULL)) frequency = 0x4028000000000000ULL;
        std::uint64_t normalized = f64_div(f64_add(frequency, frequency), rate);
        if (f64_gt(normalized, 0x3fefae147ae147aeULL)) normalized = 0x3fefae147ae147aeULL;
        const float phase = f64_to_f32(f64_mul(0x3fe0000000000000ULL, normalized));
        if (f64_lt(res, 0)) res = 0;
        if (f64_gt(res, 0x4034000000000000ULL)) res = 0x4034000000000000ULL;
        const std::uint64_t k = f64_mul(f64_pow(res, 0x3ffe8f5c28f5c28fULL), f64_from_f32(sine().interp_cyclic(phase)));
        const std::uint64_t u = f64_mul(0x3fe0000000000000ULL, f64_div(f64_sub(one, k), f64_add(one, k)));
        const std::uint64_t v = f64_add(0x3fe0000000000000ULL, u);
        const std::uint64_t w = f64_mul(v, f64_from_f32(sine().interp_cyclic(phase + 0.25f)));
        const std::uint64_t z = f64_mul(f64_sub(v, w), 0x3fd0000000000000ULL);
        out[0] = f64_add(z, z);
        out[1] = f64_mul(z, 0x4010000000000000ULL);
        out[2] = out[0];
        out[3] = f64_mul(0xc000000000000000ULL, w);
        out[4] = f64_add(u, u);
    }
    void init(float sampleRate) {
        using namespace original_conductor_v1;
        rate = f64_from_f32(sampleRate);
        smoothing = f64_sub(0x3ff0000000000000ULL, f64_exp(f64_div(0xc0578fdb9effea46ULL, rate)));
        x1 = x2 = y1 = y2 = 0; first = true;
        cutoff = 0x40d3880000000000ULL; resonance = 0x3fb999999999999aULL;
        calculate(cutoff, resonance, c);
        for (int i = 0; i < 5; ++i) target[i] = c[i];
    }
    void updateSampleRate(float sampleRate) {
        using namespace original_conductor_v1;
        rate = f64_from_f32(sampleRate);
        smoothing = f64_sub(0x3ff0000000000000ULL, f64_exp(f64_div(0xc0578fdb9effea46ULL, rate)));
        setParameters(f64_to_f32(cutoff), f64_to_f32(resonance));
    }
    void setParameters(float frequency, float res) {
        using namespace original_conductor_v1;
        cutoff = f64_from_f32(frequency);
        resonance = f64_from_f32(res);
        if (f64_lt(cutoff, 0x4028000000000000ULL)) cutoff = 0x4028000000000000ULL;
        if (f64_lt(resonance, 0)) resonance = 0;
        if (f64_gt(resonance, 0x4034000000000000ULL)) resonance = 0x4034000000000000ULL;
        calculate(cutoff, resonance, target);
    }
    float process(float input) {
        using namespace original_conductor_v1;
        if (first) { for (int i = 0; i < 5; ++i) c[i] = target[i]; first = false; }
        for (int i = 0; i < 5; ++i) c[i] = f64_fma(smoothing, f64_sub(target[i], c[i]), c[i]);
        const std::uint64_t in = f64_from_f32(input);
        const std::uint64_t out = f64_fma(f64_neg(c[4]), y2, f64_fma(f64_neg(c[3]), y1, f64_fma(c[2], x2, f64_fma(c[0], in, f64_mul(c[1], x1)))));
        const float result = f64_to_f32(out);
        x2 = x1; x1 = in; y2 = y1; y1 = f64_from_f32(result);
        return result;
    }
};
}
