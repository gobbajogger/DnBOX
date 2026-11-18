/* SPDX-License-Identifier: GPL-3.0-only */
/* Compact semantic pictures: 9x9, constant flash data; no mutable state. */
#pragma once
#include <stdint.h>
enum {
    UI_G_NONE,
    UI_G_PATTERN,
    UI_G_NEXT,
    UI_G_PREV,
    UI_G_UP,
    UI_G_DOWN,
    UI_G_PLAY,
    UI_G_STOP,
    UI_G_RECORD,
    UI_G_HOME,
    UI_G_MIX,
    UI_G_SONG,
    UI_G_GLOBE,
    UI_G_SOUND,
    UI_G_LIST,
    UI_G_SELECT,
    UI_G_CHECK,
    UI_G_LOCK,
    UI_G_KNOB,
    UI_G_FADER,
    UI_G_KEY,
    UI_G_MUTE,
    UI_G_SOLO,
    UI_G_CYCLE,
    UI_G_HOLD,
    UI_G_TARGET,
    UI_G_FX_GATE,
    UI_G_FX_REVERB,
    UI_G_FX_HIGHPASS,
    UI_G_FX_DELAY,
    UI_G_FX_LOWPASS,
    UI_G_FX_GATE8,
    UI_G_FX_GATE16,
    UI_G_FX_TRANCE,
    UI_G_FX_STUTTER,
    UI_G_FX_SHUFFLE,
    UI_G_FX_EMPTY,
    UI_G_COUNT
};
/* Exact 9-bit rows: low octets plus one packed high bit, unchanged atlas. */
static const uint8_t UI_PICTOGRAM_LOW[] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 182, 182, 0, 182, 182, 0, 182,
    182, 0, 16, 8, 4, 2, 255, 2, 4, 8, 16, 16, 32, 64, 128, 255,
    128, 64, 32, 16, 16, 56, 84, 146, 17, 16, 16, 16, 16, 16, 16, 16,
    16, 17, 146, 84, 56, 16, 64, 96, 112, 120, 124, 120, 112, 96, 64, 0,
    254, 254, 254, 254, 254, 254, 254, 0, 56, 124, 254, 255, 255, 255, 254, 124,
    56, 16, 40, 68, 130, 255, 130, 146, 146, 254, 146, 210, 151, 146, 186, 146,
    146, 146, 146, 224, 32, 224, 64, 124, 4, 15, 9, 15, 124, 146, 41, 41,
    255, 41, 41, 146, 124, 16, 48, 242, 241, 245, 241, 242, 48, 16, 0, 191,
    0, 0, 191, 0, 0, 191, 0, 199, 1, 1, 16, 56, 16, 1, 1, 199,
    0, 1, 2, 4, 8, 144, 96, 32, 0, 56, 68, 68, 68, 254, 146, 146,
    130, 254, 56, 84, 146, 17, 17, 1, 130, 68, 56, 16, 16, 16, 254, 130,
    254, 16, 16, 16, 255, 85, 85, 85, 119, 37, 37, 37, 255, 16, 48, 245,
    242, 241, 242, 245, 48, 16, 124, 130, 1, 1, 199, 69, 69, 199, 0, 120,
    133, 3, 7, 0, 193, 129, 66, 60, 16, 16, 84, 56, 16, 0, 254, 130,
    254, 16, 16, 0, 0, 255, 0, 0, 16, 16, 0, 128, 64, 32, 16, 8,
    4, 2, 255, 0, 80, 72, 36, 36, 36, 72, 80, 0, 7, 8, 16, 16,
    32, 64, 64, 128, 0, 0, 128, 132, 198, 199, 198, 132, 128, 0, 192, 32,
    16, 16, 8, 4, 4, 2, 1, 224, 160, 160, 160, 191, 0, 224, 160, 191,
    238, 170, 170, 170, 187, 0, 238, 170, 187, 128, 128, 3, 3, 0, 216, 216,
    3, 3, 182, 36, 36, 182, 0, 182, 36, 36, 182, 144, 144, 0, 200, 200,
    0, 100, 100, 0, 255, 3, 5, 9, 17, 33, 65, 129, 255,
};
static const uint8_t UI_PICTOGRAM_HIGH[] = {
    0, 182, 65, 128, 0, 1, 2, 0, 0, 56, 32, 8, 56, 192, 7, 128,
    164, 227, 16, 0, 224, 0, 240, 31, 0, 63, 126, 0, 32, 252, 255, 15,
    240, 127, 0, 0, 16, 7, 188, 31, 240, 31,
};
static uint16_t ui_pictogram_row(unsigned glyph,unsigned row)
{
    unsigned i=glyph*9+row;
    return UI_PICTOGRAM_LOW[i] | ((unsigned)((UI_PICTOGRAM_HIGH[i>>3]>>(i&7))&1u)<<8);
}

