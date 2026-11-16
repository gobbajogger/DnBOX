/* Fit labelled stand-ins and score the integer resynthesis.
 * The private 12-record SuperJ6 PCM is not read. */
#include "sj6_resynth.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static uint32_t inc_of(double hz, int rate)
{
    if (hz <= 0 || hz >= rate * 0.45) return 0;
    return (uint32_t)(hz / (double)rate * 4294967296.0);
}

static double goertzel(const int32_t *x, int n, double hz, int rate)
{
    double w = 2.0 * 3.14159265358979323846 * hz / (double)rate;
    double coeff = 2.0 * cos(w);
    double s1 = 0, s2 = 0;
    int i;
    if (n < 8 || hz <= 0) return 0;
    for (i = 0; i < n; i++) {
        double s0 = (double)x[i] + coeff * s1 - s2;
        s2 = s1;
        s1 = s0;
    }
    return sqrt(s1 * s1 + s2 * s2 - coeff * s1 * s2) * 2.0 / (double)n;
}

static int best_lag(const int32_t *x, int n)
{
    int start = n / 3;
    int len = n / 3;
    int lag, best = 0;
    double r0 = 0, bestv = 0;
    if (len > 8192) len = 8192;
    if (start + len > n) return 0;
    for (lag = 0; lag < len; lag++) r0 += (double)x[start + lag] * x[start + lag];
    if (r0 < 1) return 0;
    for (lag = 32; lag < 400 && lag < len; lag++) {
        double s = 0;
        int i;
        for (i = 0; i < len - lag; i++) s += (double)x[start + i] * x[start + i + lag];
        if (s > bestv) { bestv = s; best = lag; }
    }
    if (bestv < 0.35 * r0) return 0;
    return best;
}

static void clear_voice(Sj6Voice *v, const char *name, uint32_t seed)
{
    memset(v, 0, sizeof *v);
    snprintf(v->name, sizeof v->name, "%s", name);
    v->attack = 1;
    v->decay = 1;
    v->noise_seed = seed;
}

static void fit_voice(const int32_t *x, int n, int rate, Sj6Voice *v, const char *name)
{
    int lag = best_lag(x, n);
    int h, peak_at = 0;
    int32_t peak_abs = 0;
    int guard = n / 5;
    if (guard < 256) guard = n > 256 ? 256 : n;
    clear_voice(v, name, 0x6d2b79f5u);
    for (h = 0; h < guard; h++) {
        int32_t a = x[h] < 0 ? -x[h] : x[h];
        if (a > peak_abs) { peak_abs = a; peak_at = h; }
    }
    v->attack = (uint16_t)(peak_at > 0 ? peak_at : 1);
    v->decay = (uint16_t)(n / 6 > 1 ? n / 6 : 1);
    v->drift_len = 0;
    v->drift_depth = 0;
    if (lag) {
        double f0 = (double)rate / (double)lag;
        int sus_n = n / 3;
        int atk_n = v->attack + v->decay;
        if (sus_n > 16384) sus_n = 16384;
        if (atk_n > n / 2) atk_n = n / 2;
        if (atk_n < 256) atk_n = n > 256 ? 256 : n;
        for (h = 1; h <= SJ6_PARTIALS; h++) {
            double hz = f0 * h;
            double ms, ma;
            uint32_t inc = inc_of(hz, rate);
            if (!inc) break;
            ms = goertzel(x + n - sus_n, sus_n, hz, rate);
            ma = goertzel(x + (peak_at > 128 ? peak_at - 128 : 0), atk_n > 512 ? 512 : atk_n, hz, rate);
            if (ma < ms) ma = ms;
            if (ma < 8 && ms < 8) continue;
            v->inc[h - 1] = inc;
            v->peak[h - 1] = (int16_t)(ma > 32767 ? 32767 : ma);
            v->sustain[h - 1] = (int16_t)(ms > 32767 ? 32767 : ms);
        }
    } else {
        double ea = 0, es = 0;
        int i, ns = 0, na = guard;
        for (i = 0; i < na; i++) ea += (double)x[i] * x[i];
        for (i = n / 2; i < n; i++) { es += (double)x[i] * x[i]; ns++; }
        ea = sqrt(ea / na);
        es = ns ? sqrt(es / ns) : 0;
        if (ea > 20000) ea = 20000;
        if (es > 20000) es = 20000;
        v->noise_peak = (int16_t)ea;
        v->noise_sustain = (int16_t)es;
    }
}

