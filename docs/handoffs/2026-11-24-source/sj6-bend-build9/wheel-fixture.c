/* SPDX-License-Identifier: GPL-3.0-only */
#define ARP_FIXTURE_MAIN retained_arp_main
#include "ui_arp_engine_test.c"
#include "juno6.h"
#include <math.h>
#include "s1c_part.h"
static struct { unsigned frames; int wheel[2]; uint16_t wheel_mod_seq; } fm1_in;
static int clamp(int v,int lo,int hi){return v<lo?lo:v>hi?hi:v;}
#include "wheels.inc"
static juno6_t *j;
static s1_dry_part *s;
void __real_juno6_init(juno6_t *,float);
void __wrap_juno6_init(juno6_t *p,float rate){__real_juno6_init(p,rate);j=p;}
void __real_s1_part_init(s1_dry_part *);
void __wrap_s1_part_init(s1_dry_part *p){__real_s1_part_init(p);s=p;}
static void adc(int pitch)
{
    fm1_in.wheel[0]=pitch;fm1_in.wheel[1]=615;fm1_in.frames=301;
    smk_wheels();render(4);
}
static void select_part(unsigned v,unsigned bank)
{
    EXPECT(!engine_bass_select_preset(v,bank,0,&proj.sound));
    for(unsigned i=0;i<1024&&engine_bass_preset_pending(v);++i)render(1);
    EXPECT(engine_bass_bank(v)==(int)bank);
}
static void expect_layers(const juno6_t *j, const float base[2], const float steps[2],
                          unsigned held, float bend)
{
    const dnbx_voice *note = &j->core.voice[held];
    EXPECT(note->gate && note->note==48);
    for(unsigned l=0;l<DNBX_LAYERS;++l) {
        dnbx_layer_params p;
        EXPECT(dnbx_get_layer(&j->core,l,&p)==DNBX_OK);
        EXPECT(fabsf(p.tune_semitones-base[l]-bend)<.000001f);
        EXPECT(fabsf(note->layer[l].increment/steps[l]-exp2f(bend/12.f))<.000004f);
    }
}
static void check_sj6(unsigned v)
{
    const unsigned peer=1-v;
    select_part(peer,ORIGINAL_SOUND_SYNTHONE);s1_dry_part *other=s;
    select_part(v,ORIGINAL_SOUND_JUNO);juno6_t *part=j;
    EXPECT(part && other);
    float detm=other->e.t[31];
    sound_t saved=proj.sound;
    const unsigned writes=store_writes;
    juno6_set_layer_semi(part,1,SJ6_SEMI_CENTER+3);
    ui.part=PART_303A+v;adc(500);
    engine_bass_on(v,48,0,0);render(1);
    unsigned held=0;
    while(held<DNBX_VOICES && !part->core.voice[held].gate) held++;
    if(held==DNBX_VOICES) { EXPECT(0); return; }
    const float base[]={part->core.layer[0].params.tune_semitones,part->core.layer[1].params.tune_semitones};
    const float steps[]={part->core.voice[held].layer[0].increment,part->core.voice[held].layer[1].increment};
    EXPECT(base[1]-base[0]==3.f);

    adc(250);EXPECT(engine_bass_perf(v,0,1)==0 && part->macro[16]==0.f);
    expect_layers(part,base,steps,held,-2.f);
    ui.part=PART_303A+peer;adc(750);
    EXPECT(engine_bass_perf(v,0,1)==127 && part->macro[16]==1.f);
    expect_layers(part,base,steps,held,2.f);
    EXPECT(other->e.t[31]==detm);
    adc(500);EXPECT(engine_bass_perf(v,0,1)==64 && part->macro[16]==.5f);
    expect_layers(part,base,steps,held,0.f);
    EXPECT(other->e.t[31]==detm);
    static const unsigned raw[]={0,1,127,8190,8191,8192,8193,12288,16255,16382,16383};
    for(unsigned i=0;i<sizeof raw/sizeof *raw;++i) {
        unsigned ch=seq.ch[TRK_BASS0+v];
        host_midi_put(0xeu | ((0xe0u|ch)<<8) | ((raw[i]&127u)<<16) | (raw[i]>>7)<<24);
        render(1);
        EXPECT(part->macro[16]==(float)raw[i]/16384.f && other->e.t[31]==detm);
        expect_layers(part,base,steps,held,4.f*((float)raw[i]/16384.f-.5f));
    }
    EXPECT(!memcmp(&saved,&proj.sound,sizeof(saved)) && store_writes==writes);
    engine_bass_off(v);render(4);
}
int main(void)
{
    setup(V_PART,PART_303A);
    check_sj6(0);check_sj6(1);
    static const unsigned kinds[]={ORIGINAL_SOUND_CHICAGO,ORIGINAL_SOUND_MIAMI,ORIGINAL_SOUND_SYNTHONE,ORIGINAL_SOUND_JUNO};
    static const unsigned ids[]={ORIGINAL_CHICAGO_PITCH_BEND,ORIGINAL_MIAMI_PITCH_BEND,0,16};
    for(unsigned i=0;i<4;++i)for(unsigned v=0;v<2;++v){
        select_part(v,kinds[i]);
        EXPECT(smk_pitch_parameter(v)==(int)ids[i]);
    }
    if(failures)return 1;
    printf("PASS Build9 SJ6 held wheel/pitch MIDI both owners and layers, no writes or other-engine identity changes (%u)\n",expectations);
    return 0;
}
