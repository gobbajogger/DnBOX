#include <stdint.h>
#include "fm1_time.h"
extern uint32_t prepared(float);
extern float tick(uint32_t*,uint32_t,float,int);
enum{N=8192,C=8};
struct item {uint32_t time,phase,hash;};
struct out {uint32_t magic;struct item row[C];};
static volatile struct out *const out=(volatile struct out *)0x1c10000;
void bench_start(void){
 const float hz[C]={96.f,440.f,880.f,440.f,440.f,440.f,12000.f,261.6256f};
 const float wt[C]={0.f,1.f/3.f,2.f/3.f,1.f,0.f,0.f,.55f,1.375f};
 fm1_time_init();
 for(unsigned k=0;k<C;k++){
  uint32_t p=0x793aef,plan=prepared(hz[k]),hash=0x75e4caed;
  uint32_t start=fm1_ticks();
  for(unsigned i=0;i<N;i++){
   if(k>=4&&k<=5&&!(i&7u)) plan=prepared(hz[k]);
   const float x=(k==4||k==5)?(float)(i&1023u)*(1.f/1024.f):wt[k];
   union{float f;uint32_t u;} a={.f=tick(&p,plan,x,k!=5)};
   hash=(hash^a.u)*16777619u;
  }
  out->row[k]=(struct item){fm1_ticks()-start,p,hash};
 }
 out->magic=0x53413146u;
}
