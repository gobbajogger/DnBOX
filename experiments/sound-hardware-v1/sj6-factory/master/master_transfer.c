/* SPDX-License-Identifier: GPL-3.0-only */
/* Offline host arithmetic comparison. Device code only uses binary32 and integers. */
#include "dsp/juno6.c"
#include <stdio.h>

int main(void)
{
    const unsigned exponents[] = {3, 4, 80, 124, 125, 126, 127};
    unsigned tested = 0;
    for (unsigned i = 0; i < sizeof exponents / sizeof exponents[0]; ++i) {
        for (unsigned mantissa = 0; mantissa <= 0x7fffffu; ++mantissa) {
            fm_bits_t raw = {.u=(exponents[i] << 23) + mantissa};
            if (raw.f > JUNO6_FACTORY_MASTER_MAX) break;
            const float expected = (float)((double)raw.f * .404);
            if (master_from_raw(raw.f) != expected) return 1;
            ++tested;
        }
    }
    if (master_from_raw(0.f) != 0.f || master_from_raw(-0.f) != 0.f) return 1;
    printf("normal_raw_words_compared=%u plus_zero=2\n", tested);
    return 0;
}
