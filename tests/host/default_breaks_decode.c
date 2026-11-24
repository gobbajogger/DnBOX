/* SPDX-License-Identifier: GPL-3.0-only */
/* Decode zone N samples at byte OFF of a slot data file with the firmware's own ima_decode
 * (predictor 0, index 0 at sample 0) and write int16 LE PCM to stdout. */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#define X0X_IMA_TABLES_DEFINE
#include "dsp/ima_adpcm.h"
int main(int argc, char **argv)
{
    FILE *f;
    long len;
    uint8_t *b;
    uint32_t off, n, i;
    int32_t pred = 0, idx = 0;
    if (argc != 4 || !(f = fopen(argv[1], "rb"))) return 2;
    fseek(f, 0, SEEK_END); len = ftell(f); rewind(f);
    if (len <= 0 || !(b = malloc((size_t)len)) || fread(b, 1, (size_t)len, f) != (size_t)len) return 2;
    fclose(f);
    off = (uint32_t)strtoul(argv[2], 0, 10); n = (uint32_t)strtoul(argv[3], 0, 10);
    if (off + (n + 1u) / 2u > (uint32_t)len) return 3;
    for (i = 0; i < n; i++) {
        int16_t s = (int16_t)ima_decode(b + off, i, &pred, &idx);
        if (fwrite(&s, 2, 1, stdout) != 1) return 4;
    }
    return 0;
}
