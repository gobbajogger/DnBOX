#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
typedef union { uint32_t u; float f; } fb;
static float fabsbits(float value){ fb a={.f=value}; a.u&=0x7fffffffu;return a.f; }
static unsigned old(unsigned q,const float *l,const float *r,unsigned n){
    for(unsigned i=0;i<n;++i){
        if(fabsbits(l[i])>1.f/65536.f||fabsbits(r[i])>1.f/65536.f)q=0;
        else if(q<512u)++q;
    }
    return q;
}
static unsigned candidate(unsigned q,const float *l,const float *r,unsigned n){
    unsigned i=n;
    while(i&&!(fabsbits(l[i-1u])>1.f/65536.f||fabsbits(r[i-1u])>1.f/65536.f))--i;
    if(i)q=n-i;
    else if(q<512u)q=n<512u-q?q+n:512u;
    return q;
}
static unsigned rng=0x283aa44du;
static unsigned next(void){return rng=rng*1664525u+1013904223u;}
int main(void) {
    static float l[256],r[256];
    static const unsigned start[]={0,1,80,255,511,512,513,0xffffffffu};
    static const uint32_t edge[]={0u,0x80000000u,0x377fffffu,0x37800000u,0x37800001u,0xb7800001u,0x7fc00000u,0xff800000u,0x00000001u,0x80000001u};
    unsigned tests=0;
    for(unsigned k=0;k<(1u<<16);k++){
        for(unsigned i=0;i<16;i++){l[i]=(k>>i&1u)?1.f:-0.f; r[i]=0.f;}
        for(unsigned n=0;n<=16;++n)
            for(unsigned j=0;j<sizeof start/sizeof *start;j++){
                if(old(start[j],l,r,n)!=candidate(start[j],l,r,n))return 1;
                ++tests;
            }
    }
    for(unsigned k=0;k<400000u;k++){
        const unsigned n=next()%257u,q=next()%700u;
        const unsigned active=next()%3u;
        for(unsigned i=0;i<n;i++){
            const fb a={.u=active?edge[next()%10u]:next()},b={.u=active?edge[next()%10u]:next()};
            l[i]=a.f; r[i]=b.f;
        }
        if(old(q,l,r,n)!=candidate(q,l,r,n)) return 2;
        ++tests;
    }
    printf("%u identical quiet suffix state cases\n",tests);
    return 0;
}
