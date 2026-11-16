// Host-only inventory and fidelity probe for a bounded Synth One FX history.
// The stimuli in fixture_reference() and fixture_lifecycle() follow
// experiments/synthone-port-feasibility-20261114/full-reference/fx_reference_test.cpp
// and fx_lifecycle_test.cpp. This file does not change firmware or the pool.
#define S1FX_EXISTING_PARAMETERS
#include "S1Parameter.h"
#include "source_fx.hpp"
#include "fastmath.h"

#include <algorithm>
#include <cmath>
#include <complex>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

namespace {

constexpr int kPoolDelaySamples = 79200; // firmware/src/app/engine.c DLY_LEN
constexpr int kWiden48 = 2400;           // round(0.05 * 48000), sp_delay_init

// Faust state copied from Soundpipe phaser.c / compressor.c so sizeof is the
// history, not a host malloc header. Both structs are floats and ints only.
struct PhaserFaust {
    float fRec4[3], fRec3[3], fRec2[3], fRec1[3];
    float fRec11[3], fRec10[3], fRec9[3], fRec8[3];
    int iVec0[2];
    float fRec5[2], fRec6[2], fRec0[2], fRec7[2];
    float fHslider0, fCheckbox0, fHslider1;
    int fSamplingFreq, iConst0;
    float fConst1;
    float fHslider2, fHslider3, fHslider4, fHslider5, fConst2, fHslider6, fHslider7, fCheckbox1;
};
struct CompFaust {
    float fRec2[2], fRec1[2], fRec0[2];
    int fSamplingFreq, iConst0;
    float fConst1, fHslider0, fHslider1, fConst2, fHslider2, fHslider3;
};

// Control block the design would keep beside a shortened dly_buf.
// Pointers are uint32_t so this sizeof is the 32-bit layout, not the host ABI.
struct TargetLineMeta {
    float del, maxdel, feedback, prev, sr;
    uint32_t physical;
    uint32_t virtual_left;
    uint32_t physical_left;
    uint32_t written;
    int32_t clips_sat;
};
struct ProposedControl {
    float bitcrush[4];
    float pan_phase, pan_amp;
    PhaserFaust phaser;
    uint32_t phaser_args[10];
    uint32_t phaser_knobs[10];
    int32_t phaser_argpos;
    sp_moogladder moog[2];
    sp_buthp hipass[2];
    float crossfade_pos[4];
    CompFaust comp[6];
    uint32_t comp_args[6][4];
    uint32_t comp_knobs[6][4];
    int32_t comp_argpos[6];
    TargetLineMeta line[4];
    TargetLineMeta widen;
};

struct Store {
    std::vector<float> f;
    std::vector<int16_t> i;
    bool use_i16 = false;
    int physical = 0;
    int virtual_maxd = 0;
    uint32_t virtual_left = 0;
    uint32_t physical_left = 0;
    uint32_t written = 0;
    float prev = 0.f;
    float maxdel = 10.f;
    float sr = 48000.f;
    uint64_t clips = 0;
};

float rd_at(const Store& s, int idx) {
    if (s.use_i16) return (float)s.i[(unsigned)idx] * (1.f / 2048.f);
    return s.f[(unsigned)idx];
}
void wr_at(Store& s, int idx, float x) {
    if (!s.use_i16) { s.f[(unsigned)idx] = x; return; }
    int32_t k = (int32_t)(x * 2048.f);
    if (k > 32767) { k = 32767; s.clips++; }
    else if (k < -32767) { k = -32767; s.clips++; }
    s.i[(unsigned)idx] = (int16_t)k;
}

uint32_t age_of(const Store& s, int original) {
    int d = (int)s.virtual_left - original;
    int old = s.virtual_maxd;
    return (uint32_t)(d < 0 ? d + old : d);
}
float read_age(const Store& s, int original) {
    uint32_t d = age_of(s, original);
    if (d >= s.written) return 0.f;
    uint32_t q = (s.physical_left + (uint32_t)s.physical - d) % (uint32_t)s.physical;
    return rd_at(s, (int)q);
}

// Original sp_vdelay_compute index math, with the physical window substituted
// for the 10 s ring. One write, then any number of taps, then one advance.
void adapter_write(Store& s, float sample) {
    if (s.written < (uint32_t)s.virtual_maxd) s.written++;
    wr_at(s, (int)s.physical_left, sample);
}
float adapter_tap(sp_data* sp, const Store& s, float del) {
    int32_t maxd = s.virtual_maxd;
    int32_t indx = (int32_t)s.virtual_left;
    float fv1 = (float)((double)del * (-1.0 * (double)sp->sr));
    int32_t v1 = (int32_t)fv1;
    fv1 -= (float)v1;
    v1 += indx;
    if ((v1 < 0) || (fv1 < 0.0f)) {
        fv1++; v1--;
        while (v1 < 0) v1 += maxd;
    } else {
        while (v1 >= maxd) v1 -= maxd;
    }
    int32_t v2 = (v1 == (int32_t)(maxd - 1UL) ? 0L : v1 + 1L);
    if (maxd < 4) {
        float b1 = read_age(s, v1);
        float b2 = read_age(s, v2);
        return b1 + fv1 * (b2 - b1);
    }
    int32_t v0 = (v1 == 0 ? maxd - 1 : v1 - 1);
    int32_t v3 = (v2 == maxd - 1 ? 0 : v2 + 1);
    float w, x, y, z;
    z = fv1 * fv1; z--;
    z *= 0.1666666667;
    y = fv1;
    y++; w = (y *= 0.5); w--;
    x = 3.0 * z; y -= x; w -= z; x -= fv1;
    float b0 = read_age(s, v0);
    float b1 = read_age(s, v1);
    float b2 = read_age(s, v2);
    float b3 = read_age(s, v3);
    return (w * b0 + x * b1 + y * b2 + z * b3) * fv1 + b1;
}
void adapter_advance(Store& s, float newest) {
    int32_t indx = (int32_t)s.virtual_left;
    if (++indx == s.virtual_maxd) indx = 0;
    s.virtual_left = (uint32_t)indx;
    s.prev = newest;
    if (++s.physical_left == (uint32_t)s.physical) s.physical_left = 0;
}
void adapter_compute(sp_data* sp, Store& s, float del, float feedback, float in, float* out) {
    adapter_write(s, in + s.prev * feedback);
    *out = adapter_tap(sp, s, del);
    adapter_advance(s, *out);
}

void store_init(Store& s, int rate, int physical, bool i16) {
    s = Store{};
    s.use_i16 = i16;
    s.physical = physical;
    s.sr = (float)rate;
    s.maxdel = 10.f;
    s.virtual_maxd = (int)(uint32_t)(10.f * (float)rate);
    if (i16) s.i.assign((size_t)physical, 0);
    else s.f.assign((size_t)physical, 0.f);
}
void store_reset(Store& s) {
    s.virtual_left = s.physical_left = s.written = 0;
    s.clips = 0;
    if (s.use_i16) std::fill(s.i.begin(), s.i.end(), 0);
    else std::fill(s.f.begin(), s.f.end(), 0.f);
}

enum class Mode { Native, FloatAdapter, I16Adapter, Decim, Single };

struct Fixed12 {
    std::vector<int16_t> buf;
    int n = 0;
    int pos = 0;
    uint64_t clips = 0;
};
float fixed12(Fixed12& d, float in) {
    float y = (float)d.buf[(size_t)d.pos] * (1.f / 2048.f);
    int32_t k = (int32_t)(in * 2048.f);
    if (k > 32767) { k = 32767; d.clips++; }
    else if (k < -32767) { k = -32767; d.clips++; }
    d.buf[(size_t)d.pos] = (int16_t)k;
    if (++d.pos >= d.n) d.pos = 0;
    return y;
}

struct Bank {
    Mode mode = Mode::Native;
    int decim = 1;
    Store line[4];
    Store single;
    Fixed12 widen;
    float y_prev[4]{}, y_curr[4]{};
    int phase[4]{};
    uint64_t clips = 0;
};

int roundtrip_maxd(int want, int rate) {
    for (int n = want; n > 4; --n) {
        float md = (float)n / (float)rate;
        uint32_t back = (uint32_t)(md * (float)rate);
        if (back == (uint32_t)n) return n;
    }
    return 0;
}

struct Pack {
    int carve_bytes = 0;
    int carve_i16 = 0;
    int widen = kWiden48;
    int remain = 0;
    int part = 0;
    int spare = 0;
    int long_phys = 0;
    int short_phys = 0;
    int long_admitted = 0;
    int short_admitted = 0;
    float delay_time_max_48 = 0.f;
    float delay_time_max_44 = 0.f;
};

Pack make_pack() {
    Pack p;
    p.carve_bytes = (int)((sizeof(ProposedControl) + 3u) & ~3u);
    p.carve_i16 = p.carve_bytes / 2;
    p.remain = kPoolDelaySamples - p.widen - p.carve_i16;
    int body = p.remain - 16; // four lines, 4 samples of cubic headroom each
    if (body < 6) body = 0;
    p.part = body / 6;
    p.spare = p.remain - (6 * p.part + 16);
    p.long_phys = 2 * p.part + 4;
    p.short_phys = p.part + 4;
    p.long_admitted = roundtrip_maxd(2 * p.part, 48000);
    p.short_admitted = roundtrip_maxd(p.part, 48000);
    p.delay_time_max_48 = (float)p.short_admitted / 48000.f;
    int admitted_44 = roundtrip_maxd(p.part, 44100);
    p.delay_time_max_44 = (float)admitted_44 / 44100.f;
    return p;
}

const Pack& pack() { static Pack p = make_pack(); return p; }

void bank_setup(Bank& b, int rate, Mode mode, int decim) {
    b = Bank{};
    b.mode = mode;
    b.decim = decim < 1 ? 1 : decim;
    const Pack& p = pack();
    if (mode == Mode::FloatAdapter || mode == Mode::I16Adapter) {
        bool i16 = mode == Mode::I16Adapter;
        store_init(b.line[0], rate, p.long_phys, i16);
        store_init(b.line[1], rate, p.long_phys, i16);
        store_init(b.line[2], rate, p.short_phys, i16);
        store_init(b.line[3], rate, p.short_phys, i16);
    } else if (mode == Mode::Decim) {
        store_init(b.line[0], rate, p.long_phys, true);
        store_init(b.line[1], rate, p.long_phys, true);
        store_init(b.line[2], rate, p.short_phys, true);
        store_init(b.line[3], rate, p.short_phys, true);
        for (int i = 0; i < 4; ++i) {
            // Low-rate ring: the same words span decim times longer.
            b.line[i].maxdel = 10.f;
            b.line[i].virtual_maxd = (int)(uint32_t)(10.f * (float)(rate / b.decim));
            b.line[i].sr = (float)(rate / b.decim);
        }
    } else if (mode == Mode::Single) {
        int n = kPoolDelaySamples - 256;
        store_init(b.single, rate, n, true);
    }
    if (mode == Mode::I16Adapter) {
        b.widen.n = (int)std::lround(0.05 * (double)rate);
        b.widen.buf.assign((size_t)b.widen.n, 0);
    }
}

void bank_reset(Bank& b) {
    for (int i = 0; i < 4; ++i) {
        if (b.line[i].physical) store_reset(b.line[i]);
        b.y_prev[i] = b.y_curr[i] = 0.f;
        b.phase[i] = 0;
    }
    if (b.single.physical) store_reset(b.single);
}

uint64_t bank_clips(const Bank& b) {
    uint64_t n = b.single.clips + b.widen.clips;
    for (int i = 0; i < 4; ++i) n += b.line[i].clips;
    return n;
}

void delay_one(sp_data* sp, SourceFx& fx, Bank& b, int which, float in, float* out) {
    sp_vdelay* native[4] = {fx.delayL, fx.delayR, fx.delayRR, fx.delayFillIn};
    if (b.mode == Mode::Native) {
        sp_vdelay_compute(sp, native[which], &in, out);
        return;
    }
    if (b.mode == Mode::Decim) {
        Store& s = b.line[which];
        if (b.phase[which] == 0) {
            b.y_prev[which] = b.y_curr[which];
            sp_data low = *sp;
            low.sr = sp->sr / b.decim;
            adapter_compute(&low, s, native[which]->del, native[which]->feedback, in, &b.y_curr[which]);
        }
        float t = (float)b.phase[which] / (float)b.decim;
        *out = b.y_prev[which] + (b.y_curr[which] - b.y_prev[which]) * t;
        b.phase[which]++;
        if (b.phase[which] >= b.decim) b.phase[which] = 0;
        return;
    }
    adapter_compute(sp, b.line[which], native[which]->del, native[which]->feedback, in, out);
}

// Transcribed from source_fx.hpp SourceFx::frame. Identity mode must match it
// bit for bit; the delay calls are the only substitution.
void render(SourceFx& fx, Bank& b, float input, float* outL, float* outR,
            float lfo1_0_1, float lfo1_1_0, float lfo2_0_1, float lfo2_1_0) {
    sp_data* sp = fx.sp;
    float* parameters = fx.parameters.data();
    float lfo3_0_1 = .5f * (lfo1_0_1 + lfo2_0_1), lfo3_1_0 = .5f * (lfo1_1_0 + lfo2_1_0);
    float synthOut = input;
    float bitcrushSrate = parameters[bitCrushSampleRate];
    bitcrushSrate = log2f(bitcrushSrate);
    const float magicNumber = 4.f;
    if (parameters[bitcrushLFO] == 1.f) bitcrushSrate += magicNumber * lfo1_0_1;
    else if (parameters[bitcrushLFO] == 2.f) bitcrushSrate += magicNumber * lfo2_0_1;
    else if (parameters[bitcrushLFO] == 3.f) bitcrushSrate += magicNumber * lfo3_0_1;
    bitcrushSrate = exp2f(bitcrushSrate);
    bitcrushSrate = fx.clampedValue(bitCrushSampleRate, bitcrushSrate);
    float bitCrushOut = synthOut;
    fx.bitcrushIncr = fx.sampleRate() / bitcrushSrate;
    if (fx.bitcrushIncr < 1.f) fx.bitcrushIncr = 1.f;
    if (fx.bitcrushIndex <= fx.bitcrushSampleIndex) {
        bitCrushOut = fx.bitcrushValue = synthOut;
        fx.bitcrushIndex += fx.bitcrushIncr;
        fx.bitcrushIndex -= fx.bitcrushSampleIndex;
        fx.bitcrushSampleIndex = 0;
    } else bitCrushOut = fx.bitcrushValue;
    fx.bitcrushSampleIndex += 1.f;
    if (parameters[tremoloLFO] == 1.f) bitCrushOut *= lfo1_1_0;
    else if (parameters[tremoloLFO] == 2.f) bitCrushOut *= lfo2_1_0;
    else if (parameters[tremoloLFO] == 3.f) bitCrushOut *= lfo3_1_0;
    fx.panOscillator->freq = parameters[autoPanFrequency];
    fx.panOscillator->amp = parameters[autoPanAmount];
    float panValue = 0.f;
    sp_osc_compute(sp, fx.panOscillator, nullptr, &panValue);
    fx.pan->pan = panValue;
    float panL = 0.f, panR = 0.f;
    sp_pan2_compute(sp, fx.pan, &bitCrushOut, &panL, &panR);
    float phaserOutL = panL, phaserOutR = panR;
    float lPhaserMix = parameters[phaserMix];
    *fx.phaser0->Notch_width = parameters[phaserNotchWidth];
    *fx.phaser0->feedback_gain = parameters[phaserFeedback];
    *fx.phaser0->lfobpm = parameters[phaserRate];
    if (lPhaserMix != 0.f) {
        lPhaserMix = 1.f - lPhaserMix;
        sp_phaser_compute(sp, fx.phaser0, &panL, &panR, &phaserOutL, &phaserOutR);
        phaserOutL = lPhaserMix * panL + (1.f - lPhaserMix) * phaserOutL;
        phaserOutR = lPhaserMix * panR + (1.f - lPhaserMix) * phaserOutR;
    }
    float delayInputLowPassOutL = phaserOutL, delayInputLowPassOutR = phaserOutR;
    if (parameters[filterType] == 0.f) {
        const float pmin2 = log2f(1024.f);
        const float pmax2 = log2f(fx.maximum(cutoff));
        const float pval1 = parameters[cutoff];
        float pval2 = log2f(pval1);
        if (pval2 < pmin2) pval2 = pmin2;
        if (pval2 > pmax2) pval2 = pmax2;
        const float pnorm2 = (pval2 - pmin2) / (pmax2 - pmin2);
        const float mmax = parameters[delayInputCutoffTrackingRatio];
        const float mmin = 1.f;
        const float oscFilterFreqCutoffPercentage = mmin + pnorm2 * (mmax - mmin);
        float oscFilterFreqCutoff = pval1 * oscFilterFreqCutoffPercentage;
        oscFilterFreqCutoff = fx.clampedValue(cutoff, oscFilterFreqCutoff);
        fx.loPassInputDelayL->freq = oscFilterFreqCutoff;
        fx.loPassInputDelayL->res = 0.f;
        fx.loPassInputDelayR->freq = oscFilterFreqCutoff;
        fx.loPassInputDelayR->res = 0.f;
        sp_moogladder_compute(sp, fx.loPassInputDelayL, &phaserOutL, &delayInputLowPassOutL);
        sp_moogladder_compute(sp, fx.loPassInputDelayR, &phaserOutR, &delayInputLowPassOutR);
    }
    float delayOutL = 0.f, delayOutR = 0.f, delayOutRR = 0.f, delayFillInOut = 0.f;
    float delay_time = parameters[delayTime];
    float cap = (float)pack().short_admitted / (float)fx.sampleRate();
    if ((b.mode == Mode::FloatAdapter || b.mode == Mode::I16Adapter || b.mode == Mode::Decim) && delay_time > cap)
        delay_time = cap;
    if (b.mode == Mode::Decim) delay_time = parameters[delayTime]; // decimation keeps the long time inside the same words
    float long_del = delay_time * 2.f;
    float short_del = delay_time;
    if (b.mode == Mode::Decim) {
        float scale = (float)b.decim;
        float long_max = (float)pack().long_phys * scale / (float)fx.sampleRate();
        float short_max = (float)pack().short_phys * scale / (float)fx.sampleRate();
        if (long_del > long_max) long_del = long_max;
        if (short_del > short_max) short_del = short_max;
    }
    fx.delayL->del = fx.delayR->del = long_del;
    fx.delayRR->del = fx.delayFillIn->del = short_del;
    fx.delayL->feedback = fx.delayR->feedback = parameters[delayFeedback];
    fx.delayRR->feedback = fx.delayFillIn->feedback = parameters[delayFeedback];
    if (b.mode == Mode::Single) {
        // One 12-bit line. Write the mono sum once. Left reads at 2T, right at T,
        // and feedback comes from the long tap. This is the shared-bus shape, not
        // the original four-line graph.
        float in = 0.5f * (delayInputLowPassOutL + delayInputLowPassOutR);
        float T = parameters[delayTime];
        float max_t = (float)(b.single.physical / 2 - 4) / (float)fx.sampleRate();
        if (T > max_t) T = max_t;
        if (T < 0.f) T = 0.f;
        adapter_write(b.single, in + b.single.prev * parameters[delayFeedback]);
        delayOutL = adapter_tap(sp, b.single, T * 2.f);
        delayOutRR = adapter_tap(sp, b.single, T);
        adapter_advance(b.single, delayOutL);
        (void)delayOutR; (void)delayFillInOut;
    } else {
        delay_one(sp, fx, b, 0, delayInputLowPassOutL, &delayOutL);
        delay_one(sp, fx, b, 1, delayInputLowPassOutR, &delayOutR);
        delay_one(sp, fx, b, 3, delayInputLowPassOutR, &delayFillInOut);
        delay_one(sp, fx, b, 2, delayOutR, &delayOutRR);
        delayOutRR += delayFillInOut;
    }
    float mixedDelayL = 0.f, mixedDelayR = 0.f;
    fx.delayCrossfadeL->pos = parameters[delayMix] * parameters[delayOn];
    fx.delayCrossfadeR->pos = parameters[delayMix] * parameters[delayOn];
    sp_crossfade_compute(sp, fx.delayCrossfadeL, &phaserOutL, &delayOutL, &mixedDelayL);
    sp_crossfade_compute(sp, fx.delayCrossfadeR, &phaserOutR, &delayOutRR, &mixedDelayR);
    float butOutL = 0.f, butOutR = 0.f;
    fx.butterworthHipassL->freq = parameters[reverbHighPass];
    fx.butterworthHipassR->freq = parameters[reverbHighPass];
    sp_buthp_compute(sp, fx.butterworthHipassL, &mixedDelayL, &butOutL);
    sp_buthp_compute(sp, fx.butterworthHipassR, &mixedDelayR, &butOutR);
    butOutL *= 2.f; butOutR *= 2.f;
    float butCompressOutL = 0.f, butCompressOutR = 0.f;
    fx.mCompReverbIn.compute(butOutL, butOutR, butCompressOutL, butCompressOutR);
    float reverbWetL = 0.f, reverbWetR = 0.f;
    fx.reverbCostello->feedback = parameters[reverbFeedback];
    fx.reverbCostello->lpfreq = 0.5f * fx.sampleRate();
    sp_revsc_compute(sp, fx.reverbCostello, &butCompressOutL, &butCompressOutR, &reverbWetL, &reverbWetR);
    float wetReverbLimiterL = reverbWetL, wetReverbLimiterR = reverbWetR;
    fx.mCompReverbWet.compute(reverbWetL, reverbWetR, wetReverbLimiterL, wetReverbLimiterR);
    float reverbMixFactor = parameters[reverbMix] * parameters[reverbOn];
    if (parameters[reverbMixLFO] == 1.f) reverbMixFactor *= lfo1_1_0;
    else if (parameters[reverbMixLFO] == 2.f) reverbMixFactor *= lfo2_1_0;
    else if (parameters[reverbMixLFO] == 3.f) reverbMixFactor *= lfo3_1_0;
    fx.revCrossfadeL->pos = reverbMixFactor;
    fx.revCrossfadeR->pos = reverbMixFactor;
    float reverbCrossfadeOutL = 0.f, reverbCrossfadeOutR = 0.f;
    sp_crossfade_compute(sp, fx.revCrossfadeL, &mixedDelayL, &wetReverbLimiterL, &reverbCrossfadeOutL);
    sp_crossfade_compute(sp, fx.revCrossfadeR, &mixedDelayR, &wetReverbLimiterR, &reverbCrossfadeOutR);
    reverbCrossfadeOutL *= (2.f * parameters[masterVolume]);
    reverbCrossfadeOutR *= (2.f * parameters[masterVolume]);
    float compressorOutL = reverbCrossfadeOutL, compressorOutR = reverbCrossfadeOutR;
    fx.mCompMaster.compute(reverbCrossfadeOutL, reverbCrossfadeOutR, compressorOutL, compressorOutR);
    float widenOutR = 0.f;
    if (b.mode == Mode::I16Adapter)
        widenOutR = fixed12(b.widen, compressorOutR);
    else
        sp_delay_compute(sp, fx.widenDelay, &compressorOutR, &widenOutR);
    widenOutR = parameters[widen] * widenOutR + (1.f - parameters[widen]) * compressorOutR;
    *outL = compressorOutL;
    *outR = widenOutR;
}

struct Acc {
    double max_abs = 0, sum_sq = 0;
    uint64_t n = 0;
    double spec_max = 0, spec_sum_sq = 0;
    uint64_t spec_n = 0;
    void add(float a, float b) {
        double d = (double)a - (double)b;
        if (d < 0) d = -d;
        if (d > max_abs) max_abs = d;
        sum_sq += ((double)a - (double)b) * ((double)a - (double)b);
        n++;
    }
    double rms() const { return n ? std::sqrt(sum_sq / (double)n) : 0; }
    double snr_db(double sig_rms) const {
        double e = rms();
        if (e == 0) return 1e9;
        if (sig_rms <= 0) return 0;
        return 20.0 * std::log10(sig_rms / e);
    }
};

void fft_inplace(std::vector<std::complex<double>>& a) {
    int n = (int)a.size();
    for (int i = 1, j = 0; i < n; i++) {
        int bit = n >> 1;
        for (; j & bit; bit >>= 1) j ^= bit;
        j ^= bit;
        if (i < j) std::swap(a[(size_t)i], a[(size_t)j]);
    }
    for (int len = 2; len <= n; len <<= 1) {
        double ang = -2.0 * std::acos(-1.0) / (double)len;
        std::complex<double> wlen(std::cos(ang), std::sin(ang));
        for (int i = 0; i < n; i += len) {
            std::complex<double> w(1);
            for (int j = 0; j < len / 2; j++) {
                auto u = a[(size_t)(i + j)];
                auto v = a[(size_t)(i + j + len / 2)] * w;
                a[(size_t)(i + j)] = u + v;
                a[(size_t)(i + j + len / 2)] = u - v;
                w *= wlen;
            }
        }
    }
}

void spectral(const std::vector<float>& a, const std::vector<float>& b, Acc& acc) {
    const int N = 4096;
    if ((int)a.size() < N) return;
    std::vector<std::complex<double>> A(N), B(N);
    double wsum = 0;
    std::vector<double> win(N);
    for (int i = 0; i < N; i++) {
        win[(size_t)i] = 0.5 * (1.0 - std::cos(2.0 * std::acos(-1.0) * i / (N - 1)));
        wsum += win[(size_t)i] * win[(size_t)i];
    }
    for (size_t off = 0; off + (size_t)N <= a.size(); off += (size_t)N) {
        for (int i = 0; i < N; i++) {
            A[(size_t)i] = a[off + (size_t)i] * win[(size_t)i];
            B[(size_t)i] = b[off + (size_t)i] * win[(size_t)i];
        }
        fft_inplace(A); fft_inplace(B);
        int bins = N / 2;
        for (int i = 0; i < bins; i++) {
            double da = std::abs(A[(size_t)i]);
            double db = std::abs(B[(size_t)i]);
            double d = da - db;
            if (d < 0) d = -d;
            if (d > acc.spec_max) acc.spec_max = d;
            acc.spec_sum_sq += d * d;
            acc.spec_n++;
        }
    }
    (void)wsum;
}

struct Result {
    std::string name;
    Acc L, R;
    double sig_rms = 0;
    uint64_t clips = 0;
    bool bit_exact = false;
};

void finish_spectral(Result& r, const std::vector<float>& rl, const std::vector<float>& rr,
                     const std::vector<float>& al, const std::vector<float>& ar) {
    spectral(rl, al, r.L);
    spectral(rr, ar, r.R);
    double ss = 0;
    for (size_t i = 0; i < rl.size(); i++) {
        ss += (double)rl[i] * rl[i] + (double)rr[i] * rr[i];
    }
    r.sig_rms = rl.empty() ? 0 : std::sqrt(ss / (double)(rl.size() * 2));
    r.bit_exact = r.L.max_abs == 0 && r.R.max_abs == 0;
}

void json_result(FILE* fp, const Result& r) {
    auto spec_rms = [](const Acc& a) { return a.spec_n ? std::sqrt(a.spec_sum_sq / (double)a.spec_n) : 0; };
    std::fprintf(fp,
        "{\"name\":\"%s\",\"frames\":%llu,\"bit_exact\":%s,\"max_abs_l\":%.9g,\"max_abs_r\":%.9g,"
        "\"rms_l\":%.9g,\"rms_r\":%.9g,\"snr_db_l\":%.6g,\"snr_db_r\":%.6g,"
        "\"spec_mag_max_l\":%.9g,\"spec_mag_max_r\":%.9g,\"spec_mag_rms_l\":%.9g,\"spec_mag_rms_r\":%.9g,"
        "\"signal_rms\":%.9g,\"i16_saturations\":%llu}\n",
        r.name.c_str(), (unsigned long long)(r.L.n), r.bit_exact ? "true" : "false",
        r.L.max_abs, r.R.max_abs, r.L.rms(), r.R.rms(), r.L.snr_db(r.sig_rms), r.R.snr_db(r.sig_rms),
        r.L.spec_max, r.R.spec_max, spec_rms(r.L), spec_rms(r.R), r.sig_rms,
        (unsigned long long)r.clips);
}

void apply_reference_knobs(SourceFx& f) {
    f.parameters[delayTime] = .05f;
    f.parameters[delayMix] = .4f;
    f.parameters[delayOn] = 1.f;
    f.parameters[reverbMix] = .25f;
    f.parameters[reverbOn] = 1.f;
    f.parameters[phaserMix] = .3f;
    f.parameters[widen] = 1.f;
}

int g_alt_reverb_on = -1; // -1: follow the reference. 0 or 1: force the variant.
using FrameFn = void (*)(SourceFx&, int i, float& x, bool& reset, bool& restore);

void drive(SourceFx& ref, SourceFx& alt, Bank& bank, int frames, FrameFn prep, Result& result) {
    std::vector<float> rl(frames), rr(frames), al(frames), ar(frames);
    for (int i = 0; i < frames; i++) {
        float x = 0; bool reset = false, restore = false;
        prep(ref, i, x, reset, restore);
        alt.parameters = ref.parameters;
        if (g_alt_reverb_on >= 0) alt.parameters[reverbOn] = (float)g_alt_reverb_on;
        if (reset) {
            ref.source_reset_delays();
            alt.source_reset_delays();
            bank_reset(bank);
        }
        if (restore) {
            ref.source_restore_effect_controls();
            alt.source_restore_effect_controls();
        }
        float lfo[4] = {.3f, .8f, .6f, .9f};
        ref.frame(x, &rl[(size_t)i], &rr[(size_t)i], lfo[0], lfo[1], lfo[2], lfo[3]);
        render(alt, bank, x, &al[(size_t)i], &ar[(size_t)i], lfo[0], lfo[1], lfo[2], lfo[3]);
        result.L.add(rl[(size_t)i], al[(size_t)i]);
        result.R.add(rr[(size_t)i], ar[(size_t)i]);
    }
    result.clips = bank_clips(bank);
    finish_spectral(result, rl, rr, al, ar);
}

void ref_prep(SourceFx&, int i, float& x, bool&, bool&) {
    x = .15f * std::sin(2. * 3.141592653589793 * 220. * i / 48000.); // rate filled by closure via global
}

int g_rate = 48000;
void prep_reference(SourceFx& f, int i, float& x, bool&, bool&) {
    x = .15f * std::sin(2. * 3.141592653589793 * 220. * (double)i / (double)g_rate);
    (void)f;
}
void prep_lifecycle(SourceFx& f, int i, float& x, bool& reset, bool& restore) {
    if (i == 12000 || i == 36000) reset = true;
    if (i == 24000 || i == 48000) {
        *f.phaser0->MinNotch1Freq = 199;
        f.parameters[cutoff] = 1234;
        restore = true;
    }
    x = i < 30000 ? .15f * std::sin(float(i) * .031f) : 0.f;
}
void prep_bright(SourceFx&, int i, float& x, bool&, bool&) {
    uint32_t n = (uint32_t)(i + 1) * 1664525u + 1013904223u;
    x = ((int32_t)n) * (0.15f / 2147483648.f);
}
void prep_long(SourceFx&, int i, float& x, bool&, bool&) {
    x = .15f * std::sin(2. * 3.141592653589793 * 220. * (double)i / 48000.);
}

bool vdelay_copy_matches() {
    sp_data* sp; sp_create(&sp); sp->sr = 48000;
    sp_vdelay* o; sp_vdelay_create(&o); sp_vdelay_init(sp, o, 10.f);
    Store s; store_init(s, 48000, pack().long_phys, false);
    uint32_t rnd = 1;
    int frames = 20000;
    bool ok = true;
    int bad_at = -1;
    for (int i = 0; i < frames; i++) {
        rnd = rnd * 1664525u + 1013904223u;
        float in = ((int32_t)rnd) * (0.2f / 2147483648.f);
        float del = (i % 8000) < 4000 ? 0.05f : 0.2f;
        if (del * 2.f > (float)pack().long_admitted / 48000.f) del = 0.05f;
        // long line admits ~0.53 s; 0.2 s is inside it.
        float fb = (i / 1000) % 2 ? 0.f : 0.4f;
        o->del = del; o->feedback = fb;
        float a = 0, b = 0;
        sp_vdelay_compute(sp, o, &in, &a);
        adapter_compute(sp, s, del, fb, in, &b);
        if (std::memcmp(&a, &b, 4) != 0) { ok = false; bad_at = i; break; }
    }
    std::printf("{\"check\":\"float_adapter_vs_sp_vdelay\",\"bit_exact\":%s,\"frames\":%d,\"physical\":%d,\"fail_frame\":%d}\n",
                ok ? "true" : "false", ok ? frames : bad_at, pack().long_phys, bad_at);
    sp_vdelay_destroy(&o); sp_destroy(&sp);
    return ok;
}

void pan_table_vs_fm_sinf(FILE* out) {
    sp_data* sp; sp_create(&sp); sp->sr = 48000;
    sp_ftbl* sine; sp_ftbl_create(sp, &sine, 4096); sp_gen_sine(sp, sine);
    for (float freq : {0.25f, 10.f}) {
        sp_osc* o; sp_osc_create(&o); sp_osc_init(sp, o, sine, 0.f);
        o->freq = freq; o->amp = 1.f;
        double max_abs = 0, sum_sq = 0; int n = 48000;
        float phase = 0.f;
        for (int i = 0; i < n; i++) {
            float y = 0; sp_osc_compute(sp, o, nullptr, &y);
            float z = fm_sinf(phase * FM_TWO_PI);
            phase += freq / 48000.f;
            if (phase >= 1.f) phase -= 1.f;
            double d = (double)y - (double)z;
            if (d < 0) d = -d;
            if (d > max_abs) max_abs = d;
            sum_sq += d * d;
        }
        std::fprintf(out, "{\"check\":\"pan_table_vs_fm_sinf\",\"hz\":%.4g,\"frames\":%d,\"max_abs\":%.9g,\"rms\":%.9g}\n",
                     freq, n, max_abs, std::sqrt(sum_sq / n));
        sp_osc_destroy(&o);
    }
    sp_ftbl_destroy(&sine); sp_destroy(&sp);
}

void inventory(FILE* out) {
    SourceFx f(48000);
    auto line = [&](const char* name, size_t payload, int max_samples, const char* note) {
        std::fprintf(out, "{\"stage\":\"%s\",\"payload_bytes\":%zu,\"max_samples\":%d,\"note\":\"%s\"}\n",
                     name, payload, max_samples, note);
    };
    line("bitcrush_hold", 16, 0, "four floats; hold length is sr/rate, not a buffer");
    line("tremolo", 0, 0, "multiplies by the supplied LFO; no state");
    line("autopan_sine_table", (size_t)(4096 + 1) * 4, 4097, "sp_ftbl_create 4096 plus the guard sample");
    line("autopan_osc_pan_host_objects", sizeof(*f.panOscillator) + sizeof(*f.pan), 0, "host sizeof, pointers are 8");
    line("phaser_faust", sizeof(PhaserFaust), 0, "copied faust struct; no sample delay line");
    line("phaser_wrapper_host", sizeof(*f.phaser0), 0, "host sizeof of sp_phaser");
    line("moog_delay_input_L", sizeof(*f.loPassInputDelayL), 6, "delay[6] ladder state, both channels follow");
    line("moog_delay_input_R", sizeof(*f.loPassInputDelayR), 6, "same");
    line("vdelay_L_allocated", f.delayL->buf.size, (int)(f.delayL->buf.size / 4), "init maxdel 10s");
    line("vdelay_R_allocated", f.delayR->buf.size, (int)(f.delayR->buf.size / 4), "init maxdel 10s");
    line("vdelay_RR_allocated", f.delayRR->buf.size, (int)(f.delayRR->buf.size / 4), "init maxdel 10s");
    line("vdelay_fill_allocated", f.delayFillIn->buf.size, (int)(f.delayFillIn->buf.size / 4), "init maxdel 10s");
    size_t reachable = 0;
    for (float md : {5.f, 5.f, 2.5f, 2.5f}) {
        sp_vdelay* v; sp_vdelay_create(&v); sp_vdelay_init(f.sp, v, md);
        reachable += v->buf.size;
        sp_vdelay_destroy(&v);
    }
    std::fprintf(out, "{\"stage\":\"four_reachable_payloads\",\"payload_bytes\":%zu,\"auxdata_host\":%zu,\"plus_four_aux\":%zu,\"manifest_bytes\":2880080}\n",
                 reachable, sizeof(sp_auxdata), reachable + 4 * sizeof(sp_auxdata));
    line("delay_crossfade_pair_host", sizeof(*f.delayCrossfadeL) * 2, 0, "pos only");
    line("reverb_hipass_pair", sizeof(*f.butterworthHipassL) * 2, 0, "a[8] filter state, no delay line");
    line("revsc_lines", f.reverbCostello->aux.size, 0, "eight delay lines, iPitchMod 1");
    int rev_max = 0;
    for (int i = 0; i < 8; i++) {
        int n = f.reverbCostello->delayLines[i].bufferSize;
        if (n > rev_max) rev_max = n;
    }
    std::fprintf(out, "{\"stage\":\"revsc_longest_line\",\"samples\":%d,\"seconds\":%.8f}\n", rev_max, (double)rev_max / 48000.0);
    line("reverb_crossfade_pair_host", sizeof(*f.revCrossfadeL) * 2, 0, "pos only");
    line("widen_delay", f.widenDelay->buf.size, (int)f.widenDelay->bufsize, "fixed 0.05s, feedback forced 0");
    std::fprintf(out, "{\"stage\":\"proposed_control\",\"bytes\":%zu,\"aligned4\":%d}\n", sizeof(ProposedControl), pack().carve_bytes);
    std::fprintf(out, "{\"stage\":\"phaser_faust_sizeof\",\"bytes\":%zu}\n", sizeof(PhaserFaust));
    std::fprintf(out, "{\"stage\":\"compressor_faust_sizeof\",\"bytes\":%zu,\"times6\":%zu}\n", sizeof(CompFaust), sizeof(CompFaust) * 6);
    std::fprintf(out, "{\"stage\":\"moog_sizeof\",\"bytes\":%zu}\n", sizeof(sp_moogladder));
    std::fprintf(out, "{\"stage\":\"buthp_sizeof\",\"bytes\":%zu}\n", sizeof(sp_buthp));
    const char* names[] = {
        "bitCrushSampleRate","bitcrushLFO","tremoloLFO","autoPanAmount","autoPanFrequency",
        "phaserMix","phaserRate","phaserFeedback","phaserNotchWidth","filterType","cutoff",
        "delayInputCutoffTrackingRatio","delayInputResonance","delayOn","delayFeedback","delayTime","delayMix",
        "reverbOn","reverbFeedback","reverbHighPass","reverbMix","reverbMixLFO",
        "compressorReverbInputRatio","compressorReverbInputThreshold","compressorReverbInputAttack","compressorReverbInputRelease","compressorReverbInputMakeupGain",
        "compressorReverbWetRatio","compressorReverbWetThreshold","compressorReverbWetAttack","compressorReverbWetRelease","compressorReverbWetMakeupGain",
        "compressorMasterRatio","compressorMasterThreshold","compressorMasterAttack","compressorMasterRelease","compressorMasterMakeupGain",
        "masterVolume","widen"
    };
    S1Parameter ids[] = {
        bitCrushSampleRate,bitcrushLFO,tremoloLFO,autoPanAmount,autoPanFrequency,
        phaserMix,phaserRate,phaserFeedback,phaserNotchWidth,filterType,cutoff,
        delayInputCutoffTrackingRatio,delayInputResonance,delayOn,delayFeedback,delayTime,delayMix,
        reverbOn,reverbFeedback,reverbHighPass,reverbMix,reverbMixLFO,
        compressorReverbInputRatio,compressorReverbInputThreshold,compressorReverbInputAttack,compressorReverbInputRelease,compressorReverbInputMakeupGain,
        compressorReverbWetRatio,compressorReverbWetThreshold,compressorReverbWetAttack,compressorReverbWetRelease,compressorReverbWetMakeupGain,
        compressorMasterRatio,compressorMasterThreshold,compressorMasterAttack,compressorMasterRelease,compressorMasterMakeupGain,
        masterVolume,widen
    };
    for (unsigned i = 0; i < sizeof(ids) / sizeof(ids[0]); i++) {
        std::fprintf(out, "{\"param\":\"%s\",\"min\":%.9g,\"default\":%.9g,\"max\":%.9g}\n",
                     names[i], f.lows[ids[i]], f.parameters[ids[i]], f.highs[ids[i]]);
    }
    const Pack& p = pack();
    std::fprintf(out, "{\"pack\":{\"pool_i16\":%d,\"pool_bytes\":%d,\"carve_bytes\":%d,\"widen_i16\":%d,\"long_phys\":%d,\"short_phys\":%d,\"long_admitted\":%d,\"short_admitted\":%d,\"spare_i16\":%d,\"delay_time_max_48\":%.9g,\"delay_time_max_44\":%.9g,\"default_delay_time\":%.9g}}\n",
                 kPoolDelaySamples, kPoolDelaySamples * 2, p.carve_bytes, p.widen, p.long_phys, p.short_phys,
                 p.long_admitted, p.short_admitted, p.spare, p.delay_time_max_48, p.delay_time_max_44, f.parameters[delayTime]);
}

Result run_mode(const char* name, int rate, int frames, Mode mode, int decim, FrameFn prep, void (*tweak)(SourceFx&, SourceFx&)) {
    g_rate = rate;
    SourceFx ref(rate), alt(rate);
    tweak(ref, alt);
    Bank bank; bank_setup(bank, rate, mode, decim);
    Result r; r.name = name;
    std::fprintf(stderr, "run %s\n", name);
    drive(ref, alt, bank, frames, prep, r);
    return r;
}

void tweak_reference(SourceFx& a, SourceFx& b) { apply_reference_knobs(a); apply_reference_knobs(b); }
void tweak_lifecycle(SourceFx& a, SourceFx& b) {
    for (SourceFx* f : {&a, &b}) {
        f->parameters[delayTime] = .005f;
        f->parameters[delayOn] = 1.f;
        f->parameters[delayMix] = .7f;
        f->parameters[reverbOn] = 1.f;
        f->parameters[reverbMix] = .5f;
        f->parameters[phaserMix] = .3f;
        f->parameters[widen] = 1.f;
    }
}
void tweak_exposed(SourceFx& a, SourceFx& b) {
    for (SourceFx* f : {&a, &b}) {
        f->parameters[delayTime] = .25f;
        f->parameters[delayOn] = 1.f;
        f->parameters[delayMix] = 1.f;
        f->parameters[delayFeedback] = .9f;
        f->parameters[reverbOn] = 0.f;
        f->parameters[reverbMix] = 0.f;
        f->parameters[phaserMix] = 0.f;
        f->parameters[widen] = 0.f;
        f->parameters[autoPanAmount] = 0.f;
        f->parameters[filterType] = 1.f; // skip the tracking ladder; delay sees the dry-ish path
    }
}
void tweak_long(SourceFx& a, SourceFx& b) {
    apply_reference_knobs(a); apply_reference_knobs(b);
    a.parameters[delayTime] = 2.5f;
    b.parameters[delayTime] = 2.5f;
    a.parameters[delayFeedback] = .9f;
    b.parameters[delayFeedback] = .9f;
}

} // namespace