static double render_rms(const int32_t *x, int n, const Sj6Voice *v)
{
    Sj6State st;
    int32_t *y = (int32_t *)malloc((size_t)n * sizeof(int32_t));
    double se = 0;
    int i;
    if (!y) return 1e300;
    sj6_state_init(&st, v->noise_seed);
    sj6_render(v, &st, y, n, 0);
    for (i = 0; i < n; i++) {
        double d = (double)x[i] - y[i];
        se += d * d;
    }
    free(y);
    return sqrt(se / n);
}

static double signal_rms(const int32_t *x, int n)
{
    double s = 0;
    int i;
    for (i = 0; i < n; i++) s += (double)x[i] * x[i];
    return sqrt(s / n);
}

static void keep_lower(const int32_t *x, int n, Sj6Voice *v, const Sj6Voice *alt)
{
    Sj6State st;
    int32_t *y = (int32_t *)malloc((size_t)n * sizeof(int32_t));
    double src, got, err, cur;
    int i;
    if (!y) return;
    sj6_state_init(&st, alt->noise_seed);
    sj6_render(alt, &st, y, n, 0);
    src = signal_rms(x, n);
    got = signal_rms(y, n);
    err = 0;
    for (i = 0; i < n; i++) {
        double d = (double)x[i] - y[i];
        err += d * d;
    }
    free(y);
    err = sqrt(err / n);
    cur = render_rms(x, n, v);
    /* A silent model has a small log-spectral disaster even when the
     * absolute error looks similar. Keep the model audible. */
    if (src > 1 && got < src * 0.25) return;
    if (err < cur) *v = *alt;
}

static void polish(const int32_t *x, int n, Sj6Voice *v)
{
    Sj6Voice alt = *v;
    alt.drift_len = 0;
    alt.drift_depth = 0;
    keep_lower(x, n, v, &alt);
    alt = *v;
    alt.drift_len = (uint16_t)(n > 65535 ? 65535 : n);
    alt.drift_depth = 800;
    keep_lower(x, n, v, &alt);
    alt = *v;
    alt.noise_peak = 0;
    alt.noise_sustain = 0;
    keep_lower(x, n, v, &alt);
}

static void scale_to_loudness(const int32_t *x, int n, Sj6Voice *v)
{
    Sj6State st;
    int32_t *y = (int32_t *)malloc((size_t)n * sizeof(int32_t));
    double ro = 0, rr = 0, g;
    int i, p;
    if (!y) return;
    sj6_state_init(&st, v->noise_seed);
    sj6_render(v, &st, y, n, 0);
    for (i = 0; i < n; i++) {
        ro += (double)x[i] * x[i];
        rr += (double)y[i] * y[i];
    }
    free(y);
    if (rr < 1 || ro < 1) return;
    g = sqrt(ro / rr);
    if (g > 8) g = 8;
    if (g < 0.05) g = 0.05;
    for (p = 0; p < SJ6_PARTIALS; p++) {
        v->peak[p] = (int16_t)(v->peak[p] * g);
        v->sustain[p] = (int16_t)(v->sustain[p] * g);
    }
    v->noise_peak = (int16_t)(v->noise_peak * g);
    v->noise_sustain = (int16_t)(v->noise_sustain * g);
}

static void fit_noise_floor(const int32_t *x, int n, Sj6Voice *v)
{
    Sj6State st;
    int32_t *y = (int32_t *)malloc((size_t)n * sizeof(int32_t));
    double ea = 0, es = 0;
    int i, na, ns;
    if (!y) return;
    sj6_state_init(&st, v->noise_seed);
    sj6_render(v, &st, y, n, 0);
    na = v->attack + v->decay;
    if (na > n / 2) na = n / 2;
    if (na < 1) na = 1;
    for (i = 0; i < na; i++) {
        double d = (double)x[i] - y[i];
        ea += d * d;
    }
    ns = 0;
    for (i = n / 2; i < n; i++) {
        double d = (double)x[i] - y[i];
        es += d * d;
        ns++;
    }
    free(y);
    ea = sqrt(ea / na);
    es = ns ? sqrt(es / ns) : 0;
    if (ea > 32767) ea = 32767;
    if (es > 32767) es = 32767;
    v->noise_peak = (int16_t)ea;
    v->noise_sustain = (int16_t)es;
}

