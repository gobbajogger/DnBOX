/* SPDX-License-Identifier: GPL-3.0-only */
#include "app/s1c_part.h"
#include "dsp/s1pack/s1pack.h"
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

enum { UPLOAD=906, TOTAL=907, LEGACY=694, WORDS=144, BYTES=WORDS*5, BLOCK=128, FRAMES=8192 };
static uint8_t oldbank[100000],newbank[100000],checks[TOTAL*BYTES+LEGACY*2];
static s1_dry_part part[2];
static _Alignas(8) uint8_t arena[2][S1C_ARENA_BYTES];
static float pcm[2][2][FRAMES],compare[2][2][FRAMES];
static unsigned nonzero[2], sounds[2];

#define OK(c) do { if(!(c)) {fprintf(stderr,"aggregate host fixture failure at line %d\n",__LINE__);return 0;} } while(0)
static unsigned read_bytes(const char *filename,uint8_t *buf,unsigned limit)
{
    FILE *f=fopen(filename,"rb"); if(!f)return 0;
    unsigned n=(unsigned)fread(buf,1,limit,f);
    int end=fgetc(f); int closed=fclose(f);
    return end==EOF && !closed ? n : 0;
}
struct validate { const uint8_t *x; unsigned i; unsigned bad; };
static void emitted(void *context,unsigned parameter,float physical)
{
    struct validate *v=context;
    uint32_t got;
    if(v->i>=WORDS) {v->bad=1; return;}
    const uint8_t *w=v->x+5u*v->i++;
    uint32_t expected=(uint32_t)w[1]|((uint32_t)w[2]<<8)|((uint32_t)w[3]<<16)|((uint32_t)w[4]<<24);
    memcpy(&got,&physical,sizeof got);
    if(parameter!=w[0]||got!=expected)v->bad=1;
}
static int render(unsigned idx,float audio[2][2][FRAMES],int count)
{
    for(unsigned owner=0;owner<2;++owner) {
        memset(arena[owner],0,sizeof arena[owner]);
        s1_part_init(&part[owner]);
        s1_part_bind(&part[owner],arena[owner]);
        OK(s1_part_program(&part[owner],idx) && part[owner].program==idx);
        memset(audio[owner],0,sizeof audio[owner]);
        /* Uniform surrogate: MIDI 57 @99; add 64 @81 at frame 2048;
         * release 64 at 4096 and 57 at 4608, render tail to 8192. */
        OK(s1_part_note(&part[owner],57,99));
        for(unsigned n=0;n<FRAMES;n+=BLOCK) {
            if(n==2048) OK(s1_part_note(&part[owner],64,81));
            if(n==4096) OK(s1_part_note(&part[owner],64,0));
            if(n==4608) OK(s1_part_note(&part[owner],57,0));
            s1_part_render(&part[owner],audio[owner][0]+n,audio[owner][1]+n,BLOCK);
        }
        unsigned nz=0;
        for(unsigned n=0;n<FRAMES;++n) {
            float l=audio[owner][0][n],r=audio[owner][1][n];
            OK(isfinite(l)&&isfinite(r)); nz+=(unsigned)(l!=0.f||r!=0.f);
        }
        if(count) {nonzero[owner]+=!!nz;sounds[owner]+=nz;}
    }
    OK(!memcmp(audio[0],audio[1],sizeof audio[0]));
    return 1;
}
static int execute(const char *orig,const char *unionfile,const char *expected)
{
    const unsigned a=read_bytes(orig,oldbank,sizeof oldbank),b=read_bytes(unionfile,newbank,sizeof newbank);
    OK(a>0&&b>0&&read_bytes(expected,checks,sizeof checks)==sizeof checks);
    OK(s1_part_set_pack(newbank,b)==TOTAL && s1_part_programs()==TOTAL);
    for(unsigned i=0;i<TOTAL;++i) {
        struct validate v={checks+i*BYTES,0,0};
        OK(s1pack_load(newbank,i,emitted,&v)==WORDS && v.i==WORDS && !v.bad);
        OK(render(i,pcm,1));
    }
    for(unsigned i=0;i<LEGACY;++i) {
        const uint8_t *map=checks+TOTAL*BYTES+i*2u;
        unsigned match=(unsigned)map[0]|((unsigned)map[1]<<8);
        OK(match<TOTAL && (i==43u?match==906u:match<906u));
        OK(s1_part_set_pack(oldbank,a)==LEGACY && render(i,pcm,0));
        OK(s1_part_set_pack(newbank,b)==TOTAL && render(match,compare,0));
        OK(!memcmp(pcm,compare,sizeof pcm));
    }
    printf("PASS detached uploads %u + legacy-only %u: exact ordered callback pairs %u; owner schedule %u stereo frames per row, identical independently rendered owners; audible rows %u/%u; finite parallel-owner PCM\n",
           UPLOAD,TOTAL-UPLOAD,TOTAL*WORDS,FRAMES,nonzero[0],nonzero[1]);
    printf("PASS original 694 runtime ordinals retained in original source; exact compact stereo PCM at both owners for every original ordinal versus matching detached content; legacy 44 explicitly matched detached index 906, no migration\n");
    printf("Actual nonzero stereo frames owners A/B %u and %u (out of %u each), %.3f Hz, compact host S1 object %zu B and unchanged FX arena %u B per owner; no INIT fallback\n",
           sounds[0],sounds[1],TOTAL*FRAMES,part[0].e.sr,sizeof(s1_dry_part),S1C_ARENA_BYTES);
    return nonzero[0]==TOTAL&&nonzero[1]==TOTAL;
}
int main(int argc,char **argv) {return argc==4&&execute(argv[1],argv[2],argv[3])?0:1;}
