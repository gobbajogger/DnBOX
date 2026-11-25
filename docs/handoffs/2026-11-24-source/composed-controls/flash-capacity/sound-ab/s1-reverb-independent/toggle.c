/* Host fixture for independent live Delay and Reverb histories. */
#include "app/s1c_part.h"
#include <assert.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
struct guarded { uint32_t front[4]; _Alignas(8) unsigned char audio[S1C_ARENA_BYTES]; uint32_t end[4]; };
static struct guarded ar1,ar2;
static s1_dry_part a,b;
static void guards(struct guarded *g){
  for(unsigned i=0;i<4;i++){assert(g->front[i]==0x71abcd92u);assert(g->end[i]==0x71abcd92u);}
}
static void initialize(s1_dry_part *p,struct guarded *g){
  for(unsigned i=0;i<4;i++)g->front[i]=g->end[i]=0x71abcd92u;
  s1_part_init(p);s1_part_bind(p,g->audio);
  assert(s1_part_program(p,113));
  s1c_set(&p->e,S1C_P_DLMIX,0.f);
  s1_part_note(p,60,108);
}
int main(int argc,char **argv){
    assert(argc==2);FILE *f=fopen(argv[1],"rb");assert(f);
    assert(!fseek(f,0,SEEK_END));long bytes=ftell(f);assert(bytes>32&&bytes<1000000);
    rewind(f);unsigned char *pack=malloc((size_t)bytes);assert(pack);
    assert(fread(pack,1,(size_t)bytes,f)==(size_t)bytes);fclose(f);
    assert(s1_part_set_pack(pack,(uint32_t)bytes));initialize(&a,&ar1);initialize(&b,&ar2);
    float al[127],ar[127],bl[127],br[127];unsigned differs=0, silent_switches=0;
    for(unsigned k=0;k<1700;k++){
        if(k==400||k==401||k==402||k==403||k==620||k==621){
            s1c_set(&a.e,S1C_P_DLON,(k&1)?0.f:1.f);silent_switches++;
        }
        if(k==700){s1c_set(&a.e,S1C_P_DLMIX,.7f);s1c_set(&a.e,S1C_P_DLON,1.f);}
        if(k==1150)s1_part_note(&a,60,0),s1_part_note(&b,60,0);
        if(k==1320){s1c_set(&a.e,S1C_P_DLON,0.f);s1c_set(&a.e,S1C_P_DLMIX,0.f);}
        unsigned n=(k&1)?127:113;
        s1_part_render(&a,al,ar,n);s1_part_render(&b,bl,br,n);
        assert(a.e.rv.fb==-2.f);assert(a.e.ddec>0);
        for(unsigned i=0;i<n;i++){
          assert(isfinite(al[i])&&isfinite(ar[i]));
          if(k<700){assert(al[i]==bl[i]);assert(ar[i]==br[i]);}
          if(k>=700&&k<1320)differs+=al[i]!=bl[i]||ar[i]!=br[i];
        }
        guards(&ar1);guards(&ar2);
    }
    assert(differs>10000);
    printf("PASS: %u zero-mix live flips were transparent; active echo altered %u stereo frames; guards and room owner remained intact\n",silent_switches,differs);
    assert(s1_part_program(&a,62));
    s1_part_render(&a,al,ar,127);
    assert(a.e.rv.fb!=-2.f);guards(&ar1);
    puts("PASS: normal program adoption restores prior delay/reverb ownership");
    free(pack);
}
