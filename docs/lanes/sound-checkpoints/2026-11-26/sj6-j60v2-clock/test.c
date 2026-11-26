#include "phase_clock.h"
#include <assert.h>
#include <stdio.h>

static void check(unsigned num, unsigned den)
{
    sj6_phase_clock c = sj6_phase_init(num, den);
    for (uint64_t frame = 0; frame < 1500000; ++frame) {
        uint64_t k = frame * num;
        assert(c.input_index == k / den);
        assert(c.residual == k % den);
        uint64_t p = (k % den) * 256;
        assert(c.phase == p / den);
        assert(c.subphase == p % den);
        sj6_phase_next(&c);
    }
    /* Counter rollover must not disturb either fractional phase or ring index. */
    c = sj6_phase_init(num, den);
    c.input_index = UINT32_MAX - 20;
    for (uint64_t frame = 0; frame < 1000; ++frame) {
        uint64_t k = frame * num;
        assert(c.input_index == (uint32_t)(UINT32_MAX - 20 + k / den));
        assert(c.phase == ((k % den) * 256) / den);
        assert(c.subphase == ((k % den) * 256) % den);
        sj6_phase_next(&c);
    }
}
int main(void)
{
    check(147,320); check(320,147); /* 44.1 ↔ 96 kHz */
    check(1,2); check(2,1);        /* 48 ↔ 96 kHz */
    check(1,1);                    /* 96 kHz identity */
    puts("PASS: 7,505,000 exact cursor/phase steps, including rollover");
}
