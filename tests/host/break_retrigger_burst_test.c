/* SPDX-License-Identifier: GPL-3.0-only */
/* Host callback proxy, NOT native deadline proof. Four nonzero retrigger
 * probabilities, all six native slice lengths, and real step+render bursts. */
#define _POSIX_C_SOURCE 200809L
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include "firmware/src/dsp/breaks.h"
static uint64_t now(void){struct timespec t;clock_gettime(CLOCK_MONOTONIC,&t);return (uint64_t)t.tv_sec*1000000000u+t.tv_nsec;}
static int compare(const void*a,const void*b){uint64_t x=*(const uint64_t*)a,y=*(const uint64_t*)b;return (x>y)-(x<y);}
int main(void){
 enum{N=4096,BLOCK=64};static uint8_t audio[4096];static uint64_t times[N];
 for(unsigned i=0;i<sizeof audio;i++)audio[i]=(uint8_t)(i*37u+i/17u);
 for(unsigned length=0;length<6;length++)for(unsigned pi=0;pi<3;pi++){
  unsigned pot=pi==0?1:pi==1?50:99;
  breaks_t *b=calloc(1,sizeof *b);if(!b)return 3;
  brk_loop_t loop={.adpcm=audio,.nsamples=8192,.rate=22050,.bars=2};
  breaks_init(b);breaks_set_loop(b,0,&loop);if(!breaks_bank(b,0)->valid)return 4;
  breaks_set(b,BRK_LEVEL,127);breaks_set(b,BRK_ALEN,length);
  for(unsigned p=0;p<4;p++)breaks_set(b,BRK_R2+p,pot);
  breaks_step(b,0,0,64.f,1);
  uint64_t hash=1469598103934665603ull,sum=0;float out[BLOCK];
  for(unsigned j=0;j<N;j++){
   uint64_t start=now();
   breaks_step(b,(j+1)&15,(j+1)/16,64.f,1);breaks_render(b,out,BLOCK);
   times[j]=now()-start;sum+=times[j];
   for(unsigned i=0;i<BLOCK;i++){uint32_t u;if(!isfinite(out[i]))return 5;memcpy(&u,out+i,4);hash=(hash^u)*1099511628211ull;}
   hash=(hash^b->sub_div)*1099511628211ull;
  }
  qsort(times,N,sizeof times[0],compare);
  printf("length=%u pot=%u hash=%016llx triggers=%u p99_ns=%llu max_ns=%llu sum_ns=%llu blocks=%u\n",length,pot,(unsigned long long)hash,b->trigger_count,(unsigned long long)times[N*99/100],(unsigned long long)times[N-1],(unsigned long long)sum,N);
  free(b);
 }
 return 0;
}
