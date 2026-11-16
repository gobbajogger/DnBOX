// Host-only hybrid delay-rate designs inside the 158,400-byte send buffer.
// Compares delay-section output with the original four sp_vdelay lines.
// Does not change firmware.
#include <algorithm>
#include <cmath>
#include <complex>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>
extern "C" {
#include "soundpipe.h"
}

namespace {

constexpr int kRate = 48000;
constexpr int kSlots = 79200;
constexpr float kCap = 0.2643541667f;

struct Biquad {
    float b0 = 1, b1 = 0, b2 = 0, a1 = 0, a2 = 0, z1 = 0, z2 = 0;
    void reset() { z1 = z2 = 0; }
    void design(float sr, float fc, float q) {
        float w = 2.f * 3.14159265f * fc / sr;
        float cw = std::cos(w), sw = std::sin(w);
        float alpha = sw / (2.f * q);
        float bb0 = (1.f - cw) * 0.5f;
        float bb1 = 1.f - cw;
        float bb2 = bb0;
        float a0 = 1.f + alpha;
        b0 = bb0 / a0; b1 = bb1 / a0; b2 = bb2 / a0;
        a1 = (-2.f * cw) / a0; a2 = (1.f - alpha) / a0;
    }
    float tick(float x) {
        float y = b0 * x + z1;
        z1 = b1 * x - a1 * y + z2;
        z2 = b2 * x - a2 * y;
        if (std::fabs(z1) < 1e-20f) z1 = 0;
        if (std::fabs(z2) < 1e-20f) z2 = 0;
        return y;
    }
};

struct Lowpass {
    Biquad a, b;
    void design(float sr, float fc) {
        a.design(sr, fc, 0.54119610f);
        b.design(sr, fc, 1.30656296f);
    }
    void reset() { a.reset(); b.reset(); }
    float tick(float x) { return b.tick(a.tick(x)); }
};

int sat12(float x) {
    int k = (int)(x * 2048.f);
    if (k > 32767) k = 32767;
    if (k < -32767) k = -32767;
    return k;
}

uint8_t mulaw(float x) {
    const float mu = 255.f, scale = 16.f;
    float ax = std::fabs(x);
    if (ax > scale) ax = scale;
    float y = std::log1p(mu * ax / scale) / std::log1p(mu);
    int m = (int)(y * 127.f + 0.5f);
    if (m > 127) m = 127;
    if (m < 0) m = 0;
    return (uint8_t)(m | (x < 0.f ? 0x80 : 0));
}
float unmulaw(uint8_t c) {
    const float mu = 255.f, scale = 16.f;
    int m = c & 0x7f;
    float mag = scale * (std::pow(1.f + mu, (float)m / 127.f) - 1.f) / mu;
    return (c & 0x80) ? -mag : mag;
}
float un12(int16_t s) { return (float)s * (1.f / 2048.f); }

int choose_d(int admitted, float seconds) {
    double need = (double)seconds * (double)kRate;
    if (need <= (double)admitted) return 1;
    if (need <= (double)admitted * 2.0) return 2;
    if (need <= (double)admitted * 4.0) return 4;
    return 10;
}

struct Line {
    int admitted = 0;
    int stored = 0;
    int d = 1;
    bool mu = false;
    std::vector<int16_t> i16;
    std::vector<uint8_t> u8;
    int w = 0;
    int phase = 0;
    float last_y = 0;
    Lowpass aa, img;
    void setup(int samples, bool use_mu) {
        mu = use_mu;
        stored = samples;
        admitted = std::max(8, samples - 4);
        if (mu) u8.assign((size_t)stored, 0);
        else i16.assign((size_t)stored, 0);
        w = 0;
        phase = 0;
        last_y = 0;
    }
    void clear() {
        std::fill(i16.begin(), i16.end(), 0);
        std::fill(u8.begin(), u8.end(), 0);
        w = 0;
        phase = 0;
        last_y = 0;
        aa.reset();
        img.reset();
    }
    void set_rate(int decim) {
        d = decim < 1 ? 1 : decim;
        float fc = 0.45f * (float)kRate / (float)d;
        if (fc > 0.45f * (float)kRate) fc = 0.45f * (float)kRate;
        aa.design((float)kRate, fc);
        img.design((float)kRate, fc);
        aa.reset();
        img.reset();
        phase = 0;
    }
    float at(int i) const {
        i %= stored;
        if (i < 0) i += stored;
        return mu ? unmulaw(u8[(size_t)i]) : un12(i16[(size_t)i]);
    }
    void put(int i, float x) {
        i %= stored;
        if (i < 0) i += stored;
        if (mu) u8[(size_t)i] = mulaw(x);
        else i16[(size_t)i] = (int16_t)sat12(x);
    }
    float cubic(float rp) const {
        while (rp < 0.f) rp += (float)stored;
        while (rp >= (float)stored) rp -= (float)stored;
        int i1 = (int)rp;
        float f = rp - (float)i1;
        int i0 = i1 == 0 ? stored - 1 : i1 - 1;
        int i2 = i1 + 1 >= stored ? 0 : i1 + 1;
        int i3 = i2 + 1 >= stored ? 0 : i2 + 1;
        float y0 = at(i0), y1 = at(i1), y2 = at(i2), y3 = at(i3);
        float c1 = 0.5f * (y2 - y0);
        float c2 = y0 - 2.5f * y1 + 2.f * y2 - 0.5f * y3;
        float c3 = 0.5f * (y3 - y0) + 1.5f * (y1 - y2);
        return ((c3 * f + c2) * f + c1) * f + y1;
    }
    // seconds behind the latest stored sample, gliding between writes.
    // w is the next write. The latest stored sample is w-1.
    float read(float seconds, int since_write) const {
        float back = seconds * (float)kRate / (float)d + (float)since_write / (float)d;
        if (back > (float)(admitted - 2)) back = (float)(admitted - 2);
        if (back < 1.f) back = 1.f;
        return cubic((float)w - 1.f - back);
    }
    float step(float in, float seconds, float fb) {
        float filtered = (d == 1) ? in : aa.tick(in);
        if (phase == 0) {
            float echo = read(seconds, 0);
            put(w, filtered + fb * echo);
            last_y = echo;
            if (++w >= stored) w = 0;
        }
        float y = read(seconds, phase);
        if (++phase >= d) phase = 0;
        return (d == 1) ? y : img.tick(y);
    }
};

struct Graph {
    Line line[4];
    bool mono = false;
    Line one;
    void setup_four(int long_n, int short_n, bool mu) {
        mono = false;
        line[0].setup(long_n, mu);
        line[1].setup(long_n, mu);
        line[2].setup(short_n, mu);
        line[3].setup(short_n, mu);
    }
    void setup_mono(int n, bool mu) {
        mono = true;
        one.setup(n, mu);
    }
    void clear() {
        for (auto& L : line) L.clear();
        one.clear();
    }
    void tune_times(float t0, float t1, float t2, float t3, bool mu_long) {
        float need[4] = {t0, t1, t2, t3};
        for (int i = 0; i < 4; i++) {
            int dec = choose_d(line[i].admitted, need[i]);
            if (mu_long && dec > 1 && !line[i].mu) {
                line[i].setup(line[i].stored * 2, true);
                dec = choose_d(line[i].admitted, need[i]);
            }
            line[i].set_rate(dec);
        }
    }
    void tune(float delay_time, bool mu_long) {
        if (mono) {
            int d = choose_d(one.admitted, delay_time * 2.f);
            if (mu_long && d > 1) {
                int grown = one.stored * (one.mu ? 1 : 2);
                if (!one.mu) one.setup(grown, true);
                d = choose_d(one.admitted, delay_time * 2.f);
            }
            one.set_rate(d);
            return;
        }
        float need[4] = {delay_time * 2.f, delay_time * 2.f, delay_time, delay_time};
        for (int i = 0; i < 4; i++) {
            int d = choose_d(line[i].admitted, need[i]);
            if (mu_long && d > 1) {
                if (!line[i].mu) line[i].setup(line[i].stored * 2, true);
                d = choose_d(line[i].admitted, need[i]);
            }
            line[i].set_rate(d);
        }
    }
    void process(float in, float delay_time, float fb, float* l, float* r) {
        if (mono) {
            float y2 = 0, y1 = 0;
            // One write at the long tap, then a second read at the short tap.
            float filtered = (one.d == 1) ? in : one.aa.tick(in);
            int since = one.phase;
            if (one.phase == 0) {
                float y = one.read(delay_time * 2.f, 0);
                one.put(one.w, filtered + fb * y);
                if (++one.w >= one.stored) one.w = 0;
            }
            y2 = one.read(delay_time * 2.f, since);
            y1 = one.read(delay_time, since);
            if (++one.phase >= one.d) one.phase = 0;
            if (one.d > 1) y2 = one.img.tick(y2);
            *l = y2;
            *r = y1;
            return;
        }
        float dl = line[0].step(in, delay_time * 2.f, fb);
        float dr = line[1].step(in, delay_time * 2.f, fb);
        float fill = line[3].step(in, delay_time, fb);
        float rr = line[2].step(dr, delay_time, fb);
        *l = dl;
        *r = rr + fill;
    }
};

struct Ref {
    sp_data* sp = nullptr;
    sp_vdelay* v[4]{};
    Ref() {
        sp_create(&sp);
        sp->sr = kRate;
        for (int i = 0; i < 4; i++) {
            sp_vdelay_create(&v[i]);
            sp_vdelay_init(sp, v[i], 10.f);
        }
    }
    ~Ref() {
        for (int i = 0; i < 4; i++) sp_vdelay_destroy(&v[i]);
        sp_destroy(&sp);
    }
    void process(float in, float delay_time, float fb, float* l, float* r) {
        v[0]->del = v[1]->del = delay_time * 2.f;
        v[2]->del = v[3]->del = delay_time;
        for (int i = 0; i < 4; i++) v[i]->feedback = fb;
        float dl = 0, dr = 0, drr = 0, fill = 0;
        sp_vdelay_compute(sp, v[0], &in, &dl);
        sp_vdelay_compute(sp, v[1], &in, &dr);
        sp_vdelay_compute(sp, v[3], &in, &fill);
        sp_vdelay_compute(sp, v[2], &dr, &drr);
        *l = dl;
        *r = drr + fill;
    }
};

struct Acc {
    double max_abs = 0, sum_sq = 0, sig_sq = 0;
    uint64_t n = 0;
    double spec_max = 0, spec_sq = 0;
    uint64_t spec_n = 0;
    double hf_ref = 0, hf_var = 0;
    void add(float a, float b) {
        double d = (double)a - (double)b;
        double ad = d < 0 ? -d : d;
        if (ad > max_abs) max_abs = ad;
        sum_sq += d * d;
        sig_sq += (double)a * (double)a;
        n++;
    }
    double rms() const { return n ? std::sqrt(sum_sq / (double)n) : 0; }
    double sig() const { return n ? std::sqrt(sig_sq / (double)n) : 0; }
    double snr() const {
        double e = rms();
        if (e == 0) return 1e9;
        double s = sig();
        if (s <= 0) return 0;
        return 20.0 * std::log10(s / e);
    }
};

void fft(std::vector<std::complex<double>>& a) {
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
            std::complex<double> w(1.0, 0.0);
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

void spectral(const std::vector<float>& a, const std::vector<float>& b, Acc& acc, float hf_hz) {
    const int N = 4096;
    if ((int)a.size() < N) return;
    std::vector<double> win(N);
    for (int i = 0; i < N; i++)
        win[(size_t)i] = 0.5 * (1.0 - std::cos(2.0 * std::acos(-1.0) * i / (N - 1)));
    std::vector<std::complex<double>> A(N), B(N);
    int hf_bin = (int)(hf_hz * (double)N / (double)kRate);
    if (hf_bin < 1) hf_bin = 1;
    if (hf_bin > N / 2) hf_bin = N / 2;
    for (size_t off = 0; off + (size_t)N <= a.size(); off += (size_t)N) {
        for (int i = 0; i < N; i++) {
            A[(size_t)i] = a[off + (size_t)i] * win[(size_t)i];
            B[(size_t)i] = b[off + (size_t)i] * win[(size_t)i];
        }
        fft(A); fft(B);
        for (int i = 0; i < N / 2; i++) {
            double da = std::abs(A[(size_t)i]);
            double db = std::abs(B[(size_t)i]);
            double d = da - db;
            if (d < 0) d = -d;
            if (d > acc.spec_max) acc.spec_max = d;
            acc.spec_sq += d * d;
            acc.spec_n++;
            if (i >= hf_bin) { acc.hf_ref += da * da; acc.hf_var += db * db; }
        }
    }
}

float noise_at(int i) {
    uint32_t n = (uint32_t)(i + 1) * 1664525u + 1013904223u;
    return ((int32_t)n) * (0.15f / 2147483648.f);
}
float tone_at(int i) {
    return 0.15f * std::sin(2.0 * 3.141592653589793 * 220.0 * (double)i / (double)kRate);
}

enum class Kind { Four, Mono, Asym };

struct Job {
    const char* name;
    Kind kind;
    bool mu;
    float time;
    float fb;
    int frames;
    bool noise;
};

void run_job(const Job& job, FILE* out) {
    const int long_n = 25382, short_n = 12693;
    const int remain = 76150;
    Ref ref;
    Graph g;
    if (job.kind == Kind::Mono) {
        g.setup_mono(remain, false);
        g.tune(job.time, job.mu);
    } else if (job.kind == Kind::Asym) {
        g.setup_four(66538, 2404, false);
        g.line[1].setup(4804, false);
        g.tune_times(5.f, 0.10f, 0.05f, 0.05f, job.mu);
    } else {
        g.setup_four(long_n, short_n, false);
        g.tune(job.time, job.mu);
    }
    std::vector<float> rl(job.frames), rr(job.frames), al(job.frames), ar(job.frames);
    Acc L, R;
    for (int i = 0; i < job.frames; i++) {
        float x = job.noise ? noise_at(i) : tone_at(i);
        float t = job.time;
        float fl = 0, fr = 0, gl = 0, gr = 0;
        if (job.kind == Kind::Asym) {
            // Not a stock wiring: left line at 5 s, the other three at 0.05 s.
            ref.v[0]->del = 5.f; ref.v[1]->del = 0.10f; ref.v[2]->del = 0.05f; ref.v[3]->del = 0.05f;
            for (int k = 0; k < 4; k++) ref.v[k]->feedback = job.fb;
            sp_vdelay_compute(ref.sp, ref.v[0], &x, &fl);
            float dr = 0, fill = 0, drr = 0;
            sp_vdelay_compute(ref.sp, ref.v[1], &x, &dr);
            sp_vdelay_compute(ref.sp, ref.v[3], &x, &fill);
            sp_vdelay_compute(ref.sp, ref.v[2], &dr, &drr);
            fr = drr + fill;
            gl = g.line[0].step(x, 5.f, job.fb);
            float gdr = g.line[1].step(x, 0.10f, job.fb);
            float gfill = g.line[3].step(x, 0.05f, job.fb);
            float gdrr = g.line[2].step(gdr, 0.05f, job.fb);
            gr = gdrr + gfill;
        } else {
            ref.process(x, t, job.fb, &fl, &fr);
            g.process(x, t, job.fb, &gl, &gr);
        }
        rl[(size_t)i] = fl; rr[(size_t)i] = fr;
        al[(size_t)i] = gl; ar[(size_t)i] = gr;
        L.add(fl, gl); R.add(fr, gr);
    }
    int d0 = g.mono ? g.one.d : g.line[0].d;
    int d3 = g.mono ? g.one.d : g.line[3].d;
    float hf = 0.45f * (float)kRate / (float)std::max(d0, d3);
    spectral(rl, al, L, hf);
    spectral(rr, ar, R, hf);
    auto aligned = [](const std::vector<float>& a, const std::vector<float>& b) {
        double best = 1e300; int best_lag = 0; int n = (int)a.size();
        for (int lag = -400; lag <= 400; lag += 1) {
            double e = 0; int c = 0;
            for (int i = 0; i < n; i += 2) {
                int j = i + lag;
                if (j < 0 || j >= n) continue;
                double d = (double)a[(size_t)i] - (double)b[(size_t)j];
                e += d * d; c++;
            }
            if (c && e / c < best) { best = e / c; best_lag = lag; }
        }
        double sig = 0; int c = 0;
        for (int i = 0; i < n; i += 2) {
            int j = i + best_lag;
            if (j < 0 || j >= n) continue;
            sig += (double)a[(size_t)i] * (double)a[(size_t)i]; c++;
        }
        double snr = best <= 0 ? 1e9 : (sig > 0 && c ? 20.0 * std::log10(std::sqrt(sig / c) / std::sqrt(best)) : 0);
        return std::pair<double, int>(snr, best_lag);
    };
    auto alL = aligned(rl, al);
    auto alR = aligned(rr, ar);
    std::fprintf(out,
        "{\"name\":\"%s\",\"frames\":%d,\"delay_s\":%.4g,\"fb\":%.3g,\"D_long\":%d,\"D_short\":%d,"
        "\"max_l\":%.8g,\"max_r\":%.8g,\"rms_l\":%.8g,\"rms_r\":%.8g,\"snr_l\":%.6g,\"snr_r\":%.6g,"
        "\"spec_max_l\":%.8g,\"spec_max_r\":%.8g,\"spec_rms_l\":%.8g,\"spec_rms_r\":%.8g,"
        "\"hf_hz\":%.6g,\"hf_ratio_l\":%.6g,\"hf_ratio_r\":%.6g,"
        "\"snr_aligned_l\":%.6g,\"snr_aligned_r\":%.6g,\"align_lag_l\":%d,\"align_lag_r\":%d}\n",
        job.name, job.frames, job.time, job.fb, d0, d3,
        L.max_abs, R.max_abs, L.rms(), R.rms(), L.snr(), R.snr(),
        L.spec_max, R.spec_max,
        L.spec_n ? std::sqrt(L.spec_sq / (double)L.spec_n) : 0,
        R.spec_n ? std::sqrt(R.spec_sq / (double)R.spec_n) : 0,
        hf,
        L.hf_ref > 0 ? L.hf_var / L.hf_ref : 0,
        R.hf_ref > 0 ? R.hf_var / R.hf_ref : 0,
        alL.first, alR.first, alL.second, alR.second);
    std::fflush(out);
    std::fprintf(stderr, "hybrid %s\n", job.name);
}

float step_max(const std::vector<float>& x, int a, int b) {
    float m = 0.f;
    if (a < 1) a = 1;
    if (b > (int)x.size()) b = (int)x.size();
    for (int i = a; i < b; i++) {
        float d = std::fabs(x[(size_t)i] - x[(size_t)(i - 1)]);
        if (d > m) m = d;
    }
    return m;
}

void click_test(FILE* out) {
    const int N = kRate;
    const int at = N / 2;
    const int fade = 256;
    auto once = [&](bool fade_it, std::vector<float>* y, float* stale) {
        Graph g;
        g.setup_four(25382, 12693, false);
        g.tune(0.20f, false);
        y->assign((size_t)N, 0.f);
        auto src = [](int) { return 0.25f; };
        for (int i = 0; i < at; i++) {
            float l, r;
            g.process(src(i), 0.20f, 0.35f, &l, &r);
            (*y)[(size_t)i] = 0.5f * (l + r);
        }
        if (!fade_it) {
            g.tune(0.40f, false);
            g.clear();
            for (int i = at; i < N; i++) {
                float l, r;
                g.process(src(i), 0.40f, 0.35f, &l, &r);
                (*y)[(size_t)i] = 0.5f * (l + r);
            }
        } else {
            for (int i = 0; i < fade; i++) {
                float gmix = (float)(i + 1) / (float)fade;
                float l, r;
                g.process(src(at + i), 0.20f, 0.35f, &l, &r);
                float wet = 0.5f * (l + r);
                float dry = src(at + i);
                (*y)[(size_t)(at + i)] = (1.f - gmix) * wet + gmix * dry;
            }
            g.tune(0.40f, false);
            g.clear();
            for (int i = 0; i < fade && at + fade + i < N; i++) {
                float gmix = (float)(i + 1) / (float)fade;
                float l, r;
                g.process(src(at + fade + i), 0.40f, 0.35f, &l, &r);
                float wet = 0.5f * (l + r);
                float dry = src(at + fade + i);
                (*y)[(size_t)(at + fade + i)] = (1.f - gmix) * dry + gmix * wet;
            }
            for (int i = at + 2 * fade; i < N; i++) {
                float l, r;
                g.process(src(i), 0.40f, 0.35f, &l, &r);
                (*y)[(size_t)i] = 0.5f * (l + r);
            }
        }
        *stale = 0.f;
        Graph quiet = g;
        // Fresh clear already happened on the fade path. Measure a silent tail
        // on a cleared line at the new rate: any leftover would be stale.
        quiet.clear();
        quiet.tune(0.40f, false);
        for (int i = 0; i < 4000; i++) {
            float l, r;
            quiet.process(0.f, 0.40f, 0.35f, &l, &r);
            float a = std::fabs(0.5f * (l + r));
            if (a > *stale) *stale = a;
        }
    };
    std::vector<float> faded, abrupt;
    float stale_f = 0, stale_a = 0;
    once(true, &faded, &stale_f);
    once(false, &abrupt, &stale_a);
    float steady = step_max(faded, 2000, at - 10);
    float fade_step = step_max(faded, at - 2, at + 2 * fade + 2);
    float abrupt_step = step_max(abrupt, at - 2, at + 8);
    bool pass = fade_step <= std::max(steady * 2.f, 0.002f) && abrupt_step > fade_step && stale_f < 1e-3f;
    std::fprintf(out,
        "{\"name\":\"rate_boundary\",\"frames\":%d,\"cross_at\":%d,\"fade\":%d,"
        "\"from_s\":0.20,\"to_s\":0.40,\"steady_max_step\":%.8g,\"fade_max_step\":%.8g,"
        "\"abrupt_max_step\":%.8g,\"stale_after_clear\":%.8g,\"pass\":%s}\n",
        N, at, fade, steady, fade_step, abrupt_step, stale_f, pass ? "true" : "false");
    std::fprintf(stderr, "hybrid rate_boundary pass=%s fade=%.6g abrupt=%.6g\n",
                 pass ? "true" : "false", fade_step, abrupt_step);
}

void rate_table(FILE* out) {
    const int S = 12689, L = 25378, M = 76146, MU = 25378;
    std::fprintf(out, "{\"record\":\"rates\",\"sr\":%d", kRate);
    auto emit = [&](const char* name, int admitted, bool half) {
        std::fprintf(out, ",\"%s\":[", name);
        int ds[4] = {1, 2, 4, 10};
        for (int i = 0; i < 4; i++) {
            double line = (double)admitted * ds[i] / (double)kRate;
            double param = half ? line * 0.5 : line;
            std::fprintf(out, "%s{\"D\":%d,\"line_s\":%.6f,\"delayTime_s\":%.6f}",
                         i ? "," : "", ds[i], line, param);
        }
        std::fprintf(out, "]");
    };
    emit("four_short_12bit", S, false);
    emit("four_long_12bit", L, true);
    emit("four_short_mulaw", MU, false);
    emit("mono_pool_12bit", M, true);
    std::fprintf(out, "}\n");
}

} // namespace

int main(int argc, char** argv) {
    const char* path = argc > 1 ? argv[1] : "HYBRID.json";
    FILE* out = std::fopen(path, "w");
    if (!out) return 1;
    rate_table(out);
    Job jobs[] = {
        {"H_short", Kind::Four, false, 0.05f, 0.1f, kRate, false},
        {"H_noise", Kind::Four, false, 0.25f, 0.9f, kRate, true},
        {"H_long", Kind::Four, false, 2.5f, 0.9f, 6 * kRate, false},
        {"H_noise_long", Kind::Four, false, 2.5f, 0.5f, 4 * kRate, true},
        {"H_noise_040", Kind::Four, false, 0.40f, 0.5f, kRate, true},
        {"Hmu_short", Kind::Four, true, 0.05f, 0.1f, kRate, false},
        {"Hmu_noise", Kind::Four, true, 0.25f, 0.9f, kRate, true},
        {"Hmu_long", Kind::Four, true, 2.5f, 0.9f, 6 * kRate, false},
        {"Hmu_noise_040", Kind::Four, true, 0.40f, 0.5f, kRate, true},
        {"Hmono_short", Kind::Mono, false, 0.05f, 0.1f, kRate, false},
        {"Hmono_noise", Kind::Mono, false, 0.25f, 0.9f, kRate, true},
        {"Hmono_long", Kind::Mono, false, 2.5f, 0.9f, 6 * kRate, false},
        {"Hmono_mu_long", Kind::Mono, true, 2.5f, 0.9f, 6 * kRate, false},
        {"pool_asym_long", Kind::Asym, false, 2.5f, 0.5f, 6 * kRate, true},
    };
    for (const Job& j : jobs) run_job(j, out);
    click_test(out);
    std::fclose(out);
    return 0;
}
