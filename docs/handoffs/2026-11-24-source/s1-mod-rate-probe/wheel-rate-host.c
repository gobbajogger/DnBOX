/* SPDX-License-Identifier: GPL-3.0-only */
#include "synth_init_test_helpers.h"
#include "app/s1c_part.h"
#include "dsp/s1c/s1c_front.h"
#include <math.h>
static struct { unsigned frames; int wheel[2]; uint16_t wheel_mod_seq; } fm1_in;
static int clamp(int v,int lo,int hi) { return v<lo?lo:v>hi?hi:v; }
#include "mod-wheels.inc"
static s1_dry_part *current,*parts[2];
void __real_s1_part_init(s1_dry_part *);
void __wrap_s1_part_init(s1_dry_part *p) { __real_s1_part_init(p); current=p; }
enum { CUT=16, BEND=31, LFO1=15,LFO2=47 };
static void adc(int raw)
{
    fm1_in.wheel[0]=500;
    fm1_in.wheel[1]=raw;
    ++fm1_in.wheel_mod_seq; fm1_in.frames=301;
    smk_wheels(); render(1);
}
static void midi(unsigned owner,unsigned cc)
{
    unsigned ch=(unsigned)seq.ch[TRK_BASS0+owner];
    host_midi_put(0xbu | ((0xb0u|ch)<<8) | 1u<<16 | cc<<24);
    render(1);
    /* Consume the genuine external-MIDI owner/recordable guard. */
    smk_wheels();
}
static float target(unsigned cc,unsigned bpm)
{
    const double lo=1./1920., hi=80.;
    if (!bpm) return (float)(lo + pow((double)cc/127.,2.5)*(hi-lo));
    const double beats[]={32,24,16,12,8,4,4/1.5,2,2/1.5,1,1/1.5,.5,.5/1.5,.25,.25/1.5,.125,.125/1.5,.0625,.0625/1.5};
    double hz=bpm/60./beats[cc*18/127];
    if (hz>80. || (bpm==200 && cc==127)) hz=bpm/60./beats[17];
    return (float)hz;
}
int main(void)
{
    setup(V_PART,PART_303A); proj.set.bt|=PROJ_BT_CLK_IN;
    for(unsigned owner=0;owner<2;++owner) {
        current=NULL; select_patch(owner); CHECK(current); parts[owner]=current;
    }
    CHECK(parts[0]!=parts[1]);
    stabilize();
    const float cold[2][2]={{parts[0]->e.t[LFO1],parts[0]->e.t[LFO2]}, {parts[1]->e.t[LFO1],parts[1]->e.t[LFO2]}};
    adc(615);
    for(unsigned owner=0;owner<2;++owner) {
        unsigned peer=1-owner;
        for (unsigned route=1;route<=2;++route) {
        CHECK(s1_part_mod_route(parts[owner],route));
        CHECK(!s1_part_mod_route(parts[owner],3));
        unsigned id=route==2?LFO2:LFO1, other=route==2?LFO1:LFO2;
        float cut=parts[owner]->e.t[CUT];
        const sound_t start=proj.sound;
        unsigned writes=store_writes;
        for(unsigned j=0;j<3;++j) {
            const unsigned bpm=j==0?0:j==1?120:200;
            s1c_front_set(&parts[owner]->e,64,bpm?bpm:120);
            s1c_front_set(&parts[owner]->e,139,!!bpm);
            float in_owner=parts[owner]->e.t[other], peer_rates[]={parts[peer]->e.t[LFO1],parts[peer]->e.t[LFO2]};
            ui.part=(uint8_t)(PART_303A+owner);
            adc(485); CHECK(fabsf(parts[owner]->e.t[id]-target(127,bpm))<.0001f);
            ui.part=(uint8_t)(PART_303A+peer);
            adc(545);
            unsigned mid=(615u-545u-12u)*127u/(615u-485u-12u);
            float physical=parts[owner]->e.t[id];
            CHECK(fabsf(physical-target(mid,bpm))<.0001f);
            adc(615); CHECK(fabsf(parts[owner]->e.t[id]-target(0,bpm))<.0001f);
            midi(owner,mid); CHECK(physical==parts[owner]->e.t[id]);
            for(unsigned k=0;k<128;++k) {
                midi(owner,k);
                CHECK(fabsf(parts[owner]->e.t[id]-target(k,bpm))<.0001f);
                CHECK(in_owner==parts[owner]->e.t[other] && cut==parts[owner]->e.t[CUT]);
                CHECK(peer_rates[0]==parts[peer]->e.t[LFO1] && peer_rates[1]==parts[peer]->e.t[LFO2]);
            }
        }
        CHECK(!memcmp(&proj.sound,&start,sizeof start) && store_writes==writes);
        }
    }
    CHECK(!project_save() && !project_load());
    stabilize();
    for(unsigned owner=0;owner<2;++owner) {
        CHECK(parts[owner]->e.t[LFO1]==cold[owner][0]);
        CHECK(parts[owner]->e.t[LFO2]==cold[owner][1]);
        fprintf(stderr,"cold %u lfo %g %g expected %g %g route %u\n",owner,parts[owner]->e.t[LFO1],parts[owner]->e.t[LFO2],cold[owner][0],cold[owner][1],parts[owner]->mod_wheel_route);
        CHECK(parts[owner]->mod_wheel_route==0);
    }
    printf("PASS both routed LFO rates via real SMK ADC and MIDI, all 128 positions at free/120/200 BPM, part-switch latch, cold restore, unchanged serialized Sound; %u checks\n",checks);
}
