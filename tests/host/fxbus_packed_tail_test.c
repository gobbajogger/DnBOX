/* SPDX-License-Identifier: GPL-3.0-only */
#include "fxbus.h"
#include <stdio.h>

enum { N=4096, BLOCK=128 };
static fxbus_t bus;
static int16_t line[(FX_DELAY_BYTES(N)+1u)/2u];
static float zero[BLOCK], pulse[BLOCK], left[BLOCK], right[BLOCK];

int main(void)
{
    fxbus_init_packed(&bus,line,N);
    fxbus_set(&bus,FX_DL_FDBK,0);
    fxbus_set(&bus,FX_DL_LEVEL,127);
    if (!fxbus_can_stream_idle_delay(&bus,zero,zero,BLOCK)) return 1;
    pulse[0]=0.75f;
    fxbus_process_st(&bus,zero,zero,zero,pulse,left,right,BLOCK);
    if (fxbus_can_stream_idle_delay(&bus,zero,zero,BLOCK)) {
        fputs("Packed 909 delay with a pending echo was marked idle\n",stderr);
        return 1;
    }
    int heard=0;
    for (int block=0;block<256;++block) {
        fxbus_process_st(&bus,zero,zero,zero,zero,left,right,BLOCK);
        for (int i=0;i<BLOCK;++i) heard |= left[i]!=0 || right[i]!=0;
    }
    if (!heard || !fxbus_can_stream_idle_delay(&bus,zero,zero,BLOCK)) return 1;
    puts("Packed 909 idle gate passes initial silence, rejects a pending echo and reopens after the audible tail.");
    return 0;
}

