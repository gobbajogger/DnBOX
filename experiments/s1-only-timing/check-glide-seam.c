#include <stdint.h>
#include <assert.h>
#include <stdio.h>
#include "glide-advance.inc"

static float from[2],to[2];
static uint8_t seen[2];
static float reference(unsigned b,int known,float cu,float tg)
{
    float f=1;
    if(known) {
        if(!seen[b] || tg!=to[b]) {
            from[b]=seen[b]?cu:tg; to[b]=tg; seen[b]=1;
        }
        if(to[b]!=from[b]) f=(cu-from[b])/(to[b]-from[b]);
    }
    if(f<0)f=0;
    if(f>1)f=1;
    return f;
}
static uint32_t rng=83620851u;
static uint32_t next(void){return rng=rng*1664525u+1013904223u;}
static int dot(float f,int width)
{return (int)(f*(float)width+.5f);}
int main(void)
{
    matrix_glide_history h[2]={0};
    assert(matrix_glide_advance(&h[0],1,55.f,55.f)==reference(0,1,55.f,55.f));
    float f=matrix_glide_advance(&h[0],1,55.f,67.f);
    assert(f==reference(0,1,55.f,67.f) && f==0.f);
    f=matrix_glide_advance(&h[0],1,61.f,67.f);
    assert(f==reference(0,1,61.f,67.f) && f==.5f);
    const float captured=f;
    /* Target changes at first DMA service. The older frame gets no new
       history update; only its already-captured progress is painted. */
    float current=61.f,target=43.f;
    assert(dot(captured,19)==10 && h[0].to==67.f);
    f=matrix_glide_advance(&h[0],1,current,target);
    assert(f==reference(0,1,current,target) && f==0.f);
    f=matrix_glide_advance(&h[0],1,52.f,43.f);
    assert(f==reference(0,1,52.f,43.f) && f==.5f);
    float dest[2]={43.f,0.f};
    for(unsigned k=0;k<200000;k++) {
        unsigned b=next()>>31;
        if((next()>>29)==0)dest[b]=(float)((int)(next()%257)-128)/2.f;
        const float cu=(float)((int)(next()%257)-128)/4.f;
        int known=(next()>>30)!=0;
        float x=matrix_glide_advance(&h[b],known,cu,dest[b]),y=reference(b,known,cu,dest[b]);
        assert(x==y && h[b].from==from[b] && h[b].to==to[b] && h[b].seen==seen[b]);
        for(int w=1;w<=19;w++)assert(dot(x,w)==dot(y,w));
    }
    puts("PASS: identical glide state, fraction and cursor for 200000 frames; targeted two-window target change and same-target advance");
}
