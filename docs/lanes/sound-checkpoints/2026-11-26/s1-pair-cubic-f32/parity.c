#define _POSIX_C_SOURCE 200809L
#include "baseline-delay.h"
#include "exact-delay.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
enum { N=2200960 };
static uint32_t state=928322;
static float randf(void){
 state^=state<<13;state^=state>>17;state^=state<<5;
 return (float)(int32_t)state/2147483648.f;
}
static uint64_t stamp(void){
 struct timespec t;clock_gettime(CLOCK_PROCESS_CPUTIME_ID,&t);return (uint64_t)t.tv_sec*1000000000u+t.tv_nsec;
}
typedef struct{float t,fb,l,r,m,gate;} In;
static In input[N];
static float y0[N*2],y1[N*2];
int main(void){
 for(unsigned fs=44100;fs<=48000;fs+=3900){
  for(unsigned i=0;i<N;i++){
   unsigned k=i%(fs*13u);
   float t=k<4*fs?2.5f:k<7*fs?5.f:k<9*fs?0.f:k<10*fs?.0003628117914f:2.5f+(randf()*.5f);
   input[i]=(In){t,k>=9*fs?.98f:.78f,randf()*.5f,randf()*.5f,(i&4096u)?.81f:0.f,(i&2048u)?.77f:1.f};
  }
  for(unsigned mode=0;mode<2;mode++)for(unsigned p=0;p<4;p++){
   int order=p&1;uint64_t a[2];
   for(unsigned pass=0;pass<2;pass++){
    int n=(pass^order);
    uint64_t start;
    if(!n){
      source_s1_exact_delay *s=source_s1_delay_new(fs);assert(s);start=stamp();
      for(unsigned i=0;i<N;i++){ In z=input[i]; float *y=y0+2u*i;
       if(i==fs*20u+13u)source_s1_delay_reset(s);
       if(mode)source_s1_delay_insert(s,z.t,z.fb,0.f,2048.f,.7f,z.m,z.gate,z.l,z.r,y,y+1);
       else source_s1_delay_tap(s,z.t,z.fb,z.l,z.r,y,y+1);
      }
      a[n]=stamp()-start; source_s1_delay_free(s);
    }else{
      dnbox_s1_exact_delay *s=dnbox_s1_delay_new(fs);assert(s);start=stamp();
      for(unsigned i=0;i<N;i++){ In z=input[i];float*y=y1+2u*i;
       if(i==fs*20u+13u)dnbox_s1_delay_reset(s);
       if(mode)dnbox_s1_delay_insert(s,z.t,z.fb,0.f,2048.f,.7f,z.m,z.gate,z.l,z.r,y,y+1);
       else dnbox_s1_delay_tap(s,z.t,z.fb,z.l,z.r,y,y+1);
      }
      a[n]=stamp()-start; dnbox_s1_delay_free(s);
    }
   }
   assert(!memcmp(y0,y1,sizeof y0));
   printf("Fs=%u variant=%s replay=%u signed_pairs=%u stock_ns=%llu pair_ns=%llu\n",fs,mode?"activeLP-and-fades":"tap",p,N,(unsigned long long)a[0],(unsigned long long)a[1]);
   fflush(stdout);
  }
 }
}