int main(int argc, char** argv) {
    const char* path = argc > 1 ? argv[1] : "MEASURED.json";
    FILE* out = std::fopen(path, "w");
    if (!out) { std::perror(path); return 1; }
    std::fprintf(out, "{\"record\":\"inventory\"}\n");
    inventory(out);
    std::fprintf(out, "{\"record\":\"pan\"}\n");
    pan_table_vs_fm_sinf(out);
    if (!vdelay_copy_matches()) {
        std::fprintf(out, "{\"record\":\"FAIL\",\"check\":\"float_adapter\"}\n");
        std::fclose(out);
        return 1;
    }
    struct Job { const char* name; int rate; int frames; Mode mode; int decim; FrameFn prep; void (*tweak)(SourceFx&, SourceFx&); };
    Job jobs[] = {
        {"identity_reference_48k", 48000, 48000, Mode::Native, 1, prep_reference, tweak_reference},
        {"identity_reference_44k", 44100, 44100, Mode::Native, 1, prep_reference, tweak_reference},
        {"identity_lifecycle_48k", 48000, 60000, Mode::Native, 1, prep_lifecycle, tweak_lifecycle},
        {"float_adapter_reference_48k", 48000, 48000, Mode::FloatAdapter, 1, prep_reference, tweak_reference},
        {"i16_adapter_reference_48k", 48000, 48000, Mode::I16Adapter, 1, prep_reference, tweak_reference},
        {"i16_adapter_reference_44k", 44100, 44100, Mode::I16Adapter, 1, prep_reference, tweak_reference},
        {"i16_adapter_lifecycle_48k", 48000, 60000, Mode::I16Adapter, 1, prep_lifecycle, tweak_lifecycle},
        {"i16_plus_reverb_off_reference_48k", 48000, 48000, Mode::I16Adapter, 1, prep_reference, tweak_reference},
        {"decim4_reference_48k", 48000, 48000, Mode::Decim, 4, prep_reference, tweak_reference},
        {"decim10_reference_48k", 48000, 48000, Mode::Decim, 10, prep_reference, tweak_reference},
        {"single_reference_48k", 48000, 48000, Mode::Single, 1, prep_reference, tweak_reference},
        {"i16_exposed_48k", 48000, 48000, Mode::I16Adapter, 1, prep_bright, tweak_exposed},
        {"decim4_exposed_48k", 48000, 48000, Mode::Decim, 4, prep_bright, tweak_exposed},
        {"decim10_exposed_48k", 48000, 48000, Mode::Decim, 10, prep_bright, tweak_exposed},
        {"float_adapter_long_2s5", 48000, 288000, Mode::FloatAdapter, 1, prep_long, tweak_long},
        {"decim10_long_2s5", 48000, 288000, Mode::Decim, 10, prep_long, tweak_long},
        {"single_long_2s5", 48000, 288000, Mode::Single, 1, prep_long, tweak_long},
    };
    int rc = 0;
    for (Job& j : jobs) {
        g_alt_reverb_on = std::strstr(j.name, "reverb_off") ? 0 : -1;
        Result r = run_mode(j.name, j.rate, j.frames, j.mode, j.decim, j.prep, j.tweak);
        json_result(out, r);
        std::fflush(out);
        if (std::strncmp(j.name, "identity_", 9) == 0 && !r.bit_exact) rc = 1;
        if (std::strcmp(j.name, "float_adapter_reference_48k") == 0 && !r.bit_exact) rc = 1;
    }
    // Reverb drop uses the original frame on both sides, with reverbOn cleared on the variant.
    {
        std::fprintf(stderr, "run reverb_off_reference_48k\n");
        SourceFx a(48000), b(48000);
        apply_reference_knobs(a); apply_reference_knobs(b);
        b.parameters[reverbOn] = 0.f;
        Result r; r.name = "reverb_off_reference_48k";
        std::vector<float> rl(48000), rr(48000), al(48000), ar(48000);
        for (int i = 0; i < 48000; i++) {
            float x = .15f * std::sin(2. * 3.141592653589793 * 220. * i / 48000.);
            a.frame(x, &rl[(size_t)i], &rr[(size_t)i], .3f, .8f, .6f, .9f);
            b.frame(x, &al[(size_t)i], &ar[(size_t)i], .3f, .8f, .6f, .9f);
            r.L.add(rl[(size_t)i], al[(size_t)i]);
            r.R.add(rr[(size_t)i], ar[(size_t)i]);
        }
        finish_spectral(r, rl, rr, al, ar);
        json_result(out, r);
    }
    std::fprintf(out, "{\"record\":\"done\",\"exit\":%d}\n", rc);
    std::fclose(out);
    return rc;
}
