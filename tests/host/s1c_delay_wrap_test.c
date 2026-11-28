/* SPDX-License-Identifier: GPL-3.0-only */
/* Compare the actual device delay read with the original repeated-wrap rule. */
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "dsp/s1c/s1c_config.h"
#include "dsp/s1c/s1c.c"

static uint32_t random_bits = 0x741e241b;
static uint32_t next_bits(void)
{
    random_bits ^= random_bits << 13;
    random_bits ^= random_bits >> 17;
    random_bits ^= random_bits << 5;
    return random_bits;
}

static float reference_read(const s1c_dline *d, float delay)
{
    float pos = (float)d->w - delay;
    while (pos < 0.0f) pos += (float)d->len;
    int i = (int)pos, j = i + 1 == d->len ? 0 : i + 1;
    float f = pos - (float)i;
    const uint16_t *samples = (const uint16_t *)d->buf;
    return h16_get(samples[i]) * (1.0f - f) + h16_get(samples[j]) * f;
}

int main(void)
{
    uint16_t samples[1440];
    unsigned checks = 0;
    for (unsigned i = 0; i < sizeof samples / sizeof samples[0]; i++)
        samples[i] = (uint16_t)(next_bits() % 0x7c00);
    for (int len = 16; len <= 1440; len += 16) {
        s1c_dline d = {.buf = (void *)samples, .len = len};
        for (int k = 0; k < 3200; k++) {
            d.w = (int)(next_bits() % (unsigned)len);
            float delay = (float)(next_bits() % 3145728) * 0.125f;
            if ((k & 15) == 0) delay = (float)(d.w + len * (k % (262144 / len)));
            if ((k & 15) == 1) delay = (float)d.w;
            float actual = xl_read(&d, delay), want = reference_read(&d, delay);
            if (memcmp(&actual, &want, sizeof actual)) {
                fprintf(stderr, "delay %a with cursor %d, len %d: %a != %a\n",
                        delay, d.w, len, actual, want);
                return 1;
            }
            checks++;
        }
    }
    printf("Synth One delay wrap: %u bit-exact reads\n", checks);
    return 0;
}
