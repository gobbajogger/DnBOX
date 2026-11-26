/* Independent state-minimal realization of the known original transfer.
   No original sample/table data. A first-order section has ONE state:
   y=a*x+z, next z=x-a*y; same H(z) as previous-input/previous-output. */
#include <stdint.h>
#ifndef PACK
#define PACK 0
#endif
#if PACK
typedef int16_t qhist;
#else
typedef float qhist;
#endif
typedef struct {float phase; qhist histories[2][4];} compact_halfband;
#if PACK==2
static inline int32_t qshift(int32_t x){return (x+16384)>>15;}
static inline int16_t qsat(int32_t x){return x>32767?32767:x< -32768?-32768:x;}
static inline int32_t qp(int32_t x,int32_t a,int16_t *s){
    int32_t y=*s+qshift(a*x);
    if(x==0){
        int32_t p=-a*y;                          /* truncation to silence */
        *s=qsat((p+(p<0?32767:0))>>15);
    }else *s=qsat(x-qshift(a*y));
    return y;
}
static inline int32_t qhalf(int32_t a,int32_t b,int16_t h[4]){
    b=qp(b,2617,h);a=qp(a,9301,h+1);
    b=qp(b,17870,h+2);a=qp(a,27342,h+3);
    return (a+b+1)>>1;
}
static inline int32_t encode(float f){
    float q=f*16384.f;
    if(q>=32767.f)return 32767;
    if(q<=-32768.f)return -32768;
    return (int32_t)(q+(q<0.f?-.5f:.5f));
}
static inline float compact_decimate(compact_halfband *s,const float x[4]){
    int32_t a=qhalf(encode(x[0]),encode(x[1]),s->histories[0]);
    int32_t b=qhalf(encode(x[2]),encode(x[3]),s->histories[0]);
    return (float)qhalf(a,b,s->histories[1])*0x1p-14f;
}
#else
static inline float pole(float x,float a,qhist *h)
{
#if PACK
    const float z=(float)*h * 0x1p-14f;
#else
    const float z=*h;
#endif
    const float y=z+a*x;
    const float next=x-a*y;
#if PACK
    float q=next * 0x1p14f;
    if(q>32767.f)q=32767.f;
    if(q< -32768.f)q=-32768.f;
    *h=(qhist)(q+(x==0.f?0.f:q<0.f?-.5f:.5f));
#else
    *h=next;
#endif
    return y;
}
static inline float halfband(float even,float odd,qhist h[4])
{
    odd=pole(odd,.079866424202919f,h);
    even=pole(even,.28382933139801025f,h+1);
    odd=pole(odd,.5453536510467529f,h+2);
    even=pole(even,.8344119191169739f,h+3);
    return .5f*(odd+even);
}
static inline float compact_decimate(compact_halfband *s,const float x[4])
{
    float a=halfband(x[0],x[1],s->histories[0]);
    float b=halfband(x[2],x[3],s->histories[0]);
    return halfband(a,b,s->histories[1]);
}

#endif
