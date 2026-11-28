#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
static uint32_t rs=0xb09a600du;
static uint32_t rand_u(void) { rs ^= rs<<13; rs ^= rs>>17; return rs ^= rs<<5; }
static float asf(uint32_t u) { float v; memcpy(&v,&u,4); return v; }
static uint32_t bits(float a) { uint32_t v; memcpy(&v,&a,4); return v;}
static float ref(const float *x,unsigned n,float g,float p) {
    for(unsigned i=0;i<n;++i) { float a=fabsf(x[i])*g; if(a>p)p=a; }
    return p;
}
static float changed(const float *x,unsigned n,float g,float p) {
    if(g>=0.f && p>=0.f) {
        float raw=0.f;
        for(unsigned i=0;i<n;++i) { float a=fabsf(x[i]); if(a>raw)raw=a; }
        float b=raw*g; if(b>p)p=b;
        return p;
    }
    return ref(x,n,g,p);
}
int main(void) {
    uint32_t specials[]={0,0x80000000,0x7f800000,0xff800000,0x7fc00000,1,
                         0x80000001,0x3f800000,0x7f7fffff,0xff7fffff,0x3f7fffff};
    float x[257]; unsigned long checks=0;
    for(unsigned k=0;k<250000;++k) {
        unsigned n=rand_u()%257;
        for(unsigned i=0;i<n;++i) x[i]=asf((rand_u()&7)?rand_u():specials[rand_u()%11]);
        float g=asf((k&7)<2 ? specials[rand_u()%11] : rand_u() & 0x7fffffff);
        float p=asf((k&7)<2 ? specials[rand_u()%11] : rand_u() & 0x7fffffff);
        if(bits(ref(x,n,g,p))!=bits(changed(x,n,g,p)))return 1;
        checks+=n;
    }
    printf("PASS 250,000 randomized meter reductions, %lu samples incl signed zero, subnormals, Inf and NaN.\n",checks);
}
