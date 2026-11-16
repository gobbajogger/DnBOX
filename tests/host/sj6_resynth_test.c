/* Fit the integer resynthesis and require the synthetic stand-in under -40 dBFS. */
#include "sj6_resynth.h"
#include "fit.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static uint32_t inc_of(double hz, int rate)
{
    return (uint32_t)(hz / (double)rate * 4294967296.0);
}

static double db(double x)
{
    if (!(x > 0)) return -300;
    return 20.0 * log10(x);
}

static double score_null(const int32_t *x, int n, const Sj6Voice *v, double *peak_db, double *ops, double *cpu)
{
    Sj6State st;
    Sj6Cost cost;
    int32_t *y = malloc((size_t)n * sizeof(int32_t));
    double se = 0;
    int32_t peak = 0;
    int i;
    memset(&cost, 0, sizeof cost);
    sj6_state_init(&st, v);
    sj6_render(v, &st, y, n, &cost);
    for (i = 0; i < n; i++) {
        int32_t d = x[i] - y[i];
        int32_t a = d < 0 ? -d : d;
        if (a > peak) peak = a;
        se += (double)d * d;
    }
    free(y);
    if (ops) *ops = (double)(cost.muls + cost.adds + cost.shifts + cost.branches) / n;
    /* Measured FM-1 system clock is 360 MHz (docs/plans/2026-10-04-dual-core-and-clock.md). */
    if (cpu) *cpu = (ops ? *ops : 0) * 44100.0 / 360e6 * 100.0;
    if (peak_db) *peak_db = db((double)peak / 32768.0);
    return db(sqrt(se / n) / 32768.0);
}

static int load_wav(const char *path, int32_t **out, int *n, int *rate)
{
    FILE *f = fopen(path, "rb");
    unsigned char hdr[12], chunk[8];
    int audio = 0, ch = 0, bps = 0, sr = 0, i, count;
    unsigned char *data = 0;
    uint32_t dsz = 0;
    if (!f) return -1;
    if (fread(hdr, 1, 12, f) != 12 || memcmp(hdr, "RIFF", 4) || memcmp(hdr + 8, "WAVE", 4)) { fclose(f); return -1; }
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
            data = malloc(sz);
            if (!data || fread(data, 1, sz, f) != sz) { free(data); fclose(f); return -1; }
            dsz = sz;
            break;
        } else fseek(f, (long)sz, SEEK_CUR);
        if (sz & 1) fseek(f, 1, SEEK_CUR);
    }
    fclose(f);
    if (audio != 1 || ch != 1 || !data || (bps != 16 && bps != 24)) { free(data); return -1; }
    count = (int)(dsz / (bps / 8));
    *out = malloc((size_t)count * sizeof(int32_t));
    if (!*out) { free(data); return -1; }
    if (bps == 16) for (i = 0; i < count; i++) (*out)[i] = (int16_t)(data[i * 2] | (data[i * 2 + 1] << 8));
    else for (i = 0; i < count; i++) {
        int32_t s = data[i * 3] | (data[i * 3 + 1] << 8) | (data[i * 3 + 2] << 16);
        if (s & 0x800000) s |= ~0xffffff;
        (*out)[i] = s >> 8;
    }
    free(data);
    *n = count;
    *rate = sr;
    return 0;
}

static void write_voice(FILE *fp, const Sj6Voice *v)
{
    int p;
    fprintf(fp, "    {\"%s\", %uu, %u, %d, %d, %d, %u, %uu, {\n",
            v->name, v->loop_len, v->drift_len, v->drift_depth, v->noise_peak, v->noise_sustain, v->noise_pole, v->noise_seed);
    for (p = 0; p < SJ6_PARTIALS; p++)
        fprintf(fp, "        {%uu, %uu, %u, %u, %d, %d}%s\n",
                v->partial[p].inc, v->partial[p].phase0, v->partial[p].attack, v->partial[p].decay,
                v->partial[p].peak, v->partial[p].sustain, p + 1 == SJ6_PARTIALS ? "" : ",");
    fprintf(fp, "    }},\n");
}

static void make_synthetic(int32_t *x, int n)
{
    Sj6Voice v;
    Sj6State st;
    int p;
    memset(&v, 0, sizeof v);
    snprintf(v.name, sizeof v.name, "SYNTHETIC-C4");
    v.noise_seed = 1;
    v.noise_peak = 0;
    v.noise_sustain = 0;
    v.noise_pole = 0;
    for (p = 0; p < 5; p++) {
        v.partial[p].inc = inc_of(261.6255653005986 * (p + 1), 44100);
        v.partial[p].phase0 = (uint32_t)p * 0x08000000u;
        v.partial[p].attack = 2000;
        v.partial[p].decay = 6000;
        v.partial[p].peak = (int16_t)(14000 / (p + 1));
        v.partial[p].sustain = (int16_t)(4200 / (p + 1));
    }
    sj6_state_init(&st, &v);
    sj6_render(&v, &st, x, n, 0);
}

int main(void)
{
    const char *files[] = {
        "assets/factory-breaks/909FL.wav", "assets/factory-breaks/909GR.wav",
        "assets/909/ride.wav", "assets/909/crash.wav", "assets/909/hh.wav"
    };
    FILE *params = fopen("firmware/src/dsp/sj6_standin.inc", "w");
    int32_t *syn = malloc(16384 * sizeof(int32_t));
    Sj6Voice fitted;
    double null_db, peak_db, ops, cpu;
    int i, fail = 0;
    if (!params || !syn) return 1;
    fprintf(params, "static const Sj6Voice sj6_standins[] = {\n");
    make_synthetic(syn, 16384);
    sj6_fit(syn, 16384, 44100, -1, -1, "SYNTHETIC-C4", &fitted);
    null_db = score_null(syn, 16384, &fitted, &peak_db, &ops, &cpu);
    printf("SYNTHETIC-C4 null_rms_dBFS=%.2f null_peak_dBFS=%.2f ops/sample=%.1f cpu%%=%.3f voice_bytes=%u\n",
           null_db, peak_db, ops, cpu, (unsigned)sizeof(Sj6Voice));
    printf("fit0 inc %u phase %u atk %u dec %u peak %d sus %d\n",
           fitted.partial[0].inc, fitted.partial[0].phase0, fitted.partial[0].attack, fitted.partial[0].decay,
           fitted.partial[0].peak, fitted.partial[0].sustain);
    printf("true0 inc %u\n", inc_of(261.6255653005986, 44100));
    if (!(null_db < -40.0)) {
        printf("FAIL synthetic null is not under -40 dBFS\n");
        fail = 1;
    }
    write_voice(params, &fitted);
    free(syn);
    for (i = 0; i < 5; i++) {
        int32_t *x = 0;
        int n = 0, rate = 0;
        const char *slash, *base;
        if (load_wav(files[i], &x, &n, &rate)) { printf("FAIL load %s\n", files[i]); return 1; }
        slash = strrchr(files[i], '/');
        base = slash ? slash + 1 : files[i];
        sj6_fit(x, n, rate, -1, -1, base, &fitted);
        null_db = score_null(x, n, &fitted, &peak_db, &ops, &cpu);
        printf("STAND-IN %s n=%d null_rms_dBFS=%.2f null_peak_dBFS=%.2f ops/sample=%.1f cpu%%=%.3f\n",
               base, n, null_db, peak_db, ops, cpu);
        write_voice(params, &fitted);
        free(x);
    }
    fprintf(params, "};\n");
    fclose(params);
    printf(fail ? "FAIL resynth\n" : "PASS resynth. Private 12-record PCM NOT_RUN\n");
    return fail;
}
