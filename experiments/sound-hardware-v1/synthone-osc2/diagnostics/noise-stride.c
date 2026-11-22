/* SPDX-License-Identifier: GPL-3.0-only */
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "soundpipe.h"

#define RAND_MASK UINT32_C(0x7fffffff)
#define MULTIPLIER UINT32_C(1103515245)
#define INCREMENT UINT32_C(12345)

typedef struct { uint32_t multiplier, offset; } noise_jump;

static uint32_t next(uint32_t seed)
{
    return (seed * MULTIPLIER + INCREMENT) & RAND_MASK;
}

static noise_jump stride(unsigned voices)
{
    noise_jump result = {1, 0};
    while (voices--) {
        result.multiplier *= MULTIPLIER;
        result.offset = result.offset * MULTIPLIER + INCREMENT;
    }
    return result;
}

static float output(uint32_t seed, float amplitude)
{
    float unit = (float)seed * (1.f / 2147483648.f);
    return (unit * 2.f - 1.f) * amplitude;
}

int main(void)
{
    static float expected[6][257];
    sp_data source = {0};
    uint32_t owner_seed = 0;
    unsigned samples = 0, disabled_draws = 0;
    for (unsigned block = 0; block < 8192; ++block) {
        unsigned voices = block % 6 + 1;
        unsigned frames = (block * 113u) % 257u + 1;
        sp_noise voice[6];
        for (unsigned i = 0; i < voices; ++i) {
            sp_noise_init(&source, &voice[i]);
            float gain = ((block + i * 7) % 128) / 127.f;
            voice[i].amp = (block + i) % 11 ? gain * gain : 0.f;
        }
        for (unsigned f = 0; f < frames; ++f)
            for (unsigned i = 0; i < voices; ++i)
                sp_noise_compute(&source, &voice[i], NULL, &expected[i][f]);
        uint32_t first = owner_seed;
        noise_jump jump = stride(voices);
        for (unsigned i = 0; i < voices; ++i) {
            first = next(first);
            uint32_t state = first;
            for (unsigned f = 0; f < frames; ++f) {
                float actual = output(state, voice[i].amp);
                assert(!memcmp(&actual, &expected[i][f], sizeof actual));
                if (i == voices - 1 && f == frames - 1) owner_seed = state;
                state = (state * jump.multiplier + jump.offset) & RAND_MASK;
                disabled_draws += voice[i].amp == 0.f;
                ++samples;
            }
        }
        assert(owner_seed == source.rand);
    }
    printf("{\"initial_seed\":0,\"blocks\":8192,\"phase_exact_samples\":%u,"
           "\"zero_amplitude_clocked_samples\":%u,\"final_seed\":%u}\n",
           samples,disabled_draws,owner_seed);
}
