/* SPDX-License-Identifier: GPL-3.0-only */
/* The exact SMK wheels function and real audio/MIDI queue, selected Init route. */
#define ARP_FIXTURE_MAIN retained_arp_main
#include "ui_arp_engine_test.c"
#include "juno6.h"
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
static void adc_mod(int mod)
{
    fm1_in.wheel[0]=500; fm1_in.wheel[1]=mod;
    fm1_in.frames=301; ++fm1_in.wheel_mod_seq;
    smk_wheels(); render(4);
}
static void select_kind(unsigned v,unsigned bank)
{
    EXPECT(!engine_bass_select_preset(v,bank,0,&proj.sound));
    for(unsigned i=0;i<1024&&engine_bass_preset_pending(v);++i)render(1);
    EXPECT(engine_bass_bank(v)==(int)bank);
}
static void cc1(unsigned v,unsigned value)
{
    unsigned ch=seq.ch[TRK_BASS0+v];
    host_midi_put(0xbu | ((0xb0u|ch)<<8) | (1u<<16) | (value<<24));
    render(4);
}
static void wheel_case(unsigned v)
{
    unsigned peer=1-v;
    select_kind(peer,ORIGINAL_SOUND_SYNTHONE); s1_dry_part *other=s;
    select_kind(v,ORIGINAL_SOUND_JUNO); juno6_t *part=j;
    EXPECT(part&&other);
    const float cut=other->e.t[16], before=part->core.layer[0].params.cutoff_hz;
    sound_t saved=proj.sound; unsigned writes=store_writes;
    juno6_oscillator_edit square={1.f,.43f,1.f};
    juno6_apply_pre_note_wave(part,v,&square);
    juno6_set_panel_lfo(part,0,77,1);
    EXPECT(juno6_configure_lfo1_pwm(part,69u+v,1,.43f,0.f));
    EXPECT(juno6_set_mod_destination(part,2));
    ui.part=PART_303A+v; adc_mod(615);
    engine_bass_on(v,48,0,0); render(4);
    adc_mod(485);
    EXPECT(engine_bass_perf(v,2,1)==127&&part->wheel_lfo1_amount==128);
    ui.part=PART_303A+peer; adc_mod(550);
    int value=engine_bass_perf(v,2,1);
    EXPECT(value>0&&value<127&&part->wheel_lfo1_amount==value+1);
    cc1(v,(unsigned)value);
    EXPECT(part->wheel_lfo1_amount==value+1);
    adc_mod(615);
    EXPECT(engine_bass_perf(v,2,1)==0&&part->wheel_lfo1_amount==1);
    for(unsigned u=0;u<128;u++){
        cc1(v,u);
        EXPECT(part->wheel_lfo1_amount==u+1&&part->core.raw[89]==0.f);
        EXPECT(part->core.layer[0].params.cutoff_hz==before&&other->e.t[16]==cut);
    }
    cc1(v,0);
    EXPECT(!memcmp(&saved,&proj.sound,sizeof(saved)) && store_writes==writes);
    engine_bass_off(v); render(16);
}
int main(void)
{
    setup(V_PART,PART_303A);
    wheel_case(0); wheel_case(1);
    if(failures)return 1;
    printf("PASS real physical SMK MOD and incoming CC1 on both SuperJ6 owners, actual original LFO1 amount routed to PWM, held ownership and exact saved bytes (%u)\n",expectations);
    return 0;
}
