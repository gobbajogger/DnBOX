/* SPDX-License-Identifier: GPL-3.0-only */
#ifndef ORIGINAL_BACKEND_PITCH_H
#define ORIGINAL_BACKEND_PITCH_H

#include "../../../experiments/original_synth/motion_tables_private.h"

/* A standalone backend owns its factory table. Linking both keeps one copy
 * of the pinned 44.1 kHz data, selected when the backend initializes. */
__attribute__((weak)) const int32_t *original_backend_pitch44100(void)
{
    return original_pitch44100;
}

#endif
