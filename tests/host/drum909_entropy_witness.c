/* SPDX-License-Identifier: GPL-3.0-only */
#include "drum909.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

static uint32_t rng=1;
static uint32_t next(void) { rng^=rng<<13; rng^=rng>>17; rng^=rng<<5; return rng; }
#if DNBOX_LAB_D9_SUFFIX
void plat_cpu2_sync(void) { }
#endif
static void out(const void *p,size_t n) { assert(fwrite(p,1,n,stdout)==n); }
static void rate(drum909_t *d,unsigned k)
{
    static const float r[]={1.0f,2.0f,4.0f,0.25f,0.5f,0.7937005f,1.3348398f,3.1748021f};
    for (unsigned j=0;j<4;++j) {
        d9_smp_t *s=&d->smp[j];
        float pitch=r[(k+j)%8];
        s->inc=(uint32_t)pitch;
        s->incf=(uint32_t)((pitch-(float)s->inc)*4294967296.0f);
    }
}
int main(void)
{
    unsigned calls=0;
    for (unsigned seed=1;seed<=12;++seed) {
        drum909_t d;
        memset(&d,0,sizeof d); drum909_init(&d);
        rng=seed;
        for (int v=0;v<DR_NUM;++v) {
            for (int p=0;p<drum909_nparams(v);++p) {
                const x0x_param_t *s=drum909_param(v,p);
                drum909_set(&d,v,p,(int)(next()%(s->max+1u)));
            }
            drum909_trigger(&d,v,0.8f);
        }
        rate(&d,seed);
        for (unsigned k=0;k<1300;++k) {
            float bus[4][256]={{0}};
            unsigned n=k%257;
            uint16_t mask=(uint16_t)(next()&0x7ffu);
            if (k%53==0) drum909_trigger(&d,(int)((k/53+seed)%DR_NUM),0.6f);
            if (k%71==0) rate(&d,k/71);
            if (k==699) drum909_all_off(&d);
            if (k==700) for (int v=0;v<DR_NUM;++v) drum909_trigger(&d,v,1.0f);
            if (k==910) {
                for (unsigned v=0;v<4;++v) {
                    drum909_trigger(&d,DR_CH+(int)v,1.0f); d.smp[v].len=1u+v;
                }
                rate(&d,0);
            }
#if DNBOX_LAB_D9_SUFFIX
            if (k%3 && n>=128) {
                drum909_trial_snapshot(&d);
                if (k%3==1) drum909_trial_helper((int)n);
                drum909_trial_serial(&d,bus[0],bus[1],bus[2],bus[3],(int)n,mask);
            } else
#endif
                drum909_render_st_masked(&d,bus[0],bus[1],(k%5)?bus[2]:0,(k%7)?bus[3]:0,(int)n,mask);
            out(bus,sizeof bus);
            for (unsigned v=0;v<4;++v) {
                const d9_smp_t *s=&d.smp[v];
                uint32_t state[]={s->dk,(uint32_t)s->pred,(uint32_t)s->idx,(uint32_t)s->h0,(uint32_t)s->h1,s->pos,s->frac};
                out(state,sizeof state);
            }
            ++calls;
        }
    }
    fprintf(stderr,"%u actual renderer calls; dry L/R, sends and decoder state\n",calls);
    return 0;
}
