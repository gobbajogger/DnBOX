#include "phaser.c"
#include "fm1_time.h"
#ifndef TARGET
#include <stdio.h>
#include <time.h>
static unsigned host_tick(void){struct timespec t;clock_gettime(CLOCK_MONOTONIC,&t);return t.tv_sec*1000000000u+t.tv_nsec;}
#define tick host_tick
#else
#define tick fm1_ticks
#define AT ((volatile uint32_t *)0x01c08000)
#endif
static phaser p;
enum{RUN=512};
static float input[2][RUN],out[2][RUN];
static void benchcase(unsigned profile,unsigned repeat,unsigned*elapsed,unsigned*hash) {
 initphaser(&p,repeat&1?48000:44100);
 p.fCheckbox0=1;p.fHslider2=1275;p.fHslider6=72.f;
 for(unsigned i=0;i<RUN;i++)for(unsigned j=0;j<2;j++)
  input[j][i]=((int32_t)((i*2917u+51u+j*247u)&8191u)-4096)*(1.f/8192.f);
 unsigned a=tick();
 for(unsigned i=0;i<RUN;i++){
  float*l=input[0]+i,*r=input[1]+i,*ol=out[0]+i,*or=out[1]+i;
  FAUSTFLOAT *si[]={l,r},*so[]={ol,or};
  if(profile==1 && i%17==0){p.fHslider2=190.f+3.125f*i;p.fHslider6=34.f+.18f*i;p.fHslider0=-.015625f*i;}
  if(profile==2){p.fHslider2=190.f+3.125f*i;p.fHslider6=34.f+.18f*i;p.fHslider0=-.015625f*i;}
  if(profile==3)p.fHslider3=4.f;
  if(i<192||i>=224)computephaser(&p,1,si,so);
  else {*ol=*l;*or=*r;}
 }
 *elapsed=tick()-a;
 unsigned h=2166136261u;
 for(unsigned j=0;j<2;j++)for(unsigned i=0;i<RUN;i++){
  uint32_t u;__builtin_memcpy(&u,&out[j][i],4);h=(h^u)*16777619u;
 }
 *hash=h;
}
#ifdef TARGET
void bench(void){
 fm1_time_init();AT[0]=0x43554231;
 for(unsigned j=0;j<4;j++)for(unsigned r=0;r<2;r++){
  unsigned ticks,h;benchcase(j,r,&ticks,&h);AT[4+(j*2+r)*2]=ticks;AT[5+(j*2+r)*2]=h;
 volatile float*p=(volatile float*)0x01c20000+(j*2+r)*RUN*2;
 for(int k=0;k<2;k++)for(int i=0;i<RUN;i++)p[k*RUN+i]=out[k][i];
 }
 AT[1]=0x600d600d;for(;;)__asm__ volatile("" ::: "memory");
}
#else
int main(void){
 for(unsigned j=0;j<3;j++)for(unsigned r=0;r<32;r++){
  unsigned ticks,h;benchcase(j,r,&ticks,&h);printf("%u %u %u %u\n",j,r,ticks,h);
 }
 return 0;
}
#endif
