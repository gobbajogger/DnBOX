/* SPDX-License-Identifier: GPL-3.0-only */
#include "app/original_runtime.h"
#include "soundpipe.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

typedef struct {
    original_runtime_part wet, dry;
    sp_phaser *original;
    sp_port port[4];
    float target[4];
    unsigned frames, bypassed, changed, input_silent;
    double peak, squared, reference_power;
} pair;
static pair parts[2];
static original_runtime_workspace workspace;
static sp_data sp={.sr=44100,.nchan=2};

static void init(pair *p,const uint8_t cell[32])
{
    memset(p,0,sizeof *p);
    assert(original_runtime_init(&p->wet,cell,120,NULL,NULL)==ORIGINAL_RUNTIME_OK);
    assert(original_runtime_init(&p->dry,cell,120,NULL,NULL)==ORIGINAL_RUNTIME_OK);
    sp_phaser_create(&p->original);
    sp_phaser_init(&sp,p->original);
    *p->original->MinNotch1Freq=100;
    *p->original->MaxNotch1Freq=800;
    *p->original->NotchFreq=1.5f;
    *p->original->VibratoMode=1;
    *p->original->depth=1;
    *p->original->invert=0;
    for(unsigned k=0;k<4;++k)sp_port_init(&sp,&p->port[k],.1f);
    p->target[1]=12.f; p->target[3]=800.f;
}
static void set(pair *p,unsigned slot,unsigned q)
{
    assert(original_runtime_param(&p->wet,ORIGINAL_SYNTHONE_PHASER_MIX+slot,(float)q/127.f)==ORIGINAL_RUNTIME_OK);
    p->target[slot]=s1_phaser_control(slot,q);
}
static void note(pair *p,unsigned key,unsigned vel)
{
    assert(original_runtime_key(&p->wet,ORIGINAL_INPUT_KEYS,key,vel)==ORIGINAL_RUNTIME_OK);
    assert(original_runtime_key(&p->dry,ORIGINAL_INPUT_KEYS,key,vel)==ORIGINAL_RUNTIME_OK);
}
static void render(pair *p,unsigned n,original_runtime_part *panel,double *peak,double *sum)
{
    float l[128],r[128],dl[128],dr[128];
    assert(original_runtime_render(&p->wet,&workspace,l,r,n)==ORIGINAL_RUNTIME_OK);
    assert(original_runtime_render(&p->dry,&workspace,dl,dr,n)==ORIGINAL_RUNTIME_OK);
    float pl[128],pr[128];
    if(panel)assert(original_runtime_render(panel,&workspace,pl,pr,n)==ORIGINAL_RUNTIME_OK);
    for(unsigned i=0;i<n;++i){
        float w[4],sl=dl[i],sr=dr[i];
        for(unsigned k=0;k<4;++k)sp_port_compute(&sp,&p->port[k],&p->target[k],&w[k]);
        *p->original->lfobpm=w[1];*p->original->feedback_gain=w[2];*p->original->Notch_width=w[3];
        if(w[0]!=0.f) {
            sp_phaser_compute(&sp,p->original,&dl[i],&dr[i],&sl,&sr);
            float dry=1.f-w[0];sl=dry*dl[i]+(1.f-dry)*sl;sr=dry*dr[i]+(1.f-dry)*sr;
        } else ++p->bypassed;
        if(panel){
            double el=(double)pl[i]-sl, er=(double)pr[i]-sr;
            assert(isfinite(el)&&isfinite(er)); *peak=fmax(*peak,fmax(fabs(el),fabs(er)));*sum+=el*el+er*er;
        }
        double el=(double)l[i]-sl,er=(double)r[i]-sr;
        assert(isfinite(el)&&isfinite(er));
        p->peak=fmax(p->peak,fmax(fabs(el),fabs(er)));
        p->squared+=el*el+er*er;p->reference_power+=(double)sl*sl+(double)sr*sr;
        ++p->frames;p->changed+=l[i]!=dl[i]||r[i]!=dr[i];p->input_silent+=dl[i]==0.f&&dr[i]==0.f;
    }
    for(unsigned k=0;k<4;++k)assert(p->port[k].yt1==p->wet.dsp.synthone.phaser.current[k]);
}
static void factory(const uint8_t cell[32],const char *targets)
{
    FILE *f=fopen(targets,"r");assert(f);
    float t[4];
    unsigned programs=0, wet=0, rounded=0,worst=0;
    double src_peak=0.,src_rms=0.,panel_peak=0.,panel_rms=0.;
    while(fscanf(f,"%a %a %a %a",t,t+1,t+2,t+3)==4) {
        pair *p=&parts[0];init(p,cell);
        original_runtime_part *panel=&parts[1].wet;
        assert(original_runtime_init(panel,cell,120,NULL,NULL)==ORIGINAL_RUNTIME_OK);
        unsigned different=0;
        for(unsigned k=0;k<4;++k) {
            p->target[k]=p->wet.dsp.synthone.phaser.target[k]=t[k];
            unsigned best=0;
            for(unsigned q=1;q<128;++q)if(fabsf(t[k]-s1_phaser_control(k,q))<fabsf(t[k]-s1_phaser_control(k,best))) best=q;
            different|=t[k]!=s1_phaser_control(k,best);
            assert(original_runtime_param(panel,ORIGINAL_SYNTHONE_PHASER_MIX+k,(float)best/127.f)==ORIGINAL_RUNTIME_OK);
        }
        rounded+=different;wet+=t[0]!=0.f;
        note(p,60,110);assert(original_runtime_key(panel,ORIGINAL_INPUT_KEYS,60,110)==ORIGINAL_RUNTIME_OK);
        double sum=0.;
        while(p->frames<44100)render(p,44100-p->frames>128?128:44100-p->frames,panel,&panel_peak,&sum);
        src_peak=fmax(src_peak,p->peak);src_rms=fmax(src_rms,sqrt(p->squared/(2.*44100.)));
        double rms=sqrt(sum/(2.*44100.));if(rms>panel_rms){panel_rms=rms;worst=programs;}
        sp_phaser_destroy(&p->original);++programs;
    }
    fclose(f);
    printf("programs=%u frames=%u wet=%u source_peak=%.9g source_worst_rms=%.9g panel_rounded=%u panel_peak=%.9g panel_worst_rms=%.9g panel_worst_index=%u\n",
        programs,programs*44100u,wet,src_peak,src_rms,rounded,panel_peak,panel_rms,worst);
    assert(programs==694&&wet>0&&src_peak<.002&&src_rms<.0002);
}
int main(int argc,char **argv)
{
    uint8_t cell[32];assert(original_sound_init(cell,ORIGINAL_SOUND_SYNTHONE,0));
    for(unsigned v=0;v<2;++v)init(&parts[v],cell);
    for(unsigned b=0;b<2200;++b)for(unsigned v=0;v<2;++v){
        pair *p=&parts[v];
        if(b==3)note(p,52u+v*15u,110);
        if(b==18)set(p,3,v?0:127);
        if(b==63)set(p,1,v?127:0);
        if(b==95)set(p,0,v?75:127);
        if(b==125)set(p,2,v?127:63);
        if(b==220) {
            unsigned crush=v?43:95, pan=v?77:124;
            assert(original_runtime_param(&p->wet,ORIGINAL_SYNTHONE_CRUSH,(float)crush/127.f)==ORIGINAL_RUNTIME_OK);
            assert(original_runtime_param(&p->dry,ORIGINAL_SYNTHONE_CRUSH,(float)crush/127.f)==ORIGINAL_RUNTIME_OK);
            assert(original_runtime_param(&p->wet,ORIGINAL_SYNTHONE_PAN,(float)pan/127.f)==ORIGINAL_RUNTIME_OK);
            assert(original_runtime_param(&p->dry,ORIGINAL_SYNTHONE_PAN,(float)pan/127.f)==ORIGINAL_RUNTIME_OK);
        }
        if(b==320) {
            assert(original_runtime_all_off(&p->wet)==ORIGINAL_RUNTIME_OK);
            assert(original_runtime_all_off(&p->dry)==ORIGINAL_RUNTIME_OK);
            assert(!original_runtime_owned(&p->wet) && original_runtime_active(&p->wet));
        }
        if(b==321)note(p,52u+v*15u,110);
        if(b==358)set(p,3,v?95:23);
        if(b==437)set(p,1,v?0:127);
        if(b==602)set(p,2,v?0:127);
        if(b==760)note(p,52u+v*15u,0);
        if(b==1050){assert(original_runtime_all_off(&p->wet)==ORIGINAL_RUNTIME_OK);assert(original_runtime_all_off(&p->dry)==ORIGINAL_RUNTIME_OK);}
        if(b==1132)set(p,0,0);
        if(b==1351)set(p,0,114-14*v);
        if(b==1432)note(p,57u+6u*v,98);
        render(p,1u+(b*19u+v*23u)%128u,NULL,NULL,NULL);
    }
    for(unsigned v=0;v<2;++v) {
        pair *p=&parts[v];
        printf("owner=%c frames=%u bypass=%u input_silent=%u changed=%u peak=%.9g rms=%.9g signal_db=%.6g\n",'A'+v,p->frames,p->bypassed,p->input_silent,p->changed,p->peak,sqrt(p->squared/(2*p->frames)),10.*log10(p->squared/p->reference_power));
        assert(p->peak<.002 && 10.*log10(p->squared/p->reference_power)<-70. && p->changed>80000 && p->input_silent>10000 && p->bypassed>5000);
        sp_phaser_destroy(&p->original);
    }
    if(argc==2)factory(cell,argv[1]);
    return 0;
}
