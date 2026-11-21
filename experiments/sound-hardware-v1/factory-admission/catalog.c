/* SPDX-License-Identifier: GPL-3.0-only */
#include "original_sound.h"
#include <stdio.h>
int main(void)
{
    unsigned count = original_sound_factories(ORIGINAL_SOUND_SYNTHONE);
    for (unsigned i = 0; i < count; ++i) {
        uint8_t cell[ORIGINAL_SOUND_BYTES];
        if (!original_sound_init(cell, ORIGINAL_SOUND_SYNTHONE, i)) return 1;
        printf("%u\t%s\n", i, original_sound_name(ORIGINAL_SOUND_SYNTHONE, i));
    }
    return 0;
}
