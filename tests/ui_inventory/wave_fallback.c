/* SPDX-License-Identifier: GPL-3.0-only */
/* Test entry to the alternate wave glyph. A projection success supplies
 * draw_original_wave; only a rejection can select the alternate painter. */
#include <assert.h>
#include <stdio.h>
#include "ui_original_controls.h"
#include "ui_original_wave.h"

int main(void)
{
    unsigned factories = 0, previews = 0;
    for (unsigned kind = ORIGINAL_SOUND_CHICAGO; kind <= ORIGINAL_SOUND_MIAMI; ++kind) {
        uint32_t mask = kind == ORIGINAL_SOUND_CHICAGO ? UI_ORIGINAL_CHICAGO_WAVE_MASK
                                                      : UI_ORIGINAL_MIAMI_WAVE_MASK;
        for (unsigned f = 0; f < original_sound_factories(kind); ++f) {
            uint8_t cell[ORIGINAL_SOUND_BYTES], preview[22] = {0};
            ui_original_wave_cache cache = {0};
            assert(original_sound_init(cell, kind, f));
            assert(ui_original_wave_project(&cache, cell, preview, 0));
            ++factories;
            for (unsigned i = 0; i < original_sound_count(kind); ++i) {
                if (!(mask & (1u << i)) || original_sound_is_performance(kind,i)) continue;
                for (unsigned v = 0; v <= original_sound_param(kind,i)->max; ++v) {
                    preview[i] = (uint8_t)v;
                    assert(ui_original_wave_project(&cache, cell, preview, 1u << i));
                    ++previews;
                }
                preview[i] = 0;
            }
            /* Control: an out-of-range preview reaches the real failure path. */
            preview[0] = 255;
            assert(!ui_original_wave_project(&cache, cell, preview, 1));
        }
    }
    printf("%u %u\n", factories, previews);
    return 0;
}
