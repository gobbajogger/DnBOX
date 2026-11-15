#include "numeric_boundary.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <fenv.h>
#include <pthread.h>
static uint64_t rng(uint64_t*s){uint64_t x=*s;x^=x<<13;x^=x>>7;x^=x<<17;return*s=x;}
static double d(uint64_t u){double x;memcpy(&x,&u,8);return x;}
static uint32_t u(float x){uint32_t z;memcpy(&z,&x,4);return z;}
static unsigned ff(void){int f=fetestexcept(FE_ALL_EXCEPT);return((f&FE_INEXACT)?1:0)|((f&FE_UNDERFLOW)?2:0)|((f&FE_OVERFLOW)?4:0)|((f&FE_DIVBYZERO)?8:0)|((f&FE_INVALID)?16:0);}
static uint64_t widen(uint32_t x){uint64_t sign=(uint64_t)(x>>31)<<63;unsigned e=(x>>23)&255;uint64_t n=x&0x7fffff;if(!e&&!n)return sign;int power=e?(int)e-150:-149;if(e)n|=0x800000;unsigned bits=0;for(uint64_t a=n;a;a>>=1)bits++;int exp=(int)bits-1+power;return sign|((uint64_t)(exp+1023)<<52)|((n<<(53-bits))&UINT64_C(0xfffffffffffff));}
static int one(uint64_t x,unsigned*i){feclearexcept(FE_ALL_EXCEPT);volatile double a=d(x);volatile float f=(float)a;uint32_t want=u(f);unsigned flags=ff();if((want&0x7fffffff)>0x7f800000)want=0x7fc00000;fma32_result z=f64_to_f32_evaluate(x);if(z.bits!=want||z.flags!=flags){if((*i)++<5)printf("FAIL %016llx got%08x/%u expected%08x/%u\n",(unsigned long long)x,z.bits,z.flags,want,flags);return 1;}if(u(f64_to_f32_rne(d(x)))!=z.bits){++*i;printf("ABI failure\n");return 1;}return 0;}
struct args{unsigned bad;};static void* th(void*p){struct args*a=p;for(unsigned i=0;i<100000;++i){fma32_result z=(i&1)?f64_to_f32_evaluate(UINT64_C(0x3ff0000010000001)):f64_to_f32_evaluate(UINT64_C(0x7ff0000000000001));if(z.bits!=((i&1)?0x3f800001:0x7fc00000)||z.flags!=((i&1)?1:16))a->bad++;}return 0;}
int main(void){fesetround(FE_TONEAREST);unsigned bad=0;uint64_t s=UINT64_C(0x64f032123456789);const uint64_t v[]={0,UINT64_C(0x8000000000000000),1,UINT64_C(0x8000000000000001),UINT64_C(0xfffffffffffff),UINT64_C(0x10000000000000),UINT64_C(0x3690000000000000),UINT64_C(0x3690000000000001),UINT64_C(0x36a0000000000000),UINT64_C(0x380fffffdfffffff),UINT64_C(0x380fffffe0000000),UINT64_C(0x380fffffe0000001),UINT64_C(0x3810000000000000),UINT64_C(0x3ff0000010000000),UINT64_C(0x3ff0000010000001),UINT64_C(0x3ff0000030000000),UINT64_C(0x47efffffe0000000),UINT64_C(0x47efffffefffffff),UINT64_C(0x47effffff0000000),UINT64_C(0x47effffff0000001),UINT64_C(0x7fefffffffffffff),UINT64_C(0x7ff0000000000000),UINT64_C(0xfff0000000000000),UINT64_C(0x7ff0000000000001),UINT64_C(0x7ff8000000000000),UINT64_C(0xfff8123456789abc)};
for(unsigned i=0;i<sizeof(v)/sizeof(*v);++i)one(v[i],&bad);for(unsigned i=0;i<2000000;++i)one(rng(&s),&bad);
uint64_t exact=0;for(uint32_t x=1;x<0x800000;++x){fma32_result a=f64_to_f32_evaluate(widen(x)),b=f64_to_f32_evaluate(widen(x|0x80000000));if(a.bits!=x||b.bits!=(x|0x80000000)||a.flags||b.flags){printf("subnormal roundtrip failure%08x\n",x);return 1;}exact+=2;}
unsigned envbad=0;int modes[]={FE_TONEAREST,FE_UPWARD,FE_DOWNWARD,FE_TOWARDZERO};for(unsigned i=0;i<4;++i){fesetround(modes[i]);feclearexcept(FE_ALL_EXCEPT);feraiseexcept(FE_DIVBYZERO);fma32_result z=f64_to_f32_evaluate(UINT64_C(0x3ff0000010000001));if(z.bits!=0x3f800001||z.flags!=1||ff()!=8||fegetround()!=modes[i])envbad++;}fesetround(FE_TONEAREST);
pthread_t t[4];struct args a[4]={{0},{0},{0},{0}};unsigned tb=0;for(unsigned i=0;i<4;++i)if(pthread_create(&t[i],0,th,&a[i]))return 2;for(unsigned i=0;i<4;++i){pthread_join(t[i],0);tb+=a[i].bad;}
printf("conversion host_casts=2000026 exact_signed_subnormal_roundtrips=%llu result_or_flag_or_ABI_failures=%u environment_failures=%u concurrent_cases=400000 concurrent_failures=%u\n",(unsigned long long)exact,bad,envbad,tb);return(bad||envbad||tb)?1:0;}
