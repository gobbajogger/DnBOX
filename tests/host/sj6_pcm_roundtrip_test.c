/* Round-trip the public 44.1 kHz recordings through the exact residual codec.
 * The private SuperJ6 12-record asset is not in this tree and is not read. */
#include "sj6_pcm.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

static int load_wav(const char *path, int32_t **out, int *n, int *bits, int *rate, size_t *raw)
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
        } else {
            fseek(f, (long)sz, SEEK_CUR);
        }
        if (sz & 1) fseek(f, 1, SEEK_CUR);
    }
    fclose(f);
    if (audio != 1 || ch != 1 || !data || (bps != 16 && bps != 24) || sr <= 0) { free(data); return -1; }
    count = (int)(dsz / (bps / 8));
    *out = (int32_t *)malloc((size_t)count * sizeof(int32_t));
    if (!*out) { free(data); return -1; }
    if (bps == 16) {
        for (i = 0; i < count; i++) {
            int16_t s = (int16_t)(data[i * 2] | (data[i * 2 + 1] << 8));
            (*out)[i] = s;
        }
    } else {
        for (i = 0; i < count; i++) {
            int32_t s = data[i * 3] | (data[i * 3 + 1] << 8) | (data[i * 3 + 2] << 16);
            if (s & 0x800000) s |= ~0xffffff;
            (*out)[i] = s;
        }
    }
    free(data);
    *n = count;
    *bits = bps;
    *rate = sr;
    *raw = dsz;
    return 0;
}

static int roundtrip(const char *path)
{
    int32_t *x = 0, *y = 0;
    int n = 0, bits = 0, rate = 0, order = 0, dbits = 0, drate = 0, got, i, diffs = 0;
    size_t raw = 0, cap, stored;
    uint8_t *blob;
    Sj6PcmStats st;
    if (load_wav(path, &x, &n, &bits, &rate, &raw)) {
        printf("FAIL load %s\n", path);
        return 1;
    }
    cap = raw * 2 + 4096;
    blob = (uint8_t *)malloc(cap);
    y = (int32_t *)malloc((size_t)n * sizeof(int32_t));
    if (!blob || !y) { printf("FAIL alloc %s\n", path); return 1; }
    stored = sj6_pcm_encode(x, n, bits, rate, blob, cap, &order);
    if (!stored) { printf("FAIL encode %s\n", path); return 1; }
    got = sj6_pcm_decode(blob, stored, y, n, &dbits, &drate, &st);
    if (got != n || dbits != bits || drate != rate) {
        printf("FAIL decode %s got %d\n", path, got);
        return 1;
    }
    for (i = 0; i < n; i++) if (x[i] != y[i]) diffs++;
    {
        double ops = (double)(st.muls + st.adds + st.shifts + st.bit_reads + st.branches);
        double per = ops / (double)n;
        double core = 320e6;
        double load = per * (double)rate / core * 100.0;
        printf("%s samples=%d bits=%d raw=%zu stored=%zu order=%d diffs=%d "
               "ops/sample=%.2f cpu_at_44100_on_320MHz=%.3f%% ram=%zu\n",
               path, n, bits, raw, stored, order, diffs, per, load * ((double)rate / 44100.0),
               st.state_bytes);
        /* The printed CPU uses this file's rate. Also print the 44.1 kHz figure. */
        printf("  decode_ops %llu muls %llu adds %llu shifts %llu bit_reads %llu branches %llu "
               "share_of_320MHz_at_this_rate=%.4f%%\n",
               (unsigned long long)(st.muls + st.adds + st.shifts + st.bit_reads + st.branches),
               (unsigned long long)st.muls, (unsigned long long)st.adds,
               (unsigned long long)st.shifts, (unsigned long long)st.bit_reads,
               (unsigned long long)st.branches, load);
    }
    free(blob); free(y); free(x);
    return diffs ? 1 : 0;
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
    int i, fail = 0;
    for (i = 0; i < 5; i++) fail |= roundtrip(files[i]);
    if (fail) { printf("FAIL round trip\n"); return 1; }
    printf("PASS 5 public recordings, 0 differing samples\n");
    printf("NOT_RUN private SuperJ6 12-record PCM (7812906 bytes) is not in this tree\n");
    return 0;
}
