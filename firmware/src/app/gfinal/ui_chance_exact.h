/* SPDX-License-Identifier: GPL-3.0-only */
#ifndef UI_CHANCE_EXACT_H
#define UI_CHANCE_EXACT_H
#include <stdint.h>
/* Native BChance 0..100 only. Not a generic normalized-double replacement. */
int ui_chance_prepare_native(int value, uint8_t *prepared);
int ui_chance_exact_row(uint8_t prepared,int row,uint16_t out[26]);
#endif
