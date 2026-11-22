/* SPDX-License-Identifier: GPL-3.0-only */
#include "app/original_runtime.h"
#include "soundpipe.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

typedef struct {
    sp_phasor phase;
    sp_port smoother;
    float level;
    unsigned shape;
} original_lfo;
typedef struct {
    original_runtime_part wet, dry;
    original_lfo lfo[2];
    unsigned select, frames[4], silent, edited;
    double peak, squared;
} pair;
static pair part[2];
static original_runtime_workspace work;
static sp_data sp={.sr=44100,.nchan=2};

static float source(original_lfo *o)
{
    float p,x;
    sp_phasor_compute(&sp,&o->phase,NULL,&p);
    switch(o->shape) {
    case 1: x=p>.5f ? 1.f : -1.f; break;
    case 2: x=(p-.5f)*2.f; break;
    case 3: x=(.5f-p)*2.f; break;
    default: x=(float)sin((double)p*6.2831853071795864769); break;
    }
    sp_port_compute(&sp,&o->smoother,&x,&x);
    return 1.f-(.5f*(1.f-x)*o->level);
}

static void parameter(pair *p,unsigned id,unsigned position)
{
    float value=(float)position/127.f;
    assert(original_runtime_param(&p->wet,id,value)==ORIGINAL_RUNTIME_OK);
    unsigned choice=(position*3u+63u)/127u;
    if(id==ORIGINAL_SYNTHONE_TREM) { p->select=choice;return; }
    original_lfo *lfo=&p->lfo[id>=ORIGINAL_SYNTHONE_LFO2_SHAPE];
    if(id==ORIGINAL_SYNTHONE_LFO1_LEVEL || id==ORIGINAL_SYNTHONE_LFO2_LEVEL)lfo->level=value;
    else if(id==ORIGINAL_SYNTHONE_LFO1_SHAPE || id==ORIGINAL_SYNTHONE_LFO2_SHAPE)lfo->shape=choice;
    else lfo->phase.freq=s1_lfo_hz(position);
}
static void note(pair *p,unsigned key,unsigned vel)
{
    assert(original_runtime_key(&p->wet,ORIGINAL_INPUT_KEYS,key,vel)==ORIGINAL_RUNTIME_OK);
    assert(original_runtime_key(&p->dry,ORIGINAL_INPUT_KEYS,key,vel)==ORIGINAL_RUNTIME_OK);
}

static void factory(const uint8_t cell[32],const char *controls)
{
    FILE *f=fopen(controls,"r");
    assert(f);
    unsigned route, shape[2], programs=0,routes[4]={},rounded=0,worst=0;
    float level[2],rate[2];double peak=0.,panel_peak=0.,panel_rms=0.;
    while(fscanf(f,"%u %u %a %a %u %a %a",&route,&shape[0],&level[0],&rate[0],&shape[1],&level[1],&rate[1])==7){
        pair *p=&part[0];
        original_runtime_part *panel=&part[1].wet;
        assert(original_runtime_init(&p->wet,cell,120,NULL,NULL)==ORIGINAL_RUNTIME_OK);
        assert(original_runtime_init(&p->dry,cell,120,NULL,NULL)==ORIGINAL_RUNTIME_OK);
        assert(original_runtime_init(panel,cell,120,NULL,NULL)==ORIGINAL_RUNTIME_OK);
        p->wet.dsp.synthone.tremolo=route;
        assert(original_runtime_param(panel,ORIGINAL_SYNTHONE_TREM,(float)route/3.f)==ORIGINAL_RUNTIME_OK);
        unsigned differed=0;
        for(unsigned v=0;v<2;++v) {
            original_lfo *o=&p->lfo[v];
            sp_phasor_init(&sp,&o->phase,0);sp_port_init(&sp,&o->smoother,.0053125f);
            o->level=level[v];o->shape=shape[v];o->phase.freq=rate[v];
            s1_trem_lfo *lfo=&p->wet.dsp.synthone.lfo[v];
            lfo->shape=shape[v];lfo->level=level[v];lfo->increment=rate[v]*(1.f/44100.f);
            unsigned id=v?ORIGINAL_SYNTHONE_LFO2_SHAPE:ORIGINAL_SYNTHONE_LFO1_SHAPE;
            unsigned q=0;
            for(unsigned i=1;i<128;++i)if(fabsf(s1_lfo_hz(i)-rate[v])<fabsf(s1_lfo_hz(q)-rate[v]))q=i;
            unsigned depth=(unsigned)(level[v]*127.f+.5f);
            assert(original_runtime_param(panel,id,(float)shape[v]/3.f)==ORIGINAL_RUNTIME_OK);
            assert(original_runtime_param(panel,id+1,(float)depth/127.f)==ORIGINAL_RUNTIME_OK);
            assert(original_runtime_param(panel,id+2,(float)q/127.f)==ORIGINAL_RUNTIME_OK);
            differed |= level[v]!=(float)depth/127.f || rate[v]!=s1_lfo_hz(q);
        }
        rounded+=differed;
        assert(route<4);++routes[route];
        note(p,60,110);
        assert(original_runtime_key(panel,ORIGINAL_INPUT_KEYS,60,110)==ORIGINAL_RUNTIME_OK);
        double squared=0.;
        for(unsigned at=0;at<44100;){
            unsigned n=44100-at>64 ? 64 : 44100-at;
            float l[64],r[64],pl[64],pr[64],dry[64],dr[64];
            assert(original_runtime_render(&p->wet,&work,l,r,n)==ORIGINAL_RUNTIME_OK);
            assert(original_runtime_render(&p->dry,&work,dry,dr,n)==ORIGINAL_RUNTIME_OK);
            assert(original_runtime_render(panel,&work,pl,pr,n)==ORIGINAL_RUNTIME_OK);
            for(unsigned i=0;i<n;++i){
                float a=source(&p->lfo[0]),b=source(&p->lfo[1]);
                float expected=dry[i]*(route==1?a:route==2?b:route==3?.5f*(a+b):1.f);
                double err=fabs((double)l[i]-expected),other=(double)pl[i]-expected;
                assert(r[i]==l[i] && pr[i]==pl[i] && isfinite(err) && isfinite(other));
                peak=fmax(peak,err);panel_peak=fmax(panel_peak,fabs(other));squared+=other*other;
            }
            at+=n;
        }
        double rms=sqrt(squared/44100.);
        if(rms>panel_rms){panel_rms=rms;worst=programs;}
        ++programs;
    }
    fclose(f);
    printf("programs=%u frames=%u routes=%u,%u,%u,%u full_precision_peak=%.9g panel_rounded=%u panel_peak=%.9g panel_worst_rms=%.9g panel_worst_index=%u\n",
        programs,programs*44100u,routes[0],routes[1],routes[2],routes[3],peak,rounded,panel_peak,panel_rms,worst);
    assert(programs==694 && routes[0]==620 && peak<2e-6);
}

