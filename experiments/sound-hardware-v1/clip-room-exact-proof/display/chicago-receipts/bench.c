#define _POSIX_C_SOURCE 200809L
#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
typedef int (*lookup)(const void *,uint32_t,float *,float *);
int compact_lookup(const void *,uint32_t,float *,float *);
int original_chicago_packed_lookup(const void *,uint32_t,float *,float *);
static uint64_t now(void){struct timespec t;clock_gettime(CLOCK_MONOTONIC,&t);return UINT64_C(1000000000)*t.tv_sec+t.tv_nsec;}
int main(void){
 for(unsigned pass=0;pass<5;pass++)for(unsigned order=0;order<2;order++){
  unsigned m=order^(pass&1),sum=0u;
  lookup f=m?original_chicago_packed_lookup:compact_lookup;
  uint32_t rng=1;uint64_t t=now();
  for(unsigned k=0;k<2000000;k++){
   rng=rng*1664525u+1013904223u;
   float a,e;uint32_t x,y;
   assert(f(0,(rng>>16)%16385u,&a,&e));
   memcpy(&x,&a,4);memcpy(&y,&e,4);sum+=x+y;
  }
  uint64_t dt=now()-t;
  printf("pass %u %s %.2f ns per pair checksum %08x\n",pass,m?"fast32":"compact",dt/2000000.0,sum);
 }
}
