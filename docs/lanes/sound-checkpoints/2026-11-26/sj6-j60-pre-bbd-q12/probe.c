#include "juno6.h"
#include "fastmath.h"
#include <math.h>
#include <string.h>
static unsigned at,opt;
static float fr[2][1024],*tap[2];
static const double *auth[2];
#include "wet.inc"
__attribute__((visibility("default"))) void witness(float *l,float *r,float *hl,const double *hp,float *hr,unsigned n,int opt_) {
    juno6_chorus_t c={0};
    c.sample_rate=96000.f;c.factory_mode=JUNO6_CHORUS_I;
    uint8_t b[4]={1,0,0,128};juno6_j60_config(&c,b);
    opt=(unsigned)opt_;tap[0]=hl;tap[1]=hr;auth[0]=hp;auth[1]=hp+n;
    memset(fr,0,sizeof fr);
    for(at=0;at<n;at++){
        const float input[2]={l[at],r[at]};
        j60_core(&c,&c.j60,1,input);
        l[at]=(float)(1.17*(double)c.j60.wet[0]);r[at]=(float)(1.17*(double)c.j60.wet[1]);
    }
}
