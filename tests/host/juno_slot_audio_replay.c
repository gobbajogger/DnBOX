/* SPDX-License-Identifier: GPL-3.0-only */
/* Write interleaved binary32 stereo for a deterministic compact-provider sweep. */
#include "app/original_runtime.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>

int main(void)
{
    static original_runtime_part part;
    static original_runtime_workspace scratch;
    uint8_t cell[ORIGINAL_SOUND_BYTES];
    float left[256], right[256], stereo[512];
    for (unsigned program = 0; program < ORIGINAL_JUNO_FACTORIES; ++program) {
        for (unsigned id = 0; id < 16; ++id) {
            for (unsigned position = 0; position < 128; ++position) {
                assert(original_sound_init(cell, ORIGINAL_SOUND_JUNO, program));
                assert(original_runtime_init(&part, cell, 120, NULL, NULL) == ORIGINAL_RUNTIME_OK);
                float x;
                assert(original_sound_control_value(ORIGINAL_SOUND_JUNO, id, position, &x));
                assert(original_runtime_param(&part, id, x) == ORIGINAL_RUNTIME_OK);
                for (unsigned n = 0; n < 3; ++n)
                    assert(original_runtime_key(&part, ORIGINAL_INPUT_KEYS, 43 + 11*n, 57 + 30*n) == ORIGINAL_RUNTIME_OK);
                for (unsigned block = 0; block < 3; ++block) {
                    if (block == 2)
                        assert(original_runtime_release_source(&part, ORIGINAL_INPUT_KEYS) == ORIGINAL_RUNTIME_OK);
                    assert(original_runtime_render(&part, &scratch, left, right, 256) == ORIGINAL_RUNTIME_OK);
                    for (unsigned i = 0; i < 256; ++i) {
                        assert(isfinite(left[i]) && isfinite(right[i]));
                        stereo[2*i] = left[i];
                        stereo[2*i+1] = right[i];
                    }
                    if (fwrite(stereo, sizeof stereo, 1, stdout) != 1) return 1;
                }
                assert(original_runtime_retire(&part) == ORIGINAL_RUNTIME_OK);
            }
        }
    }
    return fflush(stdout) != 0;
}
