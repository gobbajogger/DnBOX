/* Post-Apple dry Main-only suffix, chronological 44.1 kHz. The caller must
 * independently verify leaf/source custody and fixed original control states. */
#include <math.h>
#include <string.h>

void *pan_live_new(unsigned,float,float);
void pan_live_active(void *,int);
void pan_live_free(void *);
void pan_live_run(void *,const float *,float *,unsigned);
int sj6_settled_postcrusher(const float *src,float *dst,unsigned total,
                            const unsigned *blocks,unsigned count,float raw_master)
{
    if(!src||!dst||!blocks||src==dst||!(raw_master>=0.f&&raw_master<=1.f))return 0;
    void *pan=pan_live_new(44100,0.f,0.f);
    if(!pan)return 0;
    float x[256],y[256],pre[2]={0},last[2]={0};
    const float atten=(float)(.404*(double)raw_master);
    unsigned at=0;
    int ok=1;
    pan_live_active(pan,1);
    for(unsigned b=0;b<count;b++){
        unsigned n=blocks[b];
        if(!n||n>128||n>total-at){ok=0;break;}
        for(unsigned c=0;c<2;c++)
            memcpy(x+c*n,src+c*total+at,n*sizeof *x);
        pan_live_run(pan,x,y,n);
        for(unsigned c=0;c<2;c++)for(unsigned i=0;i<n;i++){
            /* Crusher booster and stopped chorus booster unity, widen,
             * chorus, delay, Rage and cold room disabled before first render.
             * Current Mac source master booster .85, old build used .95. */
            const float in=(y[c*n+i]*.85f)*atten;
            dst[c*total+at+i]=last[c]=fmaf(.999f,last[c],in-pre[c]);
            pre[c]=in;
        }
        at+=n;
    }
    pan_live_free(pan);
    return ok&&at==total;
}
