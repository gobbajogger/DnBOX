/* SPDX-License-Identifier: GPL-3.0-only */
#include <inttypes.h>
#include <stdio.h>
#include "motion.h"
static lane_t lanes[NLANE];
static motion_t m;
static uint32_t rng=1234567, applied;
static uint64_t signature=UINT64_C(0xcbf29ce484222325), effects;
static uint32_t rnd(void){ rng ^= rng<<13; rng ^= rng>>17; return rng ^= rng<<5; }
static void val(uint32_t v){signature=(signature ^ v)*UINT64_C(1099511628211);}
static int base(void *ctx,int t,int v,int i){ (void)ctx; return (t*61+v*19+i*29+applied)&127; }
static void apply(void *ctx,int t,int v,int i,int x){
    (void)ctx; val(t); val(v); val(i); val(x); effects++;
}
static void restore(void *ctx,int t,int v,int i){apply(ctx,t,v,i,base(ctx,t,v,i));}
int main(void){
    int step[MOT_PARTS]={0};
    motion_init(&m,lanes); m.base=base; m.apply=apply;
    for(int trial=0;trial<180000;trial++){
        const uint32_t d=rnd(), e=rnd();
        int k=(int)(d%NLANE), part=(d>>9)%MOT_PARTS, pat=(e>>8)%8;
        applied=e;
        switch(d>>28){
        case 0: motion_release(&m); break;
        case 1: motion_req_rec(&m,k); break;
        case 2: motion_req_hold(&m,k); break;
        case 3: motion_clear(&m,k); break;
        case 4: case 5: {
            int next=motion_alloc(lanes,pat,part,e&31,(e>>5)&7,(e>>16)&31);
            if(next>=0) for(int j=0;j<NSTEPS;j++) lanes[next].val[j]=(j%13==0)?MOT_NONE:rnd()&127;
            break;
        }
        case 6: m.restore=(d&1)?restore:0; break;
        case 7: lanes[k].val[e%NSTEPS]=e>>24; break;
        case 8: case 9: {
            int s=step[part];
            motion_step(&m,part,pat,s,(s+1)%NSTEPS,NSTEPS); step[part]=(s+1)%NSTEPS; break;
        }
        default: motion_tick(&m,(e%512)+1); break;
        }
        for(int j=0;j<NLANE;j++) val((uint32_t)motion_value(&m,j));
    }
    if(effects<10000 || sizeof(lane_t)!=72) return 2;
    for(int j=0;j<NLANE;j++) for(int s=0;s<NSTEPS;s++) val(lanes[j].val[s]);
    printf("180000 actions, %" PRIu64 " writes/restores, %016" PRIx64 "\n",effects,signature);
}
