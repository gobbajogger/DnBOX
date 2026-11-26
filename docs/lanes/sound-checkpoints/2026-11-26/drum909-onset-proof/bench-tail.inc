#include <stddef.h>
#include "fm1_time.h"
void *memcpy(void *d,const void *s,size_t n){unsigned char *a=d;const unsigned char *b=s;while(n--)*a++=*b++;return d;}
enum {N=128,C=8};
struct item {uint32_t time,phase,hash;};
struct out {uint32_t magic;struct item row[C];};
static volatile struct out *const out=(volatile struct out *)0x1c10000;
static float dry[N],right[N],rev[N],dly[N];
static drum909_t owner;
static volatile unsigned char *const receipt=(volatile unsigned char *)0x01c20000;
static inline uint32_t word(float v){union{float f;uint32_t u;}x={.f=v};return x.u;}
void bench_start(void){
  unsigned char *raw=(unsigned char *)&owner;
  for(unsigned j=0;j<sizeof owner;j++)raw[j]=0;
  drum909_init(&owner);
  fm1_time_init();
  for(unsigned c=0;c<C;c++){
   if(c==0 || c==4) {
     drum909_trigger(&owner,DR_RD,1.0f);
     drum909_trigger(&owner,DR_CH,0.75f);
     drum909_trigger(&owner,DR_BD,1.0f);
   }
   for(unsigned i=0;i<N;i++) dry[i]=right[i]=rev[i]=dly[i]=0;
   uint32_t t=fm1_ticks();
   drum909_render_st_cleared(&owner,dry,right,rev,dly,N,(1u<<DR_NUM)-1);
   t=fm1_ticks()-t;
   uint32_t h=2166136261u,p=2166136261u;
   for(unsigned i=0;i<N;i++){
     h=(h^word(dry[i]))*16777619u; h=(h^word(right[i]))*16777619u;
     h=(h^word(rev[i]))*16777619u; h=(h^word(dly[i]))*16777619u;
   }
   // exclude stream addresses (distinct link relocations), hash every other byte of the owner
   for(unsigned i=0;i<sizeof owner;i++){
      int skip=0;
      for(unsigned k=0;k<4;k++){
        unsigned char *a=(unsigned char *)&owner.smp[k].adpcm;
        if(raw+i>=a && raw+i<a+sizeof owner.smp[k].adpcm) skip=1;
      }
      if(!skip)p=(p^raw[i])*16777619u;
   }
   volatile unsigned char *dest=receipt+4+c*(sizeof owner+N*16);
   for(unsigned i=0;i<sizeof owner;i++){
      int skip=0;
      for(unsigned k=0;k<4;k++){
        unsigned char *a=(unsigned char *)&owner.smp[k].adpcm;
        if(raw+i>=a && raw+i<a+sizeof owner.smp[k].adpcm) skip=1;
      }
      dest[i]=skip?0:raw[i];
   }
   dest+=sizeof owner;
   const float *buses[4]={dry,right,rev,dly};
   for(unsigned j=0;j<4;j++)for(unsigned i=0;i<N*4;i++)dest[j*N*4+i]=((const unsigned char *)buses[j])[i];
   out->row[c]=(struct item){t,p,h};
  }
  *(volatile uint32_t *)receipt=sizeof owner;
  out->magic=0x53413146u;
}
