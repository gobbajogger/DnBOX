#include "dnbx_core.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
static dnbx_part s;
static float out_l[128],out_r[128];
float diagnostic_source[2][128];
static void one(int mode,unsigned wave,unsigned mask,unsigned velocity,unsigned sr,int unity)
{
    assert(dnbx_init_superj6(&s,sr)==DNBX_OK);
    for(unsigned l=0;l<2;l++){
        dnbx_layer_params p;
        assert(!dnbx_get_layer(&s,l,&p));
        p.gain=1.f;p.pan=0.f;p.waveform=wave==4u?(l?2.f/3.f:0.f):wave/3.f;p.oscillator_mode=mode;
        p.cutoff_hz=20000.f;p.resonance=0.f;p.filter_env_hz=0.f;p.sub_gain=0.f;
        p.amp.attack_s=.002f;p.amp.decay_s=.002f;p.amp.sustain=1.f;p.amp.release_s=.002f;
        assert(!dnbx_set_layer(&s,l,&p));
    }
    const uint8_t aux[2]={0};dnbx_set_sj6_sources(&s,0,aux);
    float fader=unity?1.f:.808f;
    float g1=mask&1?fader:0.f,g2=mask&2?fader:0.f;
    double q[3]={0},hcos[2]={0},hsin[2]={0};unsigned n=0;uint32_t sig=2166136261u;
    for(unsigned p=0;p<65536;p+=128){
        if(p==16384&&velocity)assert(dnbx_note_on(&s,69,velocity)==0);
        memset(diagnostic_source,0,sizeof diagnostic_source);
        assert(dnbx_render_pitch_osc_levels_velocity(&s,out_l,out_r,128,1.f,g1,g2,0.f)==DNBX_OK);
        for(unsigned k=0;k<128;k++){
            for(unsigned ch=0;ch<2;ch++){
                uint32_t v;memcpy(&v,ch?out_r+k:out_l+k,sizeof v);sig=(sig^v)*16777619u;
            }
            if(p+k>=25000&&p+k<25000+sr/2){
                const float a=diagnostic_source[0][k], b=diagnostic_source[1][k];
                q[0]+=(double)a*a;q[1]+=(double)b*b;q[2]+=(double)(a+b)*(a+b);n++;
                const double phase=6.2831853071795864769253*440*(p+k-16384)/sr;
                hcos[0]+=a*cos(phase);hsin[0]+=a*sin(phase);
                hcos[1]+=b*cos(phase);hsin[1]+=b*sin(phase);
            }
        }
    }
    printf("%d %u %u %u %.12f %.12f %.12f %08x %u %d %.12f %.12f %.12f %.12f %.12f %.12f\n",mode,wave,mask,velocity,sqrt(q[0]/n),sqrt(q[1]/n),sqrt(q[2]/n),sig,sr,unity,2*hypot(hcos[0],hsin[0])/n,2*hypot(hcos[1],hsin[1])/n,2*hcos[0]/n,2*hsin[0]/n,2*hcos[1]/n,2*hsin[1]/n);
}
int main(void)
{
    for(unsigned sr=44100;sr<=48000;sr+=3900){
        for(unsigned m=0;m<4;m++)one(DNBX_OSC_SUPERJ6,0,m,87,sr,0);
        for(unsigned w=1;w<4;w++)one(DNBX_OSC_SUPERJ6,w,3,87,sr,0);
        one(DNBX_OSC_STOCK_BLEP,0,3,87,sr,0);
        one(DNBX_OSC_SUPERJ6,0,3,0,sr,0);
        one(DNBX_OSC_SUPERJ6,4,3,87,sr,0);
        one(DNBX_OSC_SUPERJ6,0,3,87,sr,1);
    }
}
