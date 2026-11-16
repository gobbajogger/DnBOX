/* Offline fitter: WAV plus optional loop points -> a Sj6Voice initializer.
 * Usage: sj6-resynth-fit input.wav [--loop-start N --loop-end M] [--name ZONE]
 * PCM is read and not written back. Do not point this at a file you intend to commit.
 */
#include "fit.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

int main(int argc, char **argv)
{
    const char *path = 0, *name = "zone";
    int loop_start = -1, loop_end = -1, i, n = 0, rate = 0, p;
    int32_t *x = 0;
    Sj6Voice v;
    for (i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "--loop-start") && i + 1 < argc) loop_start = atoi(argv[++i]);
        else if (!strcmp(argv[i], "--loop-end") && i + 1 < argc) loop_end = atoi(argv[++i]);
        else if (!strcmp(argv[i], "--name") && i + 1 < argc) name = argv[++i];
        else if (!path) path = argv[i];
        else { fprintf(stderr, "usage: sj6-resynth-fit file.wav [--loop-start N --loop-end M] [--name ZONE]\n"); return 2; }
    }
    if (!path || load_wav(path, &x, &n, &rate)) { fprintf(stderr, "could not read %s\n", path ? path : "(missing)"); return 1; }
    if (sj6_fit(x, n, rate, loop_start, loop_end, name, &v)) return 1;
    printf("/* %s %d Hz, %d frames, loop %d..%d */\n", name, rate, n, loop_start, loop_end);
    printf("{\"%s\", %uu, %u, %d, %d, %d, %u, %uu, {\n",
           v.name, v.loop_len, v.drift_len, v.drift_depth, v.noise_peak, v.noise_sustain, v.noise_pole, v.noise_seed);
    for (p = 0; p < SJ6_PARTIALS; p++)
        printf("  {%uu, %uu, %u, %u, %d, %d}%s\n", v.partial[p].inc, v.partial[p].phase0,
               v.partial[p].attack, v.partial[p].decay, v.partial[p].peak, v.partial[p].sustain,
               p + 1 == SJ6_PARTIALS ? "" : ",");
    printf("}}\n");
    free(x);
    return 0;
}
