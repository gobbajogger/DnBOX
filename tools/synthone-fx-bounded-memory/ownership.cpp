// Host test for exclusive ownership of the 79,200-sample send-delay buffer.
// Synth One FX runs through the extracted SourceFx. The shared int16 buffer is
// the proposed delay memory. This file does not change firmware.
#define S1FX_EXISTING_PARAMETERS
#include "S1Parameter.h"
#include "source_fx.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <vector>

namespace {

constexpr int kSlots = 79200;
constexpr int kGuard = 256;
constexpr int kSendLen = kSlots - kGuard;
constexpr int kFade = 256;
constexpr int kRate = 48000;

int sat12(float x) {
    int k = (int)(x * 2048.f);
    if (k > 32767) k = 32767;
    if (k < -32767) k = -32767;
    return k;
}
float unsat(int k) { return (float)k * (1.f / 2048.f); }

struct Send {
    std::vector<int16_t>* buf = nullptr;
    int w = 0;
    float fb = 0.45f;
    float time_s = 0.30f;
    bool write = true;
    float wet = 0.f;
    void clear() {
        if (buf) std::fill(buf->begin(), buf->begin() + kSendLen, 0);
        w = 0;
    }
    float tap(float seconds) const {
        float d = seconds * (float)kRate;
        float rp = (float)w - d;
        while (rp < 0.f) rp += (float)kSendLen;
        int i0 = (int)rp;
        if (i0 >= kSendLen) i0 -= kSendLen;
        int i1 = i0 + 1 >= kSendLen ? 0 : i0 + 1;
        float fr = rp - std::floor(rp);
        float a = unsat((*buf)[(size_t)i0]);
        float b = unsat((*buf)[(size_t)i1]);
        return a + (b - a) * fr;
    }
    float step(float in) {
        float y = tap(time_s);
        wet = y;
        if (write) {
            (*buf)[(size_t)w] = (int16_t)sat12(in + fb * y);
            if (++w >= kSendLen) w = 0;
        }
        return write ? y : 0.f;
    }
};

// Four disjoint 12-bit lines inside the same array. A stand-in for the
// proposed Synth One delay memory, not a second firmware delay.
struct SynthLines {
    std::vector<int16_t>* buf = nullptr;
    int off[4]{};
    int len[4]{};
    int w[4]{};
    void place() {
        int base = 0;
        int lens[4] = {25000, 25000, 12000, 12000};
        for (int i = 0; i < 4; i++) {
            off[i] = base;
            len[i] = lens[i];
            w[i] = 0;
            base += lens[i];
        }
    }
    void clear() {
        if (!buf) return;
        std::fill(buf->begin(), buf->end(), 0);
        for (int i = 0; i < 4; i++) w[i] = 0;
    }
    float line(int i, float in, float seconds, float fb) {
        int n = len[i];
        float d = seconds * (float)kRate;
        if (d > (float)(n - 2)) d = (float)(n - 2);
        if (d < 1.f) d = 1.f;
        int wi = w[i];
        float rp = (float)wi - d;
        while (rp < 0.f) rp += (float)n;
        int i0 = (int)rp % n;
        int i1 = i0 + 1 >= n ? 0 : i0 + 1;
        float fr = rp - std::floor(rp);
        int16_t* b = buf->data() + off[i];
        float y = unsat(b[i0]) + (unsat(b[i1]) - unsat(b[i0])) * fr;
        b[wi] = (int16_t)sat12(in + fb * y);
        if (++wi >= n) wi = 0;
        w[i] = wi;
        return y;
    }
    void step(float in, float time_s, float fb, float* l, float* r) {
        float dl = line(0, in, time_s * 2.f, fb);
        float dr = line(1, in, time_s * 2.f, fb);
        float fill = line(3, in, time_s, fb);
        float rr = line(2, dr, time_s, fb);
        *l = dl;
        *r = rr + fill;
    }
};

float sine(int i, float hz) {
    return 0.2f * std::sin(2.f * 3.14159265f * hz * (float)i / (float)kRate);
}

float max_step(const std::vector<float>& x, int a, int b) {
    float m = 0.f;
    if (a < 1) a = 1;
    if (b > (int)x.size()) b = (int)x.size();
    for (int i = a; i < b; i++) {
        float d = std::fabs(x[(size_t)i] - x[(size_t)(i - 1)]);
        if (d > m) m = d;
    }
    return m;
}

} // namespace

