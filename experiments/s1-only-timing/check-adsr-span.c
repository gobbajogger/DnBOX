/* Compare exact samples and complete engine/arena state against the same source without sharing. */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "app/s1c_part.h"
extern void candidate_s1c_render(s1c *,float *,float *,unsigned);
float __real_s1u_env_step(s1u_env *,float,float,float,float,float,float);
static unsigned calls;
float __wrap_s1u_env_step(s1u_env *e,float g,float a,float d,float s,float r,float sr)
{ ++calls; return __real_s1u_env_step(e,g,a,d,s,r,sr); }

static s1_dry_part part, before, expected;
_Alignas(16) static uint8_t arena[S1C_ARENA_BYTES], start[S1C_ARENA_BYTES], end[S1C_ARENA_BYTES];
static float got[2][512], ref[2][512];

int main(void)
{
    const unsigned keys[]={48,51,55,60,64,69};
    const unsigned sizes[]={1,3,4,7,8,9,15,16,17,31,32,64,127,128,129,255,256,512};
    unsigned reduced=0, unchanged=0, frames=0, chords=0, spans=0;
    for(unsigned t=0;t<72;t++) {
        memset(arena,0,sizeof arena); s1_part_init(&part); s1_part_bind(&part,arena);
        s1c_set(&part.e,20,0.f); part.e.mono_prev=0;  /* explicitly start in poly */
        s1c_set(&part.e,115,t%3);
        s1c_set(&part.e,12,(t&1)?.32f:0.f);
        s1c_set(&part.e,10,(t&2)?.29f:0.f);
        s1c_set(&part.e,51,t%4); /* LDEC changes the common per-sample amplitude decay. */
        s1c_set(&part.e,19,(t&8)?.45f:0.f);
        for(unsigned k=0;k<6;k++)s1_part_note(&part,keys[k],65+9*k);
        for(unsigned i=0;i<150;i++){
            unsigned n=sizes[(i+11*t)%(sizeof sizes/sizeof *sizes)];
            if(i%9==2){s1c_set(&part.e,28,.26f+(i%6)*.1f);s1c_set(&part.e,24,.29f+(.1f*(i%7)));s1c_set(&part.e,16,400.f+51.f*i);}
            if(i==12 && (t&4))part.e.v[1].ef.y=-0.f;
            if(i==25 || i==26)s1_part_note(&part,keys[2],i==25?0:97);
            if(i==34 || i==37)s1_part_note(&part,74,i==34?103:0);
            if(i==47)for(unsigned k=0;k<6;k++)s1_part_note(&part,keys[k],0);
            if(i==78 || i==119)s1c_set(&part.e,20,(i==78)?1.f:0.f);
            if(i==84 || i==123)for(unsigned k=0;k<6;k++)s1_part_note(&part,keys[k],75);
            if(i==97)s1_part_all_off(&part);
            memcpy(&before,&part,sizeof part); memcpy(start,arena,sizeof arena);
            calls=0; s1c_render(&part.e,ref[0],ref[1],n); unsigned original=calls;
            memcpy(&expected,&part,sizeof part);memcpy(end,arena,sizeof arena);
            memcpy(&part,&before,sizeof part);memcpy(arena,start,sizeof arena);
            calls=0; candidate_s1c_render(&part.e,got[0],got[1],n);
            if(memcmp(ref[0],got[0],4*n)||memcmp(ref[1],got[1],4*n)||
               memcmp(end,arena,sizeof arena)||memcmp(&expected,&part,sizeof part)){
                fprintf(stderr,"mismatch case %u span %u frames %u (stereo/state)\n",t,i,n); return 1;
            }
            if(i==0 && original==12*n && calls==2*n)++chords;
            if(calls<original)++reduced; else if(calls==original)++unchanged;
            else {fprintf(stderr,"unexpected added calls\n");return 2;}
            ++spans;frames+=n;
        }
    }
    printf("%u exact stereo frames and complete state, %u spans, %u 12-to-2 chord starts, %u shared / %u fallback spans\n",frames,spans,chords,reduced,unchanged);
    if(chords!=72 || !reduced || !unchanged){fprintf(stderr,"expected shared ADSR path did not execute\n");return 3;}
    return 0;
}
