/* SPDX-License-Identifier: GPL-3.0-only */
#pragma once
#include <stdint.h>
#if X0X_SJ6_NATIVE_LIMITER
enum { SJ6_LIM_THRESH, SJ6_LIM_HEADROOM, SJ6_LIM_ATTACK, SJ6_LIM_GAIN, SJ6_LIM_COUNT };
typedef struct { float threshold, inverse_knee, attack, output_gain, gain; uint8_t armed; } sj6_limiter_t;
#endif
