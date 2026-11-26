#include <stdint.h>
#include "fm1_time.h"
#define OUT ((volatile uint32_t*)0x01c10000)
typedef union {uint32_t u; float f;} U;
extern unsigned measure_split(const float *,const float *,unsigned);
extern unsigned measure_transposed(const float *,const float *,unsigned);
static const float coeff[5]={.183f,.73f,.329f,.88f,.34f};
static const float settings[2][4]={{.17f,.33f,.93f,.15f},{.52f,.28f,.72f,.14f}};
void *memset(void *dst,int c,unsigned n){unsigned char *p=dst;while(n--)*p++=(unsigned char)c;return dst;}
static void measure(unsigned row,unsigned (*f)(const float*,const float*,unsigned),unsigned p){
 unsigned a=fm1_ticks();unsigned h=f(settings[p],coeff,512u);unsigned d=fm1_ticks()-a;
 OUT[row]=d;OUT[row+1]=h;
}
void bench_start(void){
 fm1_time_init();
 /* Two equal 512-frame segments. Reading four RAM dump words avoids
  * generating a 1 MiB RAM image. Both outputs have to appear. */
 measure(0,measure_split,0);
 measure(2,measure_transposed,0);
 for(;;)__asm__ volatile("" ::: "memory");
}
