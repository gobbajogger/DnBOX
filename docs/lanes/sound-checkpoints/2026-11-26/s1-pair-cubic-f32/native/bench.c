#include "exact-delay.c"
#include "fm1_time.h"
#include <stdint.h>
#define AT ((volatile uint32_t *)0x01c08000)
enum { N=1024, RUN=512 };
static float tap[4][N];
static float input[RUN][2],ctl[RUN][2],output[RUN][2];
static sp_vdelay lane0[4];
static sp_data sp;
static dnbox_s1_exact_delay d;
static volatile unsigned iteration,kind;
static void seed(unsigned p,unsigned profile){
 sp.sr=44100;d.sp=&sp;
 for(unsigned j=0;j<4;j++){d.line[j]=&lane0[j];lane0[j].buf.ptr=tap[j];
  lane0[j].left=p;lane0[j].sr=44100;lane0[j].maxdel=(float)N/44100.f;lane0[j].prev=(float)j*.001f;
  for(unsigned i=0;i<N;i++)tap[j][i]=((int32_t)((3491u*i+509u*j+17u)&8191u)-4096)*(1.f/8192.f);}
 for(unsigned i=0;i<RUN;i++){
  input[i][0]=((int32_t)((i*2917u+51u)&8191u)-4096)*(1.f/8192.f);input[i][1]=-input[i][0]*.53f;
  ctl[i][0]=profile==0?.00513f:profile==1?.001f+(float)((i*19)&255)*(1.f/48000.f):0.f;
  ctl[i][1]=i&128u?.98f:.4f;
 }
}
#ifndef PAIR_CUBIC
static __attribute__((noinline)) void stock(float t,float fb,float l,float r,float *left,float*right){
 for(unsigned i=0;i<4;i++){d.line[i]->del=i<2?t*2.f:t;d.line[i]->feedback=fb;}
 float o,fill;
 sp_vdelay_compute(&sp,d.line[0],&l,left);sp_vdelay_compute(&sp,d.line[1],&r,&o);
 sp_vdelay_compute(&sp,d.line[2],&r,&fill);sp_vdelay_compute(&sp,d.line[3],&o,right);
 *right+=fill;
}
#endif
void bench(void){
 fm1_time_init();AT[0]=0x43554231;
 for(unsigned j=0;j<3;j++)for(unsigned r=0;r<4;r++){
  seed(r&1?1017:441,j);kind=j;iteration=r;
  unsigned a=fm1_ticks();
  for(unsigned i=0;i<RUN;i++){
#ifdef PAIR_CUBIC
   dnbox_s1_delay_tap(&d,ctl[i][0],ctl[i][1],input[i][0],input[i][1],output[i],output[i]+1);
#else
   stock(ctl[i][0],ctl[i][1],input[i][0],input[i][1],output[i],output[i]+1);
#endif
  }
  unsigned b=fm1_ticks(),h=2166136261u;
  for(unsigned i=0;i<RUN;i++)for(unsigned c=0;c<2;c++){uint32_t u;__builtin_memcpy(&u,output[i]+c,4);h=(h^u)*16777619u;}
  AT[4+(j*4+r)*2]=b-a;AT[5+(j*4+r)*2]=h;
 }
 AT[1]=0x600d600d;for(;;)__asm__ volatile("" ::: "memory");
}
