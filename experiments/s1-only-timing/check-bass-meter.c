#include <stdint.h>
#include <stdio.h>
typedef union { uint32_t u; float f; } bits;
static float fabsbits(float a){bits b={.f=a};b.u&=0x7fffffffu;return b.f;}
static unsigned rng=54009u;
static unsigned next(void){rng^=rng<<13;rng^=rng>>17;rng^=rng<<5;return rng;}
static float old(float g,float pk,const float*l,const float*r,unsigned n){
    for(unsigned i=0;i<n;i++) {
        float a=fabsbits(l[i])*g;
        if(a>pk)pk=a;
        a=fabsbits(r[i])*g;
        if(a>pk)pk=a;
    }
    return pk;
}
static float new(float g,float pk,const float*l,const float*r,unsigned n){
    if(!(g>=0.f && pk>=0.f))return old(g,pk,l,r,n);
    float raw=0.f;
    for(unsigned i=0;i<n;i++) {
        const float a=fabsbits(l[i]),b=fabsbits(r[i]);
        if(a>raw)raw=a;
        if(b>raw)raw=b;
    }
    float peak=raw*g;
    return peak>pk?peak:pk;
}
int main(void){
    static float l[256], r[256];
    static const unsigned edges[]={0,0x80000000,0x3f800000,0x7f7fffff,0x7f800000,0xff800000,0xffc00000,0x00000001,0x80000001,0x00800000,0x7fc00000,0x7f800001,0xff800001};
    for(unsigned k=0;k<400000;++k) {
        unsigned n=next()%257, a=next()%2, b=next()%2;
        bits g={.u=a?(next()&0x7fffffff):edges[next()%13]}, pk={.u=b?(next()&0x7fffffff):edges[next()%13]};
        for(unsigned i=0;i<n;i++) {
            l[i]=(bits){.u=a?next():edges[next()%13]}.f;
            r[i]=(bits){.u=b?next():edges[next()%13]}.f;
        }
        if((bits){.f=old(g.f,pk.f,l,r,n)}.u!=(bits){.f=new(g.f,pk.f,l,r,n)}.u){fprintf(stderr,"mismatch %u\n",k);return 1;}
    }
    puts("400000 exact stereo peak cases");
    return 0;
}
