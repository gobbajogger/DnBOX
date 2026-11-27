#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include "juno6.h"
static juno6_t a,b,old;
static int16_t ha[2][SJ6_UNISON_HISTORY],hb[2][SJ6_UNISON_HISTORY],ho[2][SJ6_UNISON_HISTORY];
static float al[256],ar[256],bl[256],br[256],ol[256],orr[256];
static unsigned exact,audible,changed;
static void run(unsigned rate,unsigned active,unsigned mod){
    const uint8_t ctrl[]={active,60,90,2};
    float state[2]={0},input[2]={0};
    juno6_init(&a,(float)rate);
    juno6_set_sj6_hpf(&a,.42f);
    b=a;old=a;b.hpf=0.f;
    juno6_unison_config(&a,ctrl,ha);juno6_unison_config(&b,ctrl,hb);juno6_unison_config(&old,ctrl,ho);
    const float hz=a.hpf< -1.f?-a.hpf-1.f:15.f+2000.f*a.hpf*a.hpf;
    const float hp=1.f/(1.f+6.28318530717958647692f*hz/a.core.sample_rate);
    juno6_note_on(&a,48,86);juno6_note_on(&b,48,86);juno6_note_on(&old,48,86);
    for(unsigned k=0;k<240;k++){
        const unsigned sched[]={16,17,63,64,128,251};
        unsigned n=sched[k%6];
        if(k==110){juno6_note_off(&a,48);juno6_note_off(&b,48);juno6_note_off(&old,48);}
        assert(juno6_sj6_render_osc_bus(&a,al,ar,n));
        assert(juno6_render_stereo(&b,bl,br,n));
        assert(juno6_render_stereo(&old,ol,orr,n));
        assert(!memcmp(al,bl,n*4)&&!memcmp(ar,br,n*4));
        if(mod){
          juno6_unison_process_mod(&a,al,ar,n,.3f,.7f);
          juno6_unison_process_mod(&b,bl,br,n,.3f,.7f);
          juno6_unison_process_mod(&old,ol,orr,n,.3f,.7f);
        }else{
          juno6_unison_process(&a,al,ar,n);
          juno6_unison_process(&b,bl,br,n);
          juno6_unison_process(&old,ol,orr,n);
        }
        juno6_sj6_hpf_bus(&a,al,ar,n);
        for(unsigned i=0;i<n;i++){
            const float yl=hp*(state[0]+bl[i]-input[0]),yr=hp*(state[1]+br[i]-input[1]);
            input[0]=bl[i];input[1]=br[i];state[0]=yl;state[1]=yr;bl[i]=yl;br[i]=yr;
            if(fabsf(al[i])+fabsf(ar[i])>1e-4f)++audible;
            if(al[i]!=ol[i] || ar[i]!=orr[i]){assert(active);++changed;}
        }
        assert(!memcmp(al,bl,n*4)&&!memcmp(ar,br,n*4));
        if(!active)assert(!memcmp(al,ol,n*4)&&!memcmp(ar,orr,n*4));
        exact+=n;
    }
}
int main(void){
 for(unsigned r=0;r<2;r++)for(unsigned m=0;m<2;m++)for(unsigned u=0;u<2;u++)run(r?48000:44100,u,m);
 assert(audible && changed);
 printf("source-order reference exact %u stereo frames; %u audible, active change in %u, bypass identity\n",exact,audible,changed);
}
