/* SPDX-License-Identifier: GPL-3.0-only */
#include <assert.h>
#include <stdio.h>
#include "early_drum_mailbox.h"

int main(void)
{
    early_drum_mailbox m = {0};
    unsigned hits = 0;
    for (unsigned i = 0; i != 1000; ++i) {
        /* Scanner can publish only when project admission and legacy CQ are empty. */
        assert(!early_drum_publish(&m, 5, 127, 1, 0));
        assert(!early_drum_publish(&m, 5, 127, 0, 1));
        assert(!early_drum_pending(&m));
        assert(early_drum_publish(&m, 2, 111, 0, 0));
        assert(early_drum_pending(&m)); /* project and reset are busy before audio */
        assert(!early_drum_publish(&m, 5, 127, 0, 0)); /* caller must fall back */
        /* Full IRQ exclusion at the Panic cut retires only accepted older work. */
        early_drum_cancel(&m);
        assert(!early_drum_pending(&m));
        assert(!early_drum_take(&m));
        assert(early_drum_publish(&m, 5, 127, 0, 0)); /* a fresh strike survives */
        assert(early_drum_pending(&m));
        unsigned hit = early_drum_take(&m);
        assert(early_drum_voice(hit) == 5 && early_drum_vel(hit) == 127);
        assert(!early_drum_pending(&m));
        assert(!early_drum_take(&m)); /* audio obtains the accepted strike once */
        ++hits;
    }
    for (unsigned voice=0; voice!=11; ++voice)
        for (unsigned velocity=1; velocity!=128; ++velocity) {
            assert(early_drum_publish(&m,voice,velocity,0,0));
            /* Hold must prevent subsequent publishes but not clear prior work. */
            assert(!early_drum_publish(&m,voice,velocity,1,0));
            assert(early_drum_pending(&m));
            uint32_t word=early_drum_take(&m);
            assert(early_drum_voice(word)==voice && early_drum_vel(word)==velocity);
        }
    assert(!early_drum_publish(&m, 0, 0, 0, 0));
    assert(!early_drum_publish(&m, 0, 128, 0, 0));
    assert(!early_drum_publish(&m, 11, 127, 0, 0));
    printf("%u mailbox admission/Panic/retirement cycles pass\n", hits);
}