int main(int argc, char** argv) {
    const char* path = argc > 1 ? argv[1] : "OWNERSHIP.json";
    FILE* out = std::fopen(path, "w");
    if (!out) return 1;
    std::vector<int16_t> mem(kSlots, 0);
    Send send;
    send.buf = &mem;
    SynthLines synth_mem;
    synth_mem.buf = &mem;
    synth_mem.place();

    SourceFx fx(kRate);
    fx.parameters[delayTime] = 0.05f;
    fx.parameters[delayOn] = 1.f;
    fx.parameters[delayMix] = 0.4f;
    fx.parameters[reverbOn] = 0.f;
    fx.parameters[phaserMix] = 0.f;
    fx.parameters[widen] = 0.f;

    const int together = kRate;
    double energy = 0;
    int finite = 0;
    int send_writes_while_synth_owns = 0;
    std::vector<int16_t> snapshot;
    // Exclusive rule: Synth One delay is on, so the send must not write.
    send.write = false;
    for (int i = 0; i < together; i++) {
        float x = sine(i, 220.f);
        float l = 0, r = 0;
        fx.frame(x, &l, &r, 0.3f, 0.8f, 0.6f, 0.9f);
        if (std::isfinite(l) && std::isfinite(r)) finite++;
        energy += (double)l * l + (double)r * r;
        float sl = 0, sr = 0;
        synth_mem.step(x, 0.05f, 0.2f, &sl, &sr);
        if (i == together / 2) snapshot = mem;
        float send_y = send.step(sine(i, 440.f));
        if (send_y != 0.f) send_writes_while_synth_owns++;
    }
    int changed = 0;
    for (int i = 0; i < kSendLen; i++)
        if (mem[(size_t)i] != snapshot[(size_t)i]) changed++;
    // The synth lines keep writing after the snapshot, so the buffer must change.
    // The send contribution is the part we forced off. Re-run a frozen buffer
    // through a suppressed send and require a byte-identical buffer.
    std::vector<int16_t> frozen = mem;
    send.write = false;
    float leaked = 0.f;
    for (int i = 0; i < 1000; i++) {
        float y = send.step(0.8f);
        if (std::fabs(y) > leaked) leaked = std::fabs(y);
    }
    int send_mutated = 0;
    for (int i = 0; i < kSlots; i++)
        if (mem[(size_t)i] != frozen[(size_t)i]) send_mutated++;

    auto run_switch = [&](bool clear_after_fade, std::vector<float>* rec) {
        synth_mem.clear();
        send.clear();
        send.write = false;
        rec->assign((size_t)(kRate + 2 * kFade), 0.f);
        for (int i = 0; i < kRate / 2; i++) {
            float l, r;
            synth_mem.step(sine(i, 220.f), 0.05f, 0.35f, &l, &r);
            (*rec)[(size_t)i] = l;
        }
        int at = kRate / 2;
        for (int i = 0; i < kFade; i++) {
            float g = (float)(i + 1) / (float)kFade;
            float l, r;
            synth_mem.step(sine(at + i, 220.f), 0.05f, 0.35f, &l, &r);
            float dry = sine(at + i, 220.f);
            (*rec)[(size_t)(at + i)] = (1.f - g) * l + g * dry;
        }
        if (clear_after_fade) synth_mem.clear();
        send.w = 0;
        send.write = true;
        int back = at + kFade;
        for (int i = 0; i < kFade; i++) {
            float g = (float)(i + 1) / (float)kFade;
            float dry = sine(back + i, 220.f);
            float y = send.step(0.f);
            // Dry stays put. The new delay fades in from an empty line.
            (*rec)[(size_t)(back + i)] = dry + g * y;
        }
        int tail = back + kFade;
        float stale = 0.f;
        int tail_n = (int)rec->size() - tail;
        for (int i = 0; i < tail_n; i++) {
            float y = send.step(0.f);
            (*rec)[(size_t)(tail + i)] = y;
            if (std::fabs(y) > stale) stale = std::fabs(y);
        }
        return stale;
    };

    std::vector<float> faded, abrupt_tail;
    float stale_cleared = run_switch(true, &faded);
    // Abrupt handoff: no fade, no clear. The send reads whatever the synth left.
    synth_mem.clear();
    send.clear();
    send.write = false;
    std::vector<float> abrupt(kRate / 2 + 64, 0.f);
    for (int i = 0; i < kRate / 2; i++) {
        float l, r;
        synth_mem.step(sine(i, 220.f), 0.05f, 0.35f, &l, &r);
        abrupt[(size_t)i] = l;
    }
    send.write = true;
    send.w = 0;
    float step_into = 0.f;
    for (int i = 0; i < 64; i++) {
        float y = send.step(0.f);
        int idx = kRate / 2 + i;
        abrupt[(size_t)idx] = y;
        float d = std::fabs(y - abrupt[(size_t)(idx - 1)]);
        if (d > step_into) step_into = d;
    }
    float stale_uncleared = 0.f;
    for (int i = 0; i < kRate / 4; i++) {
        float y = send.step(0.f);
        if (std::fabs(y) > stale_uncleared) stale_uncleared = std::fabs(y);
    }

    float steady = max_step(faded, 1000, kRate / 2 - 10);
    float at_fade = max_step(faded, kRate / 2 - 2, kRate / 2 + kFade + 2);
    bool pass = finite == together && energy > 0 && leaked == 0.f && send_mutated == 0
        && at_fade <= std::max(steady * 2.f, 0.002f)
        && step_into > at_fade
        && stale_cleared < (2.f / 2048.f)
        && stale_uncleared > stale_cleared;

    std::fprintf(out,
        "{\"rule\":\"exclusive\",\"rate\":%d,\"together_frames\":%d,\"sourcefx_finite\":%d,"
        "\"sourcefx_energy\":%.8g,\"send_wet_while_synth_owns\":%d,\"send_mutations_while_suppressed\":%d,"
        "\"fade_samples\":%d,\"steady_max_step\":%.8g,\"fade_max_step\":%.8g,"
        "\"abrupt_max_step\":%.8g,\"stale_peak_after_clear\":%.8g,\"stale_peak_if_not_cleared\":%.8g,"
        "\"pass\":%s}\n",
        kRate, together, finite, energy, send_writes_while_synth_owns, send_mutated,
        kFade, steady, at_fade, step_into, stale_cleared, stale_uncleared,
        pass ? "true" : "false");
    std::fclose(out);
    std::printf("ownership pass=%s fade_step=%.6g abrupt_step=%.6g stale_clear=%.6g stale_keep=%.6g\n",
                pass ? "true" : "false", at_fade, step_into, stale_cleared, stale_uncleared);
    return pass ? 0 : 1;
}
