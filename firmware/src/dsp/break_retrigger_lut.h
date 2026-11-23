/* SPDX-License-Identifier: GPL-3.0-only */
/* Experimental exact original law. Deadline qualification required. */
#ifndef X0X_BREAK_RETRIGGER_LUT_H
#define X0X_BREAK_RETRIGGER_LUT_H
#include "fastmath.h"
static inline float brk_retrigger_probability(unsigned length,unsigned pot)
{
    static const uint8_t ticks[6]={3,6,12,24,48,96};
    if(pot>=100)return 1.0f;
    float pbar=(float)pot*0.01f,inv=(float)ticks[length]*(1.0f/96.0f);
    return 1.0f-fm_powf(1.0f-pbar,inv);
}
#endif
