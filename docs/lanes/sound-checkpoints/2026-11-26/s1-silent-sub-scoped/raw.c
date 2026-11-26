#include SRC
#ifdef TARGET
#include "fm1_time.h"
#define AT ((volatile uint32_t*)0x01c08000)
#define O ((volatile uint32_t*)0x01c20000)
#else
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#endif
enum{N=512};
static s1c s;
static float out[N];
static uint32_t hashes[6];
static inline void controls(unsigned k){
    memset(&s,0,sizeof s);s.sr=44100;
    s.t[SUBV]=k==2?.32f:k==5?-.0f:0.f;
    s.t[SUBSQ]=k==4||k==5?1.f:0.f;
    s.t[FMV]=k>=2?.17f:0; s.t[FMA]=.6f;
    s.t[LPITCH]=2.f;s.t[LDEC]=1.f;s.t[LCUT]=1.f;s.t[LRES]=2.f;
    s.t[AD]=.45f;s.t[AA]=.010f;s.t[AS]=.63f;s.t[AR]=.25f;
    s.t[FA]=.03f;s.t[FD]=.28f;s.t[FS]=.61f;s.t[FR]=.28f;
    s.t[DETM]=1.f;s.t[DET2]=.218f;s.t[CUT]=2000.f;s.t[RES]=.38f;
    s.t[FMIX]=.8f;s.t[BAL]=.43f;s.t[VOL1]=k==4?0:.72f;s.t[VOL2]=k==4?0:.57f;
    s.t[I1]=.28f;s.t[I2]=.48f;s.t[SUBOCT]=1.f;
    s.t[FTYPE]=k==3?2:0; s.t[FENV]=k==3?.65f:0.f; s.t[LFENV]=k==3?1.f:0.f; s.t[L1A]=k==3?.71f:0.f;
    for (unsigned i=0;i<sizeof s1c_smid;i++)s.c[i]=s.t[s1c_smid[i]];
    for (unsigned i=0;i<(k==1?1:S1C_VOICES);i++)
      voice_start(&s.v[i],48+7*i,86+i*4,130.8128f*(1.f+.41f*i));
}
static uint32_t hbytes(uint32_t h,const void*p,unsigned n){const unsigned char*b=p;for(unsigned i=0;i<n;i++)h=(h^b[i])*16777619u;return h;}
static void run(unsigned k,unsigned long iterations){
 uint32_t h=2166136261u;
 for(unsigned long i=0;i<iterations;i++){
    s.lfo[0].o10=.72f+.25f*(float)(i&31u)*(1.f/31.f);
    s.lfo[1].o01=.8f-.4f*(float)(i&63u)*(1.f/63.f);
    if(k==3){
      if((i&63u)==8u) s1c_set(&s,SUBV,.5f);
      if((i&63u)==32u) s1c_set(&s,SUBV,0.f);
      const unsigned id=s1c_smap[SUBV];s.c[id]=.1f*s.t[SUBV]+.9f*s.c[id];
    }
    if((i&127u)==72u&&k>=2){s.v[0].gate=0.f;}
    if((i&127u)==120u&&k>=2){s.v[0].gate=1.f;}
    float y=0.f;
    for(unsigned j=0;j<(k==1?1:S1C_VOICES);j++)y+=voice_run_kr(&s,&s.v[j]);
    out[i&(N-1)]=y;
#ifndef TARGET
    if(fwrite(&y,sizeof y,1,stdout)!=1 || fwrite(s.v,sizeof s.v,1,stdout)!=1)abort();
    h=hbytes(h,&y,sizeof y);
    for(unsigned j=0;j<S1C_VOICES;j++)h=hbytes(h,&s.v[j],sizeof(s1c_voice));
#endif
 }
 hashes[k]=hbytes(h,s.v,sizeof s.v);
}
#ifdef TARGET
void bench(void){
 fm1_time_init();AT[0]=0x43554231;
 for(unsigned p=0;p<6;p++){
  controls(p);
  unsigned t=fm1_ticks();run(p,N);AT[4+p]=fm1_ticks()-t;
  AT[10+p]=hashes[p];for(unsigned i=0;i<N;i++)O[p*N+i]=((s1c_fb){.f=out[i]}).u;
 }
 AT[1]=0x600d600d;for(;;)__asm__ volatile("" ::: "memory");
}
#else
int main(void){for(unsigned i=0;i<6;i++){controls(i);run(i,131072);fprintf(stderr,"%u %08x\n",i,hashes[i]);}return 0;}
#endif
