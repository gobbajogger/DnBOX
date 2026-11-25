/* SPDX-License-Identifier: GPL-3.0-only */
#pragma once
#include <stdint.h>
/* One audio block is <=256 frames. 44.1k/80 gives at most one boundary
 * per active oscillator inside a block, plus LFO3's separate delay start.
 * Keep one closed initial interval and bounded additional boundaries. */
enum { SJ6_AUX_FX_EVENTS=5, SJ6_AUX_FX_ROUTES=5 };
typedef struct { uint8_t at, code[SJ6_AUX_FX_ROUTES]; } sj6_aux_fx_event;
typedef struct sj6_aux_fx_schedule { uint8_t count; sj6_aux_fx_event event[SJ6_AUX_FX_EVENTS]; } sj6_aux_fx_schedule;

static inline int sj6_aux_fx_supported(int id)
{
    switch(id){
    case 0:return 1;
#if X0X_SJ6_NATIVE_LIMITER
    case 17:return 1;
#endif
#if (defined(X0X_HOST) && defined(X0X_SJ6_ALT3_WITNESS)) || X0X_SJ6_ALT3_NATIVE
    case 103:case 104:return 1;
#endif
#if X0X_SJ6_NATIVE_UNISON
    case 60:case 61:return 1;
#endif
#if X0X_SJ6_NATIVE_ROOM
    case 13:case 14:case 15:case 22:case 23:case 108:return 1;
#endif
#if X0X_SJ6_NATIVE_DELAY
    case 25:case 26:case 27:return 1;
#endif
    default:return 0;
    }
}
