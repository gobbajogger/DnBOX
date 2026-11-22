/* SPDX-License-Identifier: GPL-3.0-only */
#include <stdio.h>
#include <stdlib.h>
#include "juno6.h"

enum { FRAMES = 4096 };

int main(int argc, char **argv)
{
    static float samples[2][FRAMES];
    juno6_chorus_t chorus;
    if (argc != 2 || fread(samples, sizeof samples, 1, stdin) != 1)
        return 2;
    juno6_chorus_init(&chorus, 48000.0f);
    if (argv[1][0] == '1')
        juno6_chorus_process(&chorus, samples[0], samples[1], FRAMES);
    else if (argv[1][0] != '0')
        return 2;
    return fwrite(samples, sizeof samples, 1, stdout) == 1 ? 0 : 1;
}