static int load_wav(const char *path, int32_t **out, int *n, int *rate)
{
    FILE *f = fopen(path, "rb");
    unsigned char hdr[12], chunk[8];
    int audio = 0, ch = 0, bps = 0, sr = 0;
    unsigned char *data = 0;
    uint32_t dsz = 0;
    int i, count;
    if (!f) return -1;
    if (fread(hdr, 1, 12, f) != 12 || memcmp(hdr, "RIFF", 4) || memcmp(hdr + 8, "WAVE", 4)) {
        fclose(f); return -1;
    }
    while (fread(chunk, 1, 8, f) == 8) {
        uint32_t sz = (uint32_t)chunk[4] | ((uint32_t)chunk[5] << 8) | ((uint32_t)chunk[6] << 16) | ((uint32_t)chunk[7] << 24);
        if (!memcmp(chunk, "fmt ", 4) && sz >= 16) {
            unsigned char fmt[16];
            if (fread(fmt, 1, 16, f) != 16) { fclose(f); return -1; }
            audio = fmt[0] | (fmt[1] << 8);
            ch = fmt[2] | (fmt[3] << 8);
            sr = (int)((uint32_t)fmt[4] | ((uint32_t)fmt[5] << 8) | ((uint32_t)fmt[6] << 16) | ((uint32_t)fmt[7] << 24));
            bps = fmt[14] | (fmt[15] << 8);
            if (sz > 16) fseek(f, (long)(sz - 16), SEEK_CUR);
        } else if (!memcmp(chunk, "data", 4)) {
            data = (unsigned char *)malloc(sz);
            if (!data || fread(data, 1, sz, f) != sz) { free(data); fclose(f); return -1; }
            dsz = sz;
            break;
        } else fseek(f, (long)sz, SEEK_CUR);
        if (sz & 1) fseek(f, 1, SEEK_CUR);
    }
    fclose(f);
    if (audio != 1 || ch != 1 || !data || (bps != 16 && bps != 24)) { free(data); return -1; }
    count = (int)(dsz / (bps / 8));
    *out = (int32_t *)malloc((size_t)count * sizeof(int32_t));
    if (!*out) { free(data); return -1; }
    if (bps == 16) {
        for (i = 0; i < count; i++) (*out)[i] = (int16_t)(data[i * 2] | (data[i * 2 + 1] << 8));
    } else {
        for (i = 0; i < count; i++) {
            int32_t s = data[i * 3] | (data[i * 3 + 1] << 8) | (data[i * 3 + 2] << 16);
            if (s & 0x800000) s |= ~0xffffff;
            (*out)[i] = s >> 8;
        }
    }
    free(data);
    *n = count;
    *rate = sr;
    return 0;
}

static double db(double x)
{
    if (!(x > 0)) return -300;
    return 20.0 * log10(x);
}

static void spectrum(const int32_t *x, int n, int rate, double *mag, int bins)
{
    int w, k, i;
    int wins = 0;
    memset(mag, 0, (size_t)bins * sizeof(double));
    for (w = 0; w + 1024 <= n; w += n / 8 > 1024 ? n / 8 : 1024) {
        for (k = 1; k < bins; k++) {
            double hz = (double)k * rate / 1024.0;
            mag[k] += goertzel(x + w, 1024, hz, rate);
        }
        wins++;
        if (wins >= 6) break;
    }
    if (!wins) wins = 1;
    for (k = 0; k < bins; k++) mag[k] /= wins;
    (void)i;
}

