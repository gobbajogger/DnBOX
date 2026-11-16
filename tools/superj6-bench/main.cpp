/* Original-vs-port render bench.
 * Oscillator presets run now. Recorded voices are included when WAV files
 * are present in --pcm-dir (default /workspace/dnbox-handoff/from-dot-files).
 * Those recordings are never written into the repository.
 *
 * Usage: superj6-bench [--out DIR] [--pcm-dir DIR]
 */
#include "sj6_analytic.h"
#include "OriginalAnalyticOscillator.h"
#include "OriginalWhiteNoise.h"
#include "fit.h"
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <dirent.h>
#include <string>
#include <vector>

static uint32_t bits_of(float x)
{
    uint32_t u;
    memcpy(&u, &x, 4);
    return u;
}

static void write_wav(const char *path, const int16_t *x, int n)
{
    FILE *f = std::fopen(path, "wb");
    uint32_t data = (uint32_t)n * 2, riff = 36 + data, rate = 44100, byte_rate = 44100 * 2;
    uint16_t ch = 1, bits = 16, block = 2;
    if (!f) return;
    std::fwrite("RIFF", 1, 4, f);
    std::fwrite(&riff, 4, 1, f);
    std::fwrite("WAVEfmt ", 1, 8, f);
    uint32_t fmt = 16;
    uint16_t pcm = 1;
    std::fwrite(&fmt, 4, 1, f);
    std::fwrite(&pcm, 2, 1, f);
    std::fwrite(&ch, 2, 1, f);
    std::fwrite(&rate, 4, 1, f);
    std::fwrite(&byte_rate, 4, 1, f);
    std::fwrite(&block, 2, 1, f);
    std::fwrite(&bits, 2, 1, f);
    std::fwrite("data", 1, 4, f);
    std::fwrite(&data, 4, 1, f);
    std::fwrite(x, 2, (size_t)n, f);
    std::fclose(f);
}

static int16_t q16(float x)
{
    if (x > 1.f) x = 1.f;
    if (x < -1.f) x = -1.f;
    return (int16_t)lrintf(x * 32767.f);
}

struct Metrics {
    int n, exact, compared;
    double null_db, peak_db, lsd;
};

static double db(double x) { return x > 0 ? 20.0 * std::log10(x) : -300; }

static Metrics compare(const float *a, const float *b, int n)
{
    Metrics m{};
    double se = 0, peak = 0;
    int i, k, w;
    m.n = n;
    for (i = 0; i < n; i++) {
        float d = a[i] - b[i];
        m.compared++;
        if (bits_of(a[i]) == bits_of(b[i])) m.exact++;
        se += (double)d * d;
        if (std::fabs(d) > peak) peak = std::fabs(d);
    }
    m.null_db = db(std::sqrt(se / n));
    m.peak_db = db(peak);
    m.lsd = 0;
    int bins = 0;
    for (w = 0; w + 256 <= n && bins < 4; w += 256) {
        for (k = 1; k < 32; k++) {
            double oa = 0, ob = 0, ca = 0, cb = 0;
            double ang = 2 * 3.14159265358979323846 * k / 256.0;
            for (i = 0; i < 256; i++) {
                oa += a[w + i] * std::cos(ang * i);
                ca += a[w + i] * std::sin(ang * i);
                ob += b[w + i] * std::cos(ang * i);
                cb += b[w + i] * std::sin(ang * i);
            }
            double ma = std::sqrt(oa * oa + ca * ca) + 1e-12;
            double mb = std::sqrt(ob * ob + cb * cb) + 1e-12;
            m.lsd += std::fabs(std::log10(ma) - std::log10(mb));
            bins++;
        }
    }
    if (bins) m.lsd = 20.0 * m.lsd / bins;
    return m;
}

