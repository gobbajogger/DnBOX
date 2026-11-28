/* SPDX-License-Identifier: GPL-3.0-only */
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#define X0X_DRUM_TABLES_DEFINE
#include "dsp/fxbus.h"
#include "dsp/master.h"
const char *const x0x_dist_names[8]={"Off","Diode","Clip","SAT","BFZ","PDIST","Fold","Crush"};
static fxbus_t fx[2];
static master_t mst[2];
static int16_t history[2][79200];
static uint32_t seed=0x3a8711au;
static uint32_t rnd(void){seed^=seed<<13;seed^=seed>>17;seed^=seed<<5;return seed;}
static float sample(void){return (float)(int32_t)rnd()*(2.0f/2147483648.f);}
static void chunk(fxbus_t*f,const float*d0,const float*d1,const float*v,const float*d,float*l,float*r,int i,int m)
{
    fxbus_process_st(f,d0+i,d1+i,v+i,d+i,l+i,r+i,m);
}
int main(void)
{
    unsigned frames=0;
    for(int x=0;x<2;++x){fxbus_init(&fx[x],history[x],79200);master_init(&mst[x]);}
    for(unsigned trial=0;trial<20000u;++trial){
        float ol[2][256],orr[2][256],dry[2][256],rev[256],dly[256];
        const int prefix=1+rnd()%95u, suffix=32+rnd()%(257-prefix-32);
        const int n=prefix+suffix, first=suffix<32?suffix:32;
        const float vol=(float)(rnd()%4097u)*(1.f/4096);
        for(int i=0;i<n;++i){
            dry[0][i]=sample();dry[1][i]=sample();
            rev[i]=trial%4u==0?sample():0.f;
            dly[i]=trial%7u==0?sample():0.f;
            ol[1][i]=ol[0][i]=sample();orr[1][i]=orr[0][i]=sample();
        }
        if(trial%13u==0){
            const unsigned mk=rnd()%MST_NPARAMS, fk=rnd()%FX_NPARAMS, v=rnd()%128u, q=rnd()%128u;
            for(int a=0;a<2;++a){master_set(&mst[a],mk,v);fxbus_set(&fx[a],fk,q);}
        }
        const float kick=(float)(rnd()&3u)*.25f;
        if(trial%5u==0)for(int a=0;a<2;++a)master_key(&mst[a],trial&1u,kick);
        /* Old order: master prior prefix before dispatching the new FX job. */
        float g0=master_process_st_continue(&mst[0],ol[0],orr[0],prefix,vol,0.f);
        chunk(&fx[0],dry[0],dry[1],rev,dly,ol[0],orr[0],prefix,first);
        /* New order: the worker's first chunk can finish while the older
         * master prefix runs. Neither DSP object nor its output span overlap. */
        chunk(&fx[1],dry[0],dry[1],rev,dly,ol[1],orr[1],prefix,first);
        float g1=master_process_st_continue(&mst[1],ol[1],orr[1],prefix,vol,0.f);
        g0=master_process_st_continue(&mst[0],ol[0]+prefix,orr[0]+prefix,first,vol,g0);
        g1=master_process_st_continue(&mst[1],ol[1]+prefix,orr[1]+prefix,first,vol,g1);
        for(int i=prefix+first;i<n;i+=32){
            const int m=n-i<32?n-i:32;
            for(int a=0;a<2;++a) chunk(&fx[a],dry[0],dry[1],rev,dly,ol[a],orr[a],i,m);
            g0=master_process_st_continue(&mst[0],ol[0]+i,orr[0]+i,m,vol,g0);
            g1=master_process_st_continue(&mst[1],ol[1]+i,orr[1]+i,m,vol,g1);
        }
        assert(!memcmp(ol[0],ol[1],n*sizeof(float)) && !memcmp(orr[0],orr[1],n*sizeof(float)));
        int16_t *p=fx[1].dbuf;fx[1].dbuf=fx[0].dbuf;
        assert(!memcmp(&mst[0],&mst[1],sizeof mst[0]) && !memcmp(&fx[0],&fx[1],sizeof fx[0]));
        fx[1].dbuf=p;
        assert(!memcmp(history[0],history[1],sizeof history[0]) && !memcmp(&g0,&g1,sizeof g0));
        frames+=n;
    }
    printf("Master prefix / FX suffix order: all state and stereo output exact across 20,000 spans and %u frames\n",frames);
    return 0;
}
