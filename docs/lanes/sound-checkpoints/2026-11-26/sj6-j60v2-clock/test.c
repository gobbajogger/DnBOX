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
static void check_history(unsigned num, unsigned den, unsigned lead)
{
    unsigned max_full = 0, max_prune = 0;
    for (unsigned k = 512; k < 8192; ++k) {
        unsigned coord = k*num;
        unsigned q = coord/den;
        /* Before stepping Up, host frame ceil(coord/den) is available.
         * Down steps only after the current 96k floor frame is published. */
        unsigned newest = q + (lead && coord%den != 0);
        unsigned full = newest - (q-63) + 1;
        unsigned prune = newest - (q-55) + 1;
        if (full > max_full) max_full = full;
        if (prune > max_prune) max_prune = prune;
    }
    assert(max_full == 64 + lead);
    assert(max_prune == 56 + lead);
}

int main(void)
{
    check(147,320); check(320,147); /* 44.1 ↔ 96 kHz */
    check(1,2); check(2,1);        /* 48 ↔ 96 kHz */
    check(1,1);                    /* 96 kHz identity */
    check_history(147,320,1); check_history(320,147,0);
    check_history(1,2,1); check_history(2,1,0);
    puts("PASS: 7,505,000 exact cursor/phase steps, including rollover; ring-age bounds exact");
}
