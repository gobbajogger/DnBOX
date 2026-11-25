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
 select_kind(peer,ORIGINAL_SOUND_SYNTHONE);
 select_kind(v,ORIGINAL_SOUND_JUNO);
 juno6_t *part=j;
 ui.part=PART_303A+v; build_pages();
 int pgidx=page_named("MOD / PITCH WHEELS");
 EXPECT(pgidx>=0); if(pgidx<0)return;
 for(int k=0;k<24&&cur_page()!=pgidx;k++)tap_panel(B_EDIT);
 EXPECT(cur_page()==pgidx);
 EXPECT(pg.n<=23); const pref_t selector=pg.r[pgidx][0];
 EXPECT(pref_get(selector)==0);
 const x0x_param_t *d=pref_desc(selector); static const char *const exact[]={"LFO Rate","LFO>DCO","LFO>Cutoff","Tremolo","LFO2 Rate","CUTOFF"};
 EXPECT(d&&d->max==5&&d->names);
 if(d&&d->names)for(unsigned i=0;i<6;++i)EXPECT(!strcmp(exact[i],d->names[i]));
 sound_t saved=proj.sound;unsigned writes=store_writes;int dirty=ui.dirty;
 juno6_set_panel_lfo(part,0,77,1);
 EXPECT(juno6_configure_lfo1_cutoff(part,.32f,.7f,0.f));
 engine_bass_on(v,48,0,0); render(4);
 uint64_t r0=part->lfo_step_q40[0];cc1(v,92);
 EXPECT(part->mod_wheel_lfo1==93&&part->lfo_step_q40[0]!=r0);
 uint64_t rate=part->lfo_step_q40[0];float vib=part->vibrato_depth;
 int wheel_pos=engine_bass_perf(v,2,1);
 turn(0,1); EXPECT(pref_get(selector)==1&&part->mod_destination==1);
 EXPECT(part->mod_wheel_lfo1==93&&part->lfo_step_q40[0]==rate&&part->vibrato_depth==vib&&engine_bass_perf(v,2,1)==wheel_pos);
 cc1(v,88); EXPECT(part->vibrato_depth!=vib);
 turn(0,1); EXPECT(pref_get(selector)==2&&part->mod_destination==2&&part->wheel_lfo1_amount==0);
 cc1(v,85);EXPECT(part->wheel_lfo1_amount==86&&part->core.raw[89]==0.f);
 EXPECT(!juno6_arm_alt3_free(part,10000,1));
 EXPECT(juno6_arm_alt3_free(part,10000,0));
 turn(0,1); EXPECT(pref_get(selector)==3&&part->mod_destination==3&&part->alt3.to_depth==0.f);
 cc1(v,79);EXPECT(part->alt3.to_depth==79.f/127.f);
 turn(0,1); EXPECT(pref_get(selector)==4&&part->mod_destination==4&&part->mod_wheel_lfo2==0);
 cc1(v,96);EXPECT(part->mod_wheel_lfo2==97);
 turn(0,1); EXPECT(pref_get(selector)==5&&part->mod_destination==5);
 adc_mod(615);adc_mod(550); EXPECT(part->mod_wheel_cutoff_hz>20.f);
 turn(0,-1);EXPECT(pref_get(selector)==4&&part->mod_destination==4);
 pref_set_impl(selector,6,0);render(4);EXPECT(pref_get(selector)==4);
 EXPECT(juno6_set_mod_destination(part,6)&&pref_get(selector)==0);

 EXPECT(!memcmp(&saved,&proj.sound,sizeof(saved))&&store_writes==writes&&ui.dirty==dirty);
 adc_mod(615);engine_bass_off(v);render(16);
 ui.part=PART_303A+peer;build_pages();EXPECT(page_named("MOD / PITCH WHEELS")<0);
}
int main(void){setup(V_PART,PART_303A);wheel_case(0);wheel_case(1);if(failures)return 1;printf("PASS physical original MOD selector (%u)\n",expectations);return 0;}
