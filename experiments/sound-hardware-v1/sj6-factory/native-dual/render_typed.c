/* SPDX-License-Identifier: GPL-3.0-only */
/* Deterministic Float32 stereo corpus for comparing prepared renderer variants. */
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include "juno6.h"

int main(int argc, char **argv)
{
    assert(argc == 2);
    FILE *out = fopen(argv[1], "wb");
    assert(out);
    for (unsigned l = 0; l < 2; ++l) {
        for (unsigned stop = 0; stop < 4; ++stop) {
            for (unsigned gate = 0; gate < 2; ++gate) {
                for (unsigned amount = 0; amount < 3; ++amount) {
                    juno6_t s;
                    juno6_init(&s, 44100);
                    for (unsigned p = 0; p < 17; ++p)
                        assert(juno6_set(&s, p, ((p * 7 + stop * 5) % 17) / 16.f));
                    const juno6_oscillator_edit wave = {stop / 3.f, amount * .5f, (float)gate};
                    juno6_apply_pre_note_wave(&s, l, &wave);
                    dnbx_layer_params params;
                    assert(dnbx_get_layer(&s.core, l, &params) == DNBX_OK);
                    assert(params.oscillator_mode == DNBX_OSC_SUPERJ6);
                    assert(juno6_note_on(&s, 60, 100));
                    assert(juno6_note_on(&s, 67, 90));
                    for (unsigned block = 0; block < 12; ++block) {
                        float left[128], right[128];
                        if (block == 6) juno6_note_off(&s, 60);
                        if (block == 9) juno6_note_off(&s, 67);
                        assert(juno6_render_stereo(&s, left, right, 128));
                        for (unsigned i = 0; i < 128; ++i) {
                            assert(isfinite(left[i]) && isfinite(right[i]));
                            assert(fwrite(&left[i], 4, 1, out) == 1);
                            assert(fwrite(&right[i], 4, 1, out) == 1);
                        }
                    }
                }
            }
        }
    }
    assert(fclose(out) == 0);
}
