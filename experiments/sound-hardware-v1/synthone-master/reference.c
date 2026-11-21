/* SPDX-License-Identifier: GPL-3.0-only */
/* Original dual-mono Soundpipe compressor and parameter port on the same stereo
 * source at the stock master insert. The last right-channel width tap is dry. */
#include "app/original_runtime.h"
#include "soundpipe.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

typedef struct { double peak,squared,signal; } error;
typedef struct {
    original_runtime_part dry,wet,panel;
    sp_compressor *channel[2]; sp_port volume;
    float target[6];
    error exact,rounded;
    unsigned frames,changed,input_silent,reducing,active;
} run;
static run owner[2];
static original_runtime_workspace workspace;
static sp_data sp={.sr=44100,.nchan=2};
static uint8_t cell[32];
static const unsigned defaults[6]={32,0,127,0,0,18};

static void init(run *s)
{
    memset(s,0,sizeof *s);
    assert(original_runtime_init(&s->dry,cell,120,NULL,NULL)==ORIGINAL_RUNTIME_OK);
    assert(original_runtime_init(&s->wet,cell,120,NULL,NULL)==ORIGINAL_RUNTIME_OK);
    assert(original_runtime_init(&s->panel,cell,120,NULL,NULL)==ORIGINAL_RUNTIME_OK);
    sp_port_init(&sp,&s->volume,.1f);
    for(unsigned i=0;i<6;++i)s->target[i]=s1_master_control(i,defaults[i]);
    for(unsigned c=0;c<2;++c){sp_compressor_create(&s->channel[c]);sp_compressor_init(&sp,s->channel[c]);}
}
static void done(run *s) {for(unsigned c=0;c<2;++c)sp_compressor_destroy(&s->channel[c]);}
static void key(run *s,unsigned k,unsigned v)
{
    assert(original_runtime_key(&s->dry,ORIGINAL_INPUT_KEYS,k,v)==ORIGINAL_RUNTIME_OK);
    assert(original_runtime_key(&s->wet,ORIGINAL_INPUT_KEYS,k,v)==ORIGINAL_RUNTIME_OK);
    assert(original_runtime_key(&s->panel,ORIGINAL_INPUT_KEYS,k,v)==ORIGINAL_RUNTIME_OK);
}
static unsigned control_q(unsigned slot,float physical)
{
    unsigned best=0;
    for(unsigned q=1;q<128;++q)if(fabsf(physical-s1_master_control(slot,q))<fabsf(physical-s1_master_control(slot,best)))best=q;
    return best;
}
static void control(run *s,unsigned k,unsigned q)
{
    assert(original_runtime_param(&s->wet,ORIGINAL_SYNTHONE_MASTER_VOLUME+k,(float)q/127.f)==ORIGINAL_RUNTIME_OK);
    s->target[k]=s1_master_control(k,q);s->active=1;
}
static void score(error *e,float out,float ref)
{
    double d=(double)out-ref;assert(isfinite(d));
    e->peak=fmax(e->peak,fabs(d));e->squared+=d*d;e->signal+=(double)ref*ref;
}
static void block(run *s,unsigned n,unsigned compare_panel)
{
    float dry[2][128],wet[2][128],panel[2][128];
    assert(original_runtime_render(&s->dry,&workspace,dry[0],dry[1],n)==ORIGINAL_RUNTIME_OK);
    assert(original_runtime_render(&s->wet,&workspace,wet[0],wet[1],n)==ORIGINAL_RUNTIME_OK);
    if(compare_panel)assert(original_runtime_render(&s->panel,&workspace,panel[0],panel[1],n)==ORIGINAL_RUNTIME_OK);
    for(unsigned c=0;c<2;++c){sp_compressor *p=s->channel[c];*p->ratio=s->target[1];*p->thresh=s->target[2];*p->atk=s->target[3];*p->rel=s->target[4];}
    for(unsigned i=0;i<n;++i) {
        float volume=0.f;if(s->active)sp_port_compute(&sp,&s->volume,s->target,&volume);
        for(unsigned c=0;c<2;++c) {
            float raw=dry[c][i],ref=raw;
            if(s->active) {
                float in=raw*(2.f*volume),comp;
                sp_compressor_compute(&sp,s->channel[c],&in,&comp);ref=comp*s->target[5];
                s->reducing+=fabsf(comp)<.99f*fabsf(in);
            }
            score(&s->exact,wet[c][i],ref);
            if(compare_panel)score(&s->rounded,panel[c][i],ref);
        }
        s->changed+=dry[0][i]!=wet[0][i] || dry[1][i]!=wet[1][i];
        s->input_silent+=dry[0][i]==0.f && dry[1][i]==0.f;
    }
    s->frames+=n;
    if(s->active)assert(s->volume.yt1==s->wet.dsp.synthone.master.volume);
}
static void factory(const char *path)
{
    FILE *f=fopen(path,"r");assert(f);float t[6];
    unsigned n=0,vectors=0,worst=0,attenuated=0;double peak=0,rms=0,qpeak=0,qrms=0;
    while(fscanf(f,"%a %a %a %a %a %a",t,t+1,t+2,t+3,t+4,t+5)==6) {
        run *s=&owner[0];init(s);
        memcpy(s->target,t,sizeof t);s->active=1;
        s->wet.dsp.synthone.master.active=63;s1_master_targets(&s->wet.dsp.synthone.master,t);
        unsigned diff=0;
        for(unsigned k=0;k<6;++k){
            unsigned q=control_q(k,t[k]);diff|=s1_master_control(k,q)!=t[k];
            assert(original_runtime_param(&s->panel,ORIGINAL_SYNTHONE_MASTER_VOLUME+k,(float)q/127.f)==ORIGINAL_RUNTIME_OK);
        }
        vectors+=diff;key(s,60,110);
        while(s->frames<44100)block(s,44100-s->frames>128?128:44100-s->frames,1);
        double r=sqrt(s->rounded.squared/88200.);
        if(r>qrms){qrms=r;worst=n;}
        qpeak=fmax(qpeak,s->rounded.peak);peak=fmax(peak,s->exact.peak);rms=fmax(rms,sqrt(s->exact.squared/88200.));
        attenuated+=s->reducing!=0;done(s);++n;
    }
    assert(feof(f));fclose(f);
    printf("programs=%u frames=%u detector_reduced=%u peak=%.9g worst_rms=%.9g panel_rounded=%u panel_peak=%.9g panel_worst_rms=%.9g panel_worst_index=%u\n",n,44100*n,attenuated,peak,rms,vectors,qpeak,qrms,worst);
    assert(n==694&&attenuated>0&&peak<.001&&rms<.0001);
}
int main(int argc,char **argv)
{
    assert(original_sound_init(cell,ORIGINAL_SOUND_SYNTHONE,0));
    for(unsigned v=0;v<2;++v)init(&owner[v]);
    for(unsigned b=0;b<1800;++b)for(unsigned v=0;v<2;++v) {
        run *s=&owner[v];
        if(b==10)key(s,49+v*11,117);
        if(b==26) {
            for(unsigned k=0;k<6;++k)control(s,k,(unsigned[]){123,127,25,13,38,54}[k]);
        }
        if(b==90)control(s,2,8+v*14);
        if(b==250)control(s,1,13+v*27);
        if(b==420)control(s,3,112-v*77);
        if(b==510)control(s,4,v?0:124);
        if(b==660)control(s,0,110-v*66);
        if(b==730)control(s,5,90+v*35);
        if(b==880) {
            assert(original_runtime_all_off(&s->wet)==ORIGINAL_RUNTIME_OK);
            assert(original_runtime_all_off(&s->dry)==ORIGINAL_RUNTIME_OK);
        }
        if(b==1040)key(s,72-v*7,119);
        if(b==1300)key(s,72-v*7,0);
        block(s,1+(b*19+v*13)%128,0);
    }
    for(unsigned v=0;v<2;++v) {
        run *s=&owner[v];double db=10.*log10(s->exact.squared/s->exact.signal);
        printf("owner=%c frames=%u changed=%u input_silent=%u reduced_samples=%u peak=%.9g rms=%.9g signal_db=%.6g\n",'A'+v,s->frames,s->changed,s->input_silent,s->reducing,s->exact.peak,sqrt(s->exact.squared/(2*s->frames)),db);
        assert(s->changed>50000 && s->input_silent>5000 && s->reducing>50000 && db < -65.);
        done(s);
    }
    if(argc==2)factory(argv[1]);
    return 0;
}