int main(int argc,char **argv)
{
    uint8_t cell[32];
    assert(original_sound_init(cell,ORIGINAL_SOUND_SYNTHONE,0));
    for(unsigned v=0;v<2;++v){
        pair *p=&part[v];
        assert(original_runtime_init(&p->wet,cell,120,NULL,NULL)==ORIGINAL_RUNTIME_OK);
        assert(original_runtime_init(&p->dry,cell,120,NULL,NULL)==ORIGINAL_RUNTIME_OK);
        for(unsigned i=0;i<2;++i){
            sp_phasor_init(&sp,&p->lfo[i].phase,0);
            p->lfo[i].phase.freq=.25f;
            sp_port_init(&sp,&p->lfo[i].smoother,.0053125f);
        }
    }
    for(unsigned block=0;block<2100;++block)for(unsigned v=0;v<2;++v){
        pair *p=&part[v];
        unsigned n=1+(17*block+43*v)%128;
        if(block==1)note(p,53+12*v,112);
        if(block==32){parameter(p,ORIGINAL_SYNTHONE_TREM,42);parameter(p,ORIGINAL_SYNTHONE_LFO1_LEVEL,0);}
        if(block==60)parameter(p,ORIGINAL_SYNTHONE_LFO1_LEVEL,127-25*v);
        if(block==80)parameter(p,ORIGINAL_SYNTHONE_LFO1_RATE,127-19*v);
        if(block==120)parameter(p,ORIGINAL_SYNTHONE_LFO2_RATE,87+3*v);
        if(block==141)parameter(p,ORIGINAL_SYNTHONE_LFO2_LEVEL,70+45*v);
        if(block%151==0 && block>=151){
            unsigned step=block/151;
            parameter(p,ORIGINAL_SYNTHONE_TREM,((step+v)%4)*127/3);
            parameter(p,ORIGINAL_SYNTHONE_LFO1_SHAPE,((step+1+v)%4)*127/3);
            parameter(p,ORIGINAL_SYNTHONE_LFO2_SHAPE,((step+2*v)%4)*127/3);
        }
        if(block==790)note(p,53+12*v,0);
        if(block==1100) {
            assert(original_runtime_all_off(&p->wet)==ORIGINAL_RUNTIME_OK);
            assert(original_runtime_all_off(&p->dry)==ORIGINAL_RUNTIME_OK);
        }
        if(block==1280)parameter(p,ORIGINAL_SYNTHONE_LFO1_RATE,0);
        if(block==1371)parameter(p,ORIGINAL_SYNTHONE_LFO2_RATE,127);
        if(block==1430)note(p,72-3*v,100);
        float l[128],r[128],dl[128],dr[128];
        assert(original_runtime_render(&p->wet,&work,l,r,n)==ORIGINAL_RUNTIME_OK);
        assert(original_runtime_render(&p->dry,&work,dl,dr,n)==ORIGINAL_RUNTIME_OK);
        for(unsigned i=0;i<n;++i){
            float a=source(&p->lfo[0]),b=source(&p->lfo[1]);
            float gain=p->select==1?a:p->select==2?b:p->select==3?.5f*(a+b):1.f;
            float expected=dl[i]*gain;
            double err=fabs((double)l[i]-expected);
            assert(r[i]==l[i] && isfinite(err));
            p->peak=fmax(p->peak,err);p->squared+=err*err;++p->frames[p->select];
            if(!p->select || gain==1.f) assert(l[i]==dl[i]);
            if(!dl[i]){assert(!l[i]);++p->silent;}
            if(l[i]!=dl[i])++p->edited;
        }
        for(unsigned j=0;j<2;++j) assert(p->lfo[j].phase.curphs==p->wet.dsp.synthone.lfo[j].phase);
    }
    for(unsigned v=0;v<2;++v) {
        pair *p=&part[v];unsigned total=0;
        for(unsigned j=0;j<4;++j){ assert(p->frames[j]>10000);total+=p->frames[j];}
        assert(p->peak<2e-6 && p->edited>40000 && p->silent>12000);
        printf("owner=%c frames=%u off=%u lfo1=%u lfo2=%u average=%u silent=%u changed=%u peak=%.9g rms=%.9g\n",
            'A'+v,total,p->frames[0],p->frames[1],p->frames[2],p->frames[3],p->silent,p->edited,p->peak,sqrt(p->squared/total));
    }
    if(argc==2)factory(cell,argv[1]);
    return 0;
}