static void score(const char *label, const int32_t *x, int n, int rate, const Sj6Voice *v, FILE *params)
{
    Sj6State st;
    Sj6Cost cost;
    int32_t *y = (int32_t *)malloc((size_t)n * sizeof(int32_t));
    double se = 0, so = 0, sr = 0;
    int32_t peak = 0;
    int i, lag_o, lag_r;
    double mo[128], mr[128], lsd = 0;
    int nb = 0;
    memset(&cost, 0, sizeof cost);
    sj6_state_init(&st, v->noise_seed);
    sj6_render(v, &st, y, n, &cost);
    for (i = 0; i < n; i++) {
        double d = (double)x[i] - y[i];
        int32_t a = (int32_t)d;
        if (a < 0) a = -a;
        if (a > peak) peak = a;
        se += d * d;
        so += (double)x[i] * x[i];
        sr += (double)y[i] * y[i];
    }
    spectrum(x, n, rate, mo, 128);
    spectrum(y, n, rate, mr, 128);
    for (i = 1; i < 128; i++) {
        lsd += fabs(log10(mo[i] + 1.0) - log10(mr[i] + 1.0));
        nb++;
    }
    lsd = nb ? (20.0 * lsd / nb) : 0;
    lag_o = best_lag(x, n);
    lag_r = best_lag(y, n);
    {
        double ops = (double)(cost.muls + cost.adds + cost.shifts + cost.branches);
        double per = ops / (double)n;
        double cpu = per * 44100.0 / 320e6 * 100.0;
        double cents = (lag_o && lag_r) ? 1200.0 * log2((double)lag_o / (double)lag_r) : 0;
        printf("%s n=%d param_bytes=%u null_rms_dBFS=%.2f null_peak_dBFS=%.2f log_spectral_dB=%.2f loudness_dB=%.2f pitch_cents=%.2f pitch_ok=%d ops/sample=%.1f cpu%%=%.3f\n",
               label, n, (unsigned)sizeof(Sj6Voice),
               db(sqrt(se / n) / 32768.0), db((double)peak / 32768.0), lsd,
               db(sqrt(sr / n) / (sqrt(so / n) + 1e-12)), cents, lag_o && lag_r, per, cpu);
    }
    if (params) {
        int p;
        fprintf(params, "    {\"%s\", %u, %u, %u, %d, %d, %d, %uu, {",
                v->name, v->attack, v->decay, v->drift_len, v->drift_depth,
                v->noise_peak, v->noise_sustain, v->noise_seed);
        for (p = 0; p < SJ6_PARTIALS; p++) fprintf(params, "%s%uu", p ? ", " : "", v->inc[p]);
        fprintf(params, "}, {");
        for (p = 0; p < SJ6_PARTIALS; p++) fprintf(params, "%s%d", p ? ", " : "", v->peak[p]);
        fprintf(params, "}, {");
        for (p = 0; p < SJ6_PARTIALS; p++) fprintf(params, "%s%d", p ? ", " : "", v->sustain[p]);
        fprintf(params, "}},\n");
    }
    free(y);
}

static void make_synthetic(int32_t *x, int n)
{
    Sj6Voice v;
    Sj6State st;
    int p;
    clear_voice(&v, "SYNTHETIC-C4", 1u);
    v.attack = 2000;
    v.decay = 6000;
    v.drift_len = 8000;
    v.drift_depth = 800;
    v.noise_peak = 400;
    v.noise_sustain = 180;
    for (p = 0; p < 5; p++) {
        v.inc[p] = inc_of(261.6255653005986 * (p + 1), 44100);
        v.peak[p] = (int16_t)(14000 / (p + 1));
        v.sustain[p] = (int16_t)(5000 / (p + 1));
    }
    sj6_state_init(&st, v.noise_seed);
    sj6_render(&v, &st, x, n, 0);
}

int main(void)
{
    const char *files[] = {
        "assets/factory-breaks/909FL.wav",
        "assets/factory-breaks/909GR.wav",
        "assets/909/ride.wav",
        "assets/909/crash.wav",
        "assets/909/hh.wav"
    };
    FILE *params = fopen("firmware/src/dsp/sj6_standin.inc", "w");
    int i;
    int32_t *syn;
    Sj6Voice fitted, known;
    if (!params) { printf("FAIL open params\n"); return 1; }
    fprintf(params, "static const Sj6Voice sj6_standins[] = {\n");
    syn = (int32_t *)malloc(16384 * sizeof(int32_t));
    make_synthetic(syn, 16384);
    clear_voice(&known, "SYNTHETIC-C4", 1u);
    fit_voice(syn, 16384, 44100, &fitted, "SYNTHETIC-C4");
    scale_to_loudness(syn, 16384, &fitted);
    fit_noise_floor(syn, 16384, &fitted);
    polish(syn, 16384, &fitted);
    score("STAND-IN SYNTHETIC-C4 (descriptor pitch, not a J6 record)", syn, 16384, 44100, &fitted, params);
    free(syn);
    for (i = 0; i < 5; i++) {
        int32_t *x = 0;
        int n = 0, rate = 0;
        char label[128];
        if (load_wav(files[i], &x, &n, &rate)) { printf("FAIL load %s\n", files[i]); return 1; }
        {
            const char *slash = strrchr(files[i], '/');
            const char *base = slash ? slash + 1 : files[i];
            fit_voice(x, n, rate, &fitted, base);
            scale_to_loudness(x, n, &fitted);
            fit_noise_floor(x, n, &fitted);
            polish(x, n, &fitted);
            snprintf(label, sizeof label, "STAND-IN %s", base);
        }
        score(label, x, n, rate, &fitted, params);
        free(x);
    }
    fprintf(params, "};\n");
    fclose(params);
    printf("PASS stand-in fit. Private 12-record SuperJ6 PCM NOT_RUN\n");
    printf("voice_bytes=%u\n", (unsigned)sizeof(Sj6Voice));
    return 0;
}
