// SPDX-License-Identifier: GPL-3.0-only
// Reuse the upstream 9W9 and production render paths, parameter keys and case buffers.
#define main original_test_main
#include "drum909_test.c"
#undef main
#include <assert.h>
static void emit(FILE *file, int voice, int vel, int parameter, int value)
{
    char label[64];
    const x0x_param_t *p = parameter >= 0 ? drum909_param(voice, parameter) : NULL;
    if (p) snprintf(label, sizeof label, "%s %s=%d", vname[voice], p->name, value);
    else snprintf(label, sizeof label, "%s default vel%d", vname[voice], vel);
    const set_t setting = {0, voice, parameter, value};
    run_case(label, &voice, 1, vel, p ? &setting : NULL, p ? 1 : 0, 0, NLEN);
    uint32_t meta[2] = {(uint32_t)strlen(label), NLEN};
    assert(fwrite(meta, sizeof meta, 1, file) == 1);
    assert(fwrite(label, 1, meta[0], file) == meta[0]);
    assert(fwrite(bufr, sizeof(float), NLEN, file) == NLEN);
    assert(fwrite(bufp, sizeof(float), NLEN, file) == NLEN);
}

int main(int argc, char **argv)
{
    if (argc != 3) return 2;
    g_dir = argv[1];
    FILE *output = fopen(argv[2], "wb");
    if (!output) return 2;
    for (int v = DR_CH; v <= DR_RD; ++v) {
        emit(output, v, 127, -1, 0);
        emit(output, v, 80, -1, 0);
        for (int i = 0; i < drum909_nparams(v); ++i) {
            const x0x_param_t *p = drum909_param(v, i);
            if (!dkeys[v][i] || !strcmp(p->name, "Pan")) continue;
            if (p->max == 127) {
                const int stops[] = {0, 64, 127};
                for (unsigned k = 0; k < 3; ++k) emit(output, v, 127, i, stops[k]);
            } else {
                for (int k = 0; k <= p->max; ++k) emit(output, v, 127, i, k);
            }
        }
    }
    return fclose(output) != 0;
}
