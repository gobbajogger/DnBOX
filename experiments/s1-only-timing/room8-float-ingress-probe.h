/* Experimental shared stereo early-reflection ingress history. Four delayed
 * reads per channel from one packed 4-bit block-floating ring per channel.
 * No heap, bounded indexing; fixed nominal source geometry, not exact jitter. */
#ifndef ROOM8_TAPS_H
#define ROOM8_TAPS_H
#include "room8_tapped_trim.h"
#define TAP_LEN 1856u
#define TAP_BLOCK 32u
/* nominal RevSC delay (44.1 kHz cells) / 2 - compact loop delay (wet cells),
 * plus initial seeded variation, calibrated only to source geometry. */
static const uint16_t tap_age[8]={1087,1231,1438,1587,1754,1839,968,853};
typedef struct { float hist[2][TAP_LEN]; unsigned w; } room8_taps;
static void taps_reset(room8_taps *t){memset(t,0,sizeof *t);}
static void taps_tick(room8_taps*t,float l,float r,float x[8]){
    unsigned w=t->w;
    for(unsigned i=0;i<8;i++){
        unsigned p=w+TAP_LEN-tap_age[i]; if(p>=TAP_LEN)p-=TAP_LEN;
        x[i]=t->hist[i&1u][p];
    }
    t->hist[0][w]=l; t->hist[1][w]=r;
    t->w=w+1==TAP_LEN?0:w+1;
}
#endif
