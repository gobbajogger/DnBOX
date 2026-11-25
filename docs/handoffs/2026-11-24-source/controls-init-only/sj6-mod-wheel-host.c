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
    juno6_lfo_view entry, exit;
    juno6_read_lfo(part,0,&entry);
    ui.part=PART_303A+v; adc_mod(615);
    engine_bass_on(v,48,0,0); render(4);
    adc_mod(485);
    EXPECT(part->mod_wheel_lfo1==128 && engine_bass_perf(v,2,1)==127);
    uint64_t fast=part->lfo_step_q40[0];
    EXPECT(fast>0 && part->core.layer[0].params.cutoff_hz==before && other->e.t[16]==cut);
    ui.part=PART_303A+peer; adc_mod(550);
    uint64_t mid=part->lfo_step_q40[0];
    int value=engine_bass_perf(v,2,1);
    EXPECT(value>0 && value<127 && part->mod_wheel_lfo1==value+1 && mid!=fast);
    EXPECT(other->e.t[16]==cut && part->core.layer[0].params.cutoff_hz==before);
    cc1(v,(unsigned)value);
    EXPECT(part->mod_wheel_lfo1==value+1 && part->lfo_step_q40[0]==mid);
    adc_mod(615);
    EXPECT(part->mod_wheel_lfo1==1 && engine_bass_perf(v,2,1)==0);
    EXPECT(other->e.t[16]==cut && part->core.layer[0].params.cutoff_hz==before);
    juno6_read_lfo(part,0,&exit);
    EXPECT(entry.rate==exit.rate && entry.mode==exit.mode && entry.present==exit.present);
    EXPECT(!memcmp(&saved,&proj.sound,sizeof(saved)) && store_writes==writes);
    engine_bass_off(v); render(16);
}
int main(void)
{
    setup(V_PART,PART_303A);
    wheel_case(0); wheel_case(1);
    if(failures)return 1;
    printf("PASS real physical SMK MOD and incoming CC1 on both SuperJ6 owners, transient default LFO1 rate, held ownership and exact saved bytes (%u)\n",expectations);
    return 0;
}
