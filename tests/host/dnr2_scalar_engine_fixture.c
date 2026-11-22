/* SPDX-License-Identifier: GPL-3.0-only */
#include "../../firmware/src/app/engine.c"

/* Host only: inspect the static single-producer queue and audio-owned value. */
uint32_t probe_writes(void) { return cq_w; }
uint32_t probe_pending(void) { return cq_w - cq_r; }
float probe_909_gain(void) { return mix_level[PART_909]; }
float probe_gain_for(int value) { return mix_level_from_pot(value); }
int probe_909_enqueue(uint8_t value)
{
    uint32_t before = cq_w;
    engine_set(T_MIX, PART_909, MX_LEVEL, value);
    return cq_w != before;
}
