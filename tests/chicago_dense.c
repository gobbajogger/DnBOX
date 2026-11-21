/* SPDX-License-Identifier: GPL-3.0-only */
#include <assert.h>
#include <stdint.h>
#include <string.h>
#include "../experiments/original_synth/chicago_packed.h"

extern int chicago_raw_bits(uint32_t index, uint32_t *a, uint32_t *e);

int main(void)
{
    /* Cross every restart boundary in both directions, including the last
     * single-entry block. Compare the actual public reader against the
     * independently encoded original source used by the shipping tests. */
    for (uint32_t i = 0; i <= 16384; ++i) {
        uint32_t j = i & 1u ? 16384u - i / 2u : i / 2u;
        float a, e;
        uint32_t raw_a, raw_e;
        assert(chicago_raw_bits(j, &raw_a, &raw_e));
        assert(original_chicago_packed_lookup(NULL, j, &a, &e));
        assert(!memcmp(&a, &raw_a, sizeof a) && !memcmp(&e, &raw_e, sizeof e));
    }
    float a, e;
    assert(!original_chicago_packed_lookup(NULL, 16385, &a, &e));
    assert(!original_chicago_packed_lookup(NULL, UINT32_MAX, &a, &e));
    assert(!original_chicago_packed_lookup(NULL, 0, &a, &a));
    return 0;
}
