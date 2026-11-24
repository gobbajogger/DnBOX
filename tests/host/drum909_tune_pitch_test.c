/* SPDX-License-Identifier: GPL-3.0-only */
/* FINAL-GOAL row 16: independent kick sweep time / base pitch, default PCM unchanged. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "dsp/drum909.h"
void ref_drum909_init(drum909_t *);
void ref_drum909_set(drum909_t *,int,int,int);
void ref_drum909_trigger(drum909_t *,int,float);
void ref_drum909_render(drum909_t *,float *,float *,float *,int);
#define CHECK(x) do{if(!(x)){fprintf(stderr,"FAIL tune/pitch:%d %s\n",__LINE__,#x);exit(1);}}while(0)
static void set_defaults(drum909_t *d,int ref)
{
    if(ref)ref_drum909_init(d);else drum909_init(d);
    for(int v=0;v<=DR_KIT;v++)for(int p=0;p<drum909_nparams(v);p++){
        int value=drum909_param(v,p)->def;
        if(ref)ref_drum909_set(d,v,p,value);else drum909_set(d,v,p,value);
    }
}
static void pcm(float *out,int tune,int pitch)
{
    drum909_t d; set_defaults(&d,0);drum909_set(&d,DR_BD,0,tune);drum909_set(&d,DR_BD,5,pitch);drum909_trigger(&d,DR_BD,1.f);
    for(int i=0;i<8192;i+=128){float r[128]={0},l[128]={0};memset(out+i,0,128*sizeof(float));drum909_render(&d,out+i,r,l,128);}
}
int main(void)
{
    drum909_t old,now;
    /* Both boot-native and serialized-pot defaults, each voice and the whole kit. */
    for(int defaults=0;defaults<2;defaults++)for(int voice=0;voice<=DR_NUM;voice++){
        if(defaults){set_defaults(&old,1);set_defaults(&now,0);}else{ref_drum909_init(&old);drum909_init(&now);}
        for(int v=0;v<DR_NUM;v++)if(voice==DR_NUM||v==voice){ref_drum909_trigger(&old,v,.73f);drum909_trigger(&now,v,.73f);}
        for(int block=0;block<512;block++){
            float a[128]={0},b[128]={0},ar[128]={0},br[128]={0},al[128]={0},bl[128]={0};
            ref_drum909_render(&old,a,ar,al,128);drum909_render(&now,b,br,bl,128);
            CHECK(!memcmp(a,b,sizeof a)&&!memcmp(ar,br,sizeof ar)&&!memcmp(al,bl,sizeof al));
        }
    }
    puts("PASS boot + preset defaults: each voice and whole kit, 65536 frames bit-identical to pre-change engine");
    float pm0,tau0,m0;drum909_param_si(DR_BD,5,45,&pm0);drum909_param_si(DR_BD,0,34,&tau0);drum909_param_si(DR_BD,4,0,&m0);CHECK(m0==0);
    for(int value=0;value<128;value++){
        float tau,pm;drum909_param_si(DR_BD,0,value,&tau);drum909_param_si(DR_BD,5,value,&pm);
        float f=drum909_bd_hz(tau,m0,pm0,2000.f),base=drum909_bd_hz(tau0,m0,pm,2000.f);
        CHECK(f==49.f); if(value!=45)CHECK(fabsf(base-49.f)>.1f);
        /* e^-1 at one TUNE time constant, independent of PITCH's offset. */
        float start=drum909_bd_hz(tau,m0,pm,16.f)-base,one=drum909_bd_hz(tau,m0,pm,16.f+tau)-base;
        CHECK(fabsf(one/start-expf(-1.f))<.00001f);
        /* Changing PITCH translates the entire Hz trajectory; it cannot change tau. */
        for(int t=0;t<150;t+=7){float delta=drum909_bd_hz(tau0,m0,pm,(float)t)-drum909_bd_hz(tau0,m0,pm0,(float)t);CHECK(fabsf(delta-(base-49.f))<.0001f);}
        float a[8192],b[8192];pcm(a,value,45);pcm(b,34,value);CHECK(memcmp(a,b,sizeof a));
    }
    puts("PASS all 128 values: TUNE changes sweep tau, PITCH changes fundamental at default P.Dpth; audio effects never identical");return 0;
}
