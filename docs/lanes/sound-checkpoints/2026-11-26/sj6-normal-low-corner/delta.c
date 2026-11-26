#include "dnbx_core.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#if EXACT_NORMAL
#include "normal_coeff.h"
#include "leaf.h"
#endif
static dnbx_part s;
#if RAW_SAW_ABLATION
#include "sj6_main_4x.h"
extern sj6_main_4x eps_saw[2];
#endif
static float out_l[128],out_r[128];
float diagnostic_source[2][128];
static void one(unsigned wave,unsigned layer,unsigned note)
{
    assert(dnbx_init_superj6(&s,44100)==DNBX_OK);
#if RAW_SAW_ABLATION
    memset(eps_saw,0,sizeof eps_saw);
#endif
    for(unsigned l=0;l<2;l++){
        dnbx_layer_params p;assert(!dnbx_get_layer(&s,l,&p));
        p.gain=1.f;p.pan=0.f;p.waveform=wave/3.f;p.oscillator_mode=DNBX_OSC_SUPERJ6;
        p.cutoff_hz=20000.f;p.resonance=0.f;p.filter_env_hz=0.f;p.sub_gain=0.f;p.pulse_width=.5f;
        p.amp.attack_s=.002f;p.amp.decay_s=.002f;p.amp.sustain=1.f;p.amp.release_s=.002f;
        assert(!dnbx_set_layer(&s,l,&p));
    }
    const uint8_t aux[2]={0};dnbx_set_sj6_sources(&s,0,aux);
#if EXACT_NORMAL
    sj6_normal_hist history={0};
    const sj6_normal_coeff p3={0x1.851d9c7caa4aap-1,0x1.851d9c7caa4aap+0,0x1.851d9c7caa4aap-1,0x1.7cb2883c57ccap+0,0x1.1b116179f9916p-1};
    const sj6_normal_coeff p4={0x1.cb5f067432c00p-12,0x1.cb5f067432c00p-11,0x1.cb5f067432c00p-12,-0x1.ea91fe81f06abp+0,0x1.d609ac871aeedp-1};
    const sj6_normal_coeff profile=note==57?p3:p4;
    const nf_coef f3={(float)p3.b0,(float)p3.b1,(float)p3.b2,(float)(1.+p3.a1+p3.a2),(float)p3.a2};
    const nf_coef f4={(float)p4.b0,(float)p4.b1,(float)p4.b2,(float)(1.+p4.a1+p4.a2),(float)p4.a2};
    nf_hist hn={0};
    nf_trans ht={0};
    double full=0,e2=0,et=0,step_e2=0,step_et=0,step_total=0;
    float max_err=0,max_t=0;
    unsigned bit=0;
    double release_energy=0,release_err=0;
    int absolute=0;
#endif
    double q=0,peak=0,sc[8]={0},ss[8]={0}, dc=0;unsigned n=0;
    for(unsigned p=0;p<50000;p+=128){
        if(p==16384)assert(dnbx_note_on(&s,note,87)==0);
        if(p==40960)assert(dnbx_note_off(&s,note)==DNBX_OK);
        memset(diagnostic_source,0,sizeof diagnostic_source);
        assert(dnbx_render_pitch_osc_levels_velocity(&s,out_l,out_r,128,1.f,layer?0.f:.808f,layer?.808f:0.f,0.f)==DNBX_OK);
        for(unsigned k=0;k<128;k++){
            const float raw=diagnostic_source[layer][k];
#if EXACT_NORMAL
            absolute++;
            const sj6_normal_coeff *c=EXACT_NORMAL==2 && absolute>27008?(note==57?&p4:&p3):&profile;
            const nf_coef *fc=c==&p3?&f3:c==&p4?&f4:note==57?&f3:&f4;
            const float a=sj6_normal_receiver(&history,c,raw);
            const float b=normal_delta(&hn,fc,raw);
            const float z=normal_trans(&ht,fc,raw);
            const float diff=a-b, dift=a-z;
            full+=(double)a*a;e2+=(double)diff*diff;et+=(double)dift*dift;bit+=a==b;
            if(absolute>27000&&absolute<27265){step_total+=(double)a*a;step_e2+=(double)diff*diff;step_et+=(double)dift*dift;}
            if(absolute>=40960){release_energy+=(double)a*a;release_err+=(double)diff*diff;}
            max_err=fmaxf(max_err,fabsf(diff));max_t=fmaxf(max_t,fabsf(dift));
#else
            const float a=raw;
#endif
            if(p+k<16384+4410||p+k>=16384+22050)continue;
            assert(diagnostic_source[layer^1][k]==0.f);
            ++n; q+=(double)a*a;dc+=a;peak=fmax(peak,fabs(a));
            const double rad=6.2831853071795864769253*(note==57?220:440)*(p+k-16384)/44100;
            for(unsigned h=0;h<8;h++){sc[h]+=a*cos((h+1)*rad);ss[h]+=a*sin((h+1)*rad);}
        }
    }
    assert(n==17640);
    printf("%u %u %u %.12f %.12f %.12f",wave,layer+1,note,sqrt(q/n),peak,dc/n);
    for(unsigned h=0;h<8;h++)printf(" %.12f %.12f",2*sc[h]/n,2*ss[h]/n);
#if EXACT_NORMAL
    printf(" DB %.6f %.6f step %.6f %.6f max %.12f %.12f same %u/%d release %.6f",10*log10(e2/full),10*log10(et/full),10*log10(step_e2/step_total),10*log10(step_et/step_total),max_err,max_t,bit,absolute,10*log10(release_err/release_energy));
#endif
    putchar('\n');
}
int main(void){
    for(unsigned w=0;w<4;w++)for(unsigned l=0;l<2;l++){ one(w,l,57);one(w,l,69); }
}
