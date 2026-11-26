#include "fm1_time.h"
enum{N=8192,C=8};
struct item {uint32_t time,phase,hash;};
struct out {uint32_t magic;struct item row[C];};
static volatile struct out *const out=(volatile struct out *)0x1c10000;
static inline uint32_t word(float v){union{float f;uint32_t u;}x={.f=v};return x.u;}
void bench_start(void){
 const uint32_t p[C]={0,0xffffffffu,0x40000000u,0x40000000u,0,0x80000000u,0x7fffffffu,0x40000001u};
 const uint32_t inc[C]={7054092u,19173961u,11860145u,14919241u,1,1,1,0u};
 fm1_time_init();
 for(unsigned k=0;k<C;k++){
  uint32_t q=p[k],h=0x811c9dc5u,t=fm1_ticks();
  for(unsigned i=0;i<N;i++){
   q+=inc[k];
   float x=k<4 ? (k==0? d9_diode16(d9_tri(q)): d9_triangle20(q))
        : d9_triangle20(q) + d9_diode16(d9_tri(q));
   h=(h^word(x))*16777619u;
  }
  out->row[k]=(struct item){fm1_ticks()-t,q,h};
 }
 out->magic=0x53413146u;
}
