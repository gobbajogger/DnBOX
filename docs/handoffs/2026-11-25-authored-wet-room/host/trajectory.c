/* Finite owner-local wet/delay lifecycle test. Audio and raw history never published. */
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "app/s1c_part.h"
static s1_dry_part part[2];
static _Alignas(8) unsigned char arena[2][S1C_ARENA_BYTES];
static uint32_t accum(uint32_t h,const void *p,unsigned n){
    for(const unsigned char *q=p;n--;q++) h=(h^*q)*16777619u;
    return h;
}
static void set(unsigned owner,unsigned id,float v){s1c_set(&part[owner].e,id,v);}
int main(int argc,char **argv){
    if(argc!=2)return 2;
    unsigned wet_start=!strcmp(argv[1],"wet");
    unsigned expected8=wet_start;
#ifndef TEST_WET_ROOM_EXPECT
    expected8=0;
#endif
    for(unsigned i=0;i<2;i++){
        memset(arena[i],0xa5,sizeof arena[i]);
        s1_part_init(&part[i]);s1_part_bind(&part[i],arena[i]);
        for(unsigned n=0;n<S1C_RVB_BYTES;n++) assert(arena[i][n]==0);
        assert(arena[i][S1C_RVB_BYTES]==0xa5);
        set(i,S1C_P_RVON,1);set(i,S1C_P_DLON,0);set(i,S1C_P_RVMIX,wet_start?.75f:0.f);
        assert(s1_part_note(&part[i],52,109));
    }
    printf("profile %s attach-zero-verified %u bytes, immediate next byte untouched\n",argv[1],(unsigned)S1C_RVB_BYTES);
    /* Both have precisely same upstream signal and clock. Toggling room mix
     * or enable only on A must not affect its room-history vs shadow B. */
    uint32_t pcm=2166136261u;
    for(unsigned n=0;n<1024;n++){
        if(n==128){set(0,S1C_P_RVMIX,.75f);if(!wet_start)set(1,S1C_P_RVMIX,.75f);}
        if(n==256){for(unsigned i=0;i<2;i++)s1c_set_delay_gate(&part[i].e,1.f,1);}
        if(n==320){for(unsigned i=0;i<2;i++)s1c_set_delay_gate(&part[i].e,0.f,1);}
        if(n==384)set(0,S1C_P_RVMIX,0.f);
        if(n==640)set(0,S1C_P_RVMIX,.75f);
        if(n==768)set(0,S1C_P_RVON,0.f);
        if(n==896)set(0,S1C_P_RVON,1.f);
        if(n==960)for(unsigned i=0;i<2;i++)s1_part_note(&part[i],52,0);
        float l[2][64],r[2][64];
        for(unsigned i=0;i<2;i++){
            s1_part_render(&part[i],l[i],r[i],64);
            assert((part[i].e.rv.fb==-2.f)==expected8);
        }
        for(unsigned f=0;f<64;f++)assert(isfinite(l[0][f])&&isfinite(r[0][f]));
        assert(!memcmp(arena[0],arena[1],S1C_RVB_BYTES));
        assert(part[0].e.rv.m!=part[1].e.rv.m);
        pcm=accum(accum(pcm,l[0],sizeof l[0]),r[0],sizeof r[0]);
        if(n==0||n==127||n==128||n==256||n==320||n==384||n==639||n==640||n==768||n==896||n==960||n==1023){
            printf("frame=%u A[w=%.2f,rv=%.0f,dl=%.0f] shadow[w=%.2f,rv=%.0f,dl=%.0f] mode=%u history=%08x pcm=%08x\n",
                   64*n,part[0].e.t[S1C_P_RVMIX],part[0].e.t[S1C_P_RVON],part[0].e.t[S1C_P_DLON],
                   part[1].e.t[S1C_P_RVMIX],part[1].e.t[S1C_P_RVON],part[1].e.t[S1C_P_DLON],expected8?8:4,
                   accum(2166136261u,arena[0],S1C_RVB_BYTES),pcm);
        }
    }
    return 0;
}
