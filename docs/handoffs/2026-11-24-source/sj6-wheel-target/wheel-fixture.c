/* SPDX-License-Identifier: GPL-3.0-only */
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
static void check_sj6(unsigned v)
{
    const unsigned peer=1-v;
    select_part(peer,ORIGINAL_SOUND_SYNTHONE);s1_dry_part *other=s;
    select_part(v,ORIGINAL_SOUND_JUNO);juno6_t *part=j;
    EXPECT(part && other);
    float detm=other->e.t[31];
    sound_t saved=proj.sound;
    const unsigned writes=store_writes;
    ui.part=PART_303A+v;adc(500);
    engine_bass_on(v,48,0,0);render(1);
    adc(250);EXPECT(engine_bass_perf(v,0,1)==0 && part->macro[16]==0.f);
    ui.part=PART_303A+peer;adc(750);
    EXPECT(engine_bass_perf(v,0,1)==127 && part->macro[16]==1.f);
    EXPECT(other->e.t[31]==detm);
    adc(500);EXPECT(engine_bass_perf(v,0,1)==64 && part->macro[16]==.5f);
    EXPECT(other->e.t[31]==detm);
    for(unsigned i=0;i<3;++i) {
        static const unsigned raw[]={0,8192,16383};
        unsigned ch=seq.ch[TRK_BASS0+v];
        host_midi_put(0xeu | ((0xe0u|ch)<<8) | ((raw[i]&127u)<<16) | (raw[i]>>7)<<24);
        render(1);
        EXPECT(part->macro[16]==(float)i*.5f && other->e.t[31]==detm);
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
    printf("PASS real SMK wheel both SJ6 owners and unchanged other-engine identities (%u)\n",expectations);
    return 0;
}
