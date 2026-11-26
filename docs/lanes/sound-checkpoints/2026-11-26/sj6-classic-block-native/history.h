#ifndef SJ6_CLASSIC_BLOCK_HISTORY_H
#define SJ6_CLASSIC_BLOCK_HISTORY_H
#include <stdint.h>
enum {BS=32, NG=8, MAX_EXP=7};
typedef struct {uint8_t exp[NG], previous;} blockmeta;
_Static_assert(sizeof(blockmeta)==9,"per-owner exponent overhead");
static inline float blockread(const int16_t *p,const blockmeta *m,unsigned w,unsigned i)
{
    unsigned g=i/BS,exp=m->exp[g];
    /* Before first store, no new prefix exists. exp[g] is still old.
       On subsequent writes, the current group's suffix is old. */
    if ((w&(BS-1)) && (w/BS)==g && i>=w) exp=m->previous;
    return ((int32_t)p[i]*(int32_t)(1u<<exp))*(1.f/32768.f);
}
static inline int32_t truncate_scale(int32_t x,unsigned e)
{
    return x<0?-(int32_t)(((uint32_t)-x)>>e):(int32_t)(((uint32_t)x)>>e);
}
static inline unsigned blockwrite(int16_t *p,blockmeta *m,unsigned w,float x,unsigned *lifts)
{
    unsigned offset=w&(BS-1),g=w/BS,e=m->exp[g],start=w-offset;
    if(!offset){m->previous=(uint8_t)e; e=0;}
    float q=x*32767.f;
    if(x!=x)q=0.f;
    q=q>4194303.f?4194303.f:q< -4194304.f?-4194304.f:q;
    int32_t s=(int32_t)q,t=truncate_scale(s,e);
    unsigned old=e;
    while(t>32767 || t< -32768){++e;t=truncate_scale(s,e);}
    if(e!=old){
        ++*lifts;
        /* Read-before-write guarantees old suffix is still live. Never touch it. */
        for(unsigned j=start;j<w;++j)p[j]=(int16_t)truncate_scale(p[j],e-old);
    }
    m->exp[g]=(uint8_t)e;
    p[w]=(int16_t)t;
    return e==old?0u:offset;
}
#endif
