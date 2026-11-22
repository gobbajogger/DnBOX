/* SPDX-License-Identifier: GPL-3.0-only */
#include "dnbx_core.c"
#include <stdio.h>
static unsigned random_word = 0x571bc8u;
static float uniform(void)
{
    random_word ^= random_word << 13;
    random_word ^= random_word >> 17;
    random_word ^= random_word << 5;
    return (float)(random_word >> 8) * 0x1p-24f;
}
int main(void)
{
    dnbx_layer_voice voice = {0};
    dnbx_work work = {0};
    for (unsigned i = 0; i < 1000000; ++i) {
        if (i % 128 == 0) {
            voice.phase = uniform();
            voice.increment = 0.000001f + .449999f * uniform();
        }
        float width = i % 2 ? .5f : .03f + .47f * uniform();
        float sample = factory_oscillator(&voice, 1.f, width, &work);
        if (fwrite(&sample, sizeof sample, 1, stdout) != 1) return 1;
        voice.phase += voice.increment;
        if (voice.phase >= 1.f) voice.phase -= 1.f;
    }
    return fwrite(&work, sizeof work, 1, stdout) != 1;
}
