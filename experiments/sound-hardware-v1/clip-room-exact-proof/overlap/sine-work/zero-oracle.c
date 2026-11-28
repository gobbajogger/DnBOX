#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include "s1u.h"
typedef union{float f;uint32_t u;} word;
static const uint32_t velocities[]={0x38000000u,0x3f800000u,0xb8000000u,0xbf800000u};
static void check(uint32_t p, uint64_t i)
{
    float s=s1u_sin(p);
    word bits={.f=s};
    int pos=p-1u<0x7fffffffu;
    if ((bits.u>>31)!=(p>>31) || (s>0.f)!=pos) abort();
    word gain={.u=(uint32_t)((i&1u)<<31)}, v={.u=velocities[(i>>1)&3u]};
    word zs={.f=s*v.f*gain.f*3.f}, sq={.f=s>0.f?gain.f:-gain.f};
    if(zs.u!=((p^v.u^gain.u)&0x80000000u) ||
       sq.u!=((gain.u^(!pos?0x80000000u:0u)))) abort();
}
int main(void){
    uint64_t n=0;
    const uint32_t ends[]={0u,127u,128u,255u};
    for(unsigned j=0;j<4;j++)for(uint32_t x=0;x<0x1000000u;x++)
        check((ends[j]<<24)|x,n++);
    for(unsigned j=0;j<256;j++)for(uint32_t x=0;x<0x1000000u;x+=251u)
        check((j<<24)|x,n++);
    for(uint32_t p=0, i=0;i<8388608u;i++,p=p*1664525u+1013904223u)
        check(p,n++);
    printf("%llu exact signed-zero phase/velocity/volume/square comparisons\n",(unsigned long long)n);
}
