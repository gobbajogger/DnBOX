/* SPDX-License-Identifier: GPL-3.0-only */
#pragma once
#include <stdint.h>
#if X0X_SJ6_NATIVE_ROOM
enum { SJ6_ROOM_ENABLE, SJ6_ROOM_TYPE, SJ6_ROOM_SIZE, SJ6_ROOM_LOCUT,
       SJ6_ROOM_MIX, SJ6_ROOM_WIDEN, SJ6_ROOM_RATE, SJ6_ROOM_DEPTH, SJ6_ROOM_COUNT };
enum { SJ6_ROOM_LINE = 113, SJ6_ROOM_WIDE = 23, SJ6_ROOM_STRIDE = 8 };
typedef struct {
    int16_t ring[4][SJ6_ROOM_LINE], wide[SJ6_ROOM_WIDE];
    float lo[2], acc[2], wet[2], last[2], pan_phase;
    float hp_coeff, fb, mix, rate, depth, clock_bpm;
    uint8_t rate_code, clock_sync;
    uint8_t pos[4], wide_pos, tick, enable, type, widen, armed;
} sj6_room_t;
#endif
