#include <stdint.h>
#include "fm1_time.h"
#define OUT ((volatile uint32_t*)0x01c10000)
typedef union {uint32_t u;float f;} U;
static float input[512][6],output[512][6];
extern void stock(const float [512][6],float [512][6],unsigned);
extern void candidate(const float [512][6],float [512][6],unsigned);
void *memset(void *dst,int c,unsigned n){unsigned char *p=dst;while(n--)*p++=(unsigned char)c;return dst;}
static void measure(unsigned row,void (*f)(const float [512][6],float [512][6],unsigned)){
 const unsigned a=fm1_ticks();
 f(input,output,ACTIVE);
 const unsigned d=fm1_ticks()-a;
 uint32_t h=2166136261u;
 for(unsigned i=0;i<512;i++) for(unsigned v=0;v<ACTIVE;v++){U y={.f=output[i][v]};h=(h^y.u)*16777619u;}
 OUT[row]=d;OUT[row+1]=h;
}
void bench_start(void){
 volatile uint32_t rng[6]; rng[0]=0x71c5adf3u; rng[1]=0x35d2844fu; rng[2]=0x12025d69u; rng[3]=0x91d2fa8fu; rng[4]=0x13a0e358u; rng[5]=0xd213d215u;
 for(unsigned i=0;i<512;i++) for(unsigned v=0;v<6;v++){
   uint32_t r=rng[v];r^=r<<13;r^=r>>17;r^=r<<5;rng[v]=r;
   input[i][v]=(float)((int32_t)(r&65535u)-32768)*(1.f/524288.f);
 }
 fm1_time_init();
 measure(0,stock);measure(2,candidate);
 for(;;)__asm__ volatile("" ::: "memory");
}