static void dump(const char *dir, const char *name, const float *orig, const float *port, int n, FILE *json, int first)
{
    std::vector<int16_t> a(n), b(n), d(n);
    char path[512];
    int i;
    for (i = 0; i < n; i++) {
        a[i] = q16(orig[i]);
        b[i] = q16(port[i]);
        int v = (int)a[i] - (int)b[i];
        if (v > 32767) v = 32767;
        if (v < -32768) v = -32768;
        d[i] = (int16_t)v;
    }
    std::snprintf(path, sizeof path, "%s/%s-original.wav", dir, name);
    write_wav(path, a.data(), n);
    std::snprintf(path, sizeof path, "%s/%s-port.wav", dir, name);
    write_wav(path, b.data(), n);
    std::snprintf(path, sizeof path, "%s/%s-diff.wav", dir, name);
    write_wav(path, d.data(), n);
    Metrics m = compare(orig, port, n);
    std::fprintf(json, "%s{\"name\":\"%s\",\"samples\":%d,\"bit_exact\":%d,\"null_dBFS\":%.2f,\"null_peak_dBFS\":%.2f,\"log_spectral_dB\":%.2f}",
                 first ? "" : ",\n", name, m.compared, m.exact, m.null_db, m.peak_db, m.lsd);
    std::printf("%s bit_exact %d / %d null_dBFS %.2f lsd %.2f\n", name, m.exact, m.compared, m.null_db, m.lsd);
}

static int load_wav(const char *path, std::vector<int32_t> &out, int *rate)
{
    FILE *f = std::fopen(path, "rb");
    unsigned char hdr[12], chunk[8];
    int audio = 0, ch = 0, bps = 0, sr = 0;
    if (!f) return -1;
    if (std::fread(hdr, 1, 12, f) != 12 || memcmp(hdr, "RIFF", 4) || memcmp(hdr + 8, "WAVE", 4)) { std::fclose(f); return -1; }
    while (std::fread(chunk, 1, 8, f) == 8) {
        uint32_t sz = (uint32_t)chunk[4] | ((uint32_t)chunk[5] << 8) | ((uint32_t)chunk[6] << 16) | ((uint32_t)chunk[7] << 24);
        if (!memcmp(chunk, "fmt ", 4) && sz >= 16) {
            unsigned char fmt[16];
            if (std::fread(fmt, 1, 16, f) != 16) { std::fclose(f); return -1; }
            audio = fmt[0] | (fmt[1] << 8);
            ch = fmt[2] | (fmt[3] << 8);
            sr = (int)((uint32_t)fmt[4] | ((uint32_t)fmt[5] << 8) | ((uint32_t)fmt[6] << 16) | ((uint32_t)fmt[7] << 24));
            bps = fmt[14] | (fmt[15] << 8);
            if (sz > 16) std::fseek(f, (long)(sz - 16), SEEK_CUR);
        } else if (!memcmp(chunk, "data", 4)) {
            if (audio != 1 || ch != 1 || (bps != 16 && bps != 24)) { std::fclose(f); return -1; }
            int count = (int)(sz / (bps / 8));
            if (count > 16384) count = 16384;
            out.resize(count);
            for (int i = 0; i < count; i++) {
                if (bps == 16) {
                    unsigned char s[2];
                    if (std::fread(s, 1, 2, f) != 2) { std::fclose(f); return -1; }
                    out[i] = (int16_t)(s[0] | (s[1] << 8));
                } else {
                    unsigned char s[3];
                    if (std::fread(s, 1, 3, f) != 3) { std::fclose(f); return -1; }
                    int32_t v = s[0] | (s[1] << 8) | (s[2] << 16);
                    if (v & 0x800000) v |= ~0xffffff;
                    out[i] = v >> 8;
                }
            }
            std::fclose(f);
            *rate = sr;
            return 0;
        } else std::fseek(f, (long)sz, SEEK_CUR);
        if (sz & 1) std::fseek(f, 1, SEEK_CUR);
    }
    std::fclose(f);
    return -1;
}

