#include <stdint.h>
#include <stdio.h>
#include <string.h>
#ifdef FMA32_BASELINE_DOUBLE_MODEL
static uint32_t evaluate(uint32_t a,uint32_t b,uint32_t c){float af,bf,cf,z;uint32_t u;memcpy(&af,&a,4);memcpy(&bf,&b,4);memcpy(&cf,&c,4);volatile double p=(double)af*(double)bf;volatile double s=p+(double)cf;z=(float)s;memcpy(&u,&z,4);return u;}
#else
#include "fma32.h"
static uint32_t evaluate(uint32_t a,uint32_t b,uint32_t c){return fma32_evaluate(a,b,c).bits;}
#endif
struct vector {uint32_t a,b,c,z;};
static const struct vector cases[]={
{0x3f800001,0x3fc00000,0x8d800000,0x3fc00001},
{0x3f800003,0x3fc00000,0x0d800000,0x3fc00005},
{0x7f7fffff,0x40000000,0xff7fffff,0x7f7fffff},
{0x3f800001,0x3f7ffffe,0xbf800000,0xa8800000},
{0x00000001,0x3f000000,0x00000000,0x00000000},
{0x00000001,0x3fc00000,0x00000000,0x00000002},
{0x80000001,0x3f000000,0x80000000,0x80000000},
{0x00000001,0x3f800000,0x00000000,0x00000001},
{0x00800000,0x3f800000,0x80000001,0x007fffff},
{0x80000000,0x3f800000,0x80000000,0x80000000},
{0x80000000,0x3f800000,0x00000000,0x00000000},
{0x3f800000,0x3f800000,0xbf800000,0x00000000},
{0x7f7fffff,0x40000000,0x00000000,0x7f800000},
{0xff7fffff,0x40000000,0x00000000,0xff800000},
{0x7f800000,0x3f800000,0x00000000,0x7f800000},
{0x7f800000,0x00000000,0x00000000,0x7fc00000},
{0x7f800000,0x3f800000,0xff800000,0x7fc00000},
{0x7fc00001,0x3f800000,0x00000000,0x7fc00000},
{0x7f800001,0x3f800000,0x00000000,0x7fc00000}
};
int main(void){unsigned bad=0;for(unsigned i=0;i<sizeof(cases)/sizeof(cases[0]);++i){const struct vector *v=&cases[i];uint32_t z=evaluate(v->a,v->b,v->c);if(z!=v->z){++bad;printf("FAIL corner%u %08x*%08x+%08x got%08x expected%08x\n",i,v->a,v->b,v->c,z,v->z);}}printf("corners=%u failures=%u\n",(unsigned)(sizeof(cases)/sizeof(cases[0])),bad);return bad!=0;}
