/* SPDX-License-Identifier: GPL-3.0-only */
#include <cassert>
#include <cmath>
#include <cstdio>
using std::abs;
#include "S1Rate.hpp"
extern "C" {
#include "s1c_front.h"
}

static s1c part;

int main()
{
    const unsigned ids[] = {15u, 47u};
    const float bpms[] = {1.f, 73.f, 120.f, 200.f};
    S1Rate original;
    unsigned free_checks = 0, division_checks = 0;
    for (unsigned id : ids) {
        s1c_init(&part, 48000.f, nullptr);
        assert(s1c_front_lfo_steps(&part) == 127u);
        float lo, hi;
        s1c_front_bounds(id, &lo, &hi);
        for (unsigned pos = 0; pos < 128; ++pos) {
            s1c_front_lfo_apply(&part, id, pos+1);
            assert(s1c_front_lfo_position(&part, id) == pos);
            const float expected = lo + std::pow((float)pos / 127.f, 2.5f) * (hi-lo);
            assert(std::fabs(part.t[id] - expected) <= expected * 2.e-5f);
            ++free_checks;
        }
        assert(s1c_front_set(&part, 139, 1.f));
        assert(s1c_front_lfo_steps(&part) == 18u);
        for (float bpm : bpms) {
            assert(s1c_front_set(&part, 64, bpm));
            const unsigned last = s1c_front_lfo_steps(&part);
            assert(last == (bpm == 200.f ? 17u : 18u));
            for (unsigned div = 0; div < 19; ++div) {
                s1c_front_lfo_apply(&part, id, 129u+div);
                const float exact = original.frequency(bpm, static_cast<AKSynthOneRate>(div));
                const float valid = s1c_front_clamp(id, exact);
                const float expected = original.nearestFrequency(valid, bpm, lo, hi).value;
                assert(part.t[id] == s1c_front_clamp(id, expected));
                assert(s1c_front_lfo_position(&part, id) == (div <= last ? div : last));
                ++division_checks;
            }
        }
    }
    std::printf("PASS original S1 editor: %u free taper round trips and %u exact phase-rate division words\n",
                free_checks, division_checks);
}