int main(int argc, char **argv)
{
    const char *out_dir = "/tmp/sj6-bench";
    const char *pcm_dir = "/workspace/dnbox-handoff/from-dot-files";
    int i;
    for (i = 1; i < argc; i++) {
        if (!std::strcmp(argv[i], "--out") && i + 1 < argc) out_dir = argv[++i];
        else if (!std::strcmp(argv[i], "--pcm-dir") && i + 1 < argc) pcm_dir = argv[++i];
    }
    std::string mkdir = std::string("mkdir -p ") + out_dir;
    if (std::system(mkdir.c_str())) return 1;
    char json_path[512];
    std::snprintf(json_path, sizeof json_path, "%s/metrics.json", out_dir);
    FILE *json = std::fopen(json_path, "w");
    if (!json) return 1;
    std::fprintf(json, "[\n");
    const float inc = 0.02f;
    const int n = 2048;
    int first = 1;
    auto osc = [&](const char *name, int kind) {
        std::vector<float> a(n), b(n);
        DunneCore::OriginalAnalyticOscillator o;
        Sj6Osc p;
        o.reset();
        sj6_osc_reset(&p);
        o.increment = p.increment = inc;
        for (int s = 0; s < n; s++) {
            if (kind == 0) { a[s] = o.processSaw(1.f); b[s] = sj6_osc_saw(&p, 1.f); }
            else if (kind == 1) { a[s] = o.processTriangle(1.f); b[s] = sj6_osc_triangle(&p, 1.f); }
            else if (kind == 2) { a[s] = o.processSine(1.f); b[s] = sj6_osc_sine(&p, 1.f); }
            else { a[s] = o.processPWM(1.f, 0.5f); b[s] = sj6_osc_pwm(&p, 1.f, 0.5f); }
        }
        dump(out_dir, name, a.data(), b.data(), n, json, first);
        first = 0;
    };
    osc("saw", 0);
    osc("triangle", 1);
    osc("sine", 2);
    osc("pulse", 3);
    {
        std::vector<float> a(n), b(n);
        DunneCore::OriginalWhiteNoise rn;
        Sj6Noise pn;
        rn.seed = pn.seed = 1;
        rn.reset();
        sj6_noise_reset(&pn);
        for (int s = 0; s < n; s++) { a[s] = rn.sample(); b[s] = sj6_noise_sample(&pn); }
        dump(out_dir, "noise", a.data(), b.data(), n, json, first);
        first = 0;
    }
    DIR *dir = opendir(pcm_dir);
    int pcm_n = 0;
    if (!dir) std::printf("recorded voices NOT_RUN (no %s)\n", pcm_dir);
    else {
        struct dirent *ent;
        while ((ent = readdir(dir))) {
            const char *nm = ent->d_name;
            size_t len = std::strlen(nm);
            if (len < 5 || std::strcmp(nm + len - 4, ".wav")) continue;
            std::string path = std::string(pcm_dir) + "/" + nm;
            std::vector<int32_t> pcm;
            int rate = 0;
            if (load_wav(path.c_str(), pcm, &rate) || pcm.empty()) continue;
            Sj6Voice voice;
            std::string label = std::string("rec-") + nm;
            if (label.size() > 20) label.resize(20);
            if (sj6_fit(pcm.data(), (int)pcm.size(), rate, -1, -1, label.c_str(), &voice)) continue;
            std::vector<int32_t> y(pcm.size());
            Sj6State st;
            sj6_state_init(&st, &voice);
            sj6_render(&voice, &st, y.data(), (int)y.size(), 0);
            std::vector<float> a(pcm.size()), b(pcm.size());
            for (size_t s = 0; s < pcm.size(); s++) { a[s] = pcm[s] / 32768.f; b[s] = y[s] / 32768.f; }
            dump(out_dir, label.c_str(), a.data(), b.data(), (int)a.size(), json, first);
            first = 0;
            pcm_n++;
        }
        closedir(dir);
        if (!pcm_n) std::printf("recorded voices NOT_RUN (%s has no wav)\n", pcm_dir);
    }
    std::fprintf(json, "\n]\n");
    std::fclose(json);
    std::printf("metrics %s\n", json_path);
    return 0;
}
