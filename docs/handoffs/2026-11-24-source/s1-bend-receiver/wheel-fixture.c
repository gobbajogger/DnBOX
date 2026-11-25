/* SPDX-License-Identifier: GPL-3.0-only */
#define ARP_FIXTURE_MAIN retained_arp_main
#include "ui_arp_engine_test.c"
#include "s1c_part.h"
#include <math.h>

static struct { unsigned frames; int wheel[2]; uint16_t wheel_mod_seq; } fm1_in;
static int clamp(int v,int lo,int hi) { return v<lo?lo:v>hi?hi:v; }
enum { S1_DETM_ID=31 };
#include "wheels.inc"
static s1_dry_part *current_part, *parts[2];
void __real_s1_part_init(s1_dry_part *);
void __wrap_s1_part_init(s1_dry_part *p)
{
    __real_s1_part_init(p);
    current_part=p;
}
static void adc(int pitch)
{
    fm1_in.wheel[0]=pitch;fm1_in.wheel[1]=615;fm1_in.frames=301;
    smk_wheels();render(4);
}
static void wheel_witness(unsigned owner)
{
    const unsigned peer=1-owner;
    ui.part=PART_303A+owner;
    s1_dry_part *part=parts[owner];
    float neutral=part->e.t[S1_DETM_ID], other=parts[peer]->e.t[S1_DETM_ID];
    adc(500);
    adc(250);EXPECT(engine_bass_perf(owner,0,1)==0 && part->e.t[S1_DETM_ID] < neutral);
    EXPECT(parts[peer]->e.t[S1_DETM_ID]==other);
    ui.part=PART_303A+peer;
    adc(750);EXPECT(engine_bass_perf(owner,0,1)==127 && part->e.t[S1_DETM_ID] > neutral);
    EXPECT(parts[peer]->e.t[S1_DETM_ID]==other);
    adc(500);EXPECT(engine_bass_perf(owner,0,1)==64 && part->e.t[S1_DETM_ID] == neutral);
    EXPECT(parts[peer]->e.t[S1_DETM_ID]==other);
}
#if defined(X0X_S1_BEND_RANGE_PROBE) && X0X_S1_BEND_RANGE_PROBE
static void inject_bend(unsigned owner,unsigned raw)
{
    const unsigned ch=seq.ch[TRK_BASS0+owner];
    host_midi_put(0xeu | ((0xe0u|ch)<<8) | ((raw&127u)<<16) | (raw>>7)<<24);
    render(1);
}
static void bend_witness(unsigned owner)
{
    s1_dry_part *part=parts[owner], *peer=parts[1-owner];
    const float peer_detm=peer->e.t[S1_DETM_ID], base=part->detm;
    EXPECT(part->bend_min==-12 && part->bend_max==12);
    const float lo=owner ? -23.5f : -3.75f, hi=owner ? 7.5f : 24.f;
    EXPECT(s1_part_bend_ranges(part,lo,hi));
    EXPECT(!s1_part_bend_ranges(part,-25,hi) && !s1_part_bend_ranges(part,lo,NAN));
    EXPECT(part->bend_min==lo && part->bend_max==hi);
    engine_bass_on(owner,48,0,0);render(2);
    const unsigned values[]={0,1,127,128,2442,8190,8191,8192,8193,9867,16256,16382,16383};
    for(unsigned i=0;i<sizeof values/sizeof values[0];++i){
        const unsigned raw=values[i];
        inject_bend(owner,raw);
        float st=raw<8192?lo*((8191.f-raw)/8191.f):hi*((raw-8192.f)/8192.f);
        EXPECT(part->bend_raw==raw);
        EXPECT(fabsf(part->e.t[S1_DETM_ID]/base-exp2f(st/12.f))<0.000004f);
        if(raw==8191||raw==8192)EXPECT(part->e.t[S1_DETM_ID]==base);
        EXPECT(peer->e.t[S1_DETM_ID]==peer_detm);
    }
    EXPECT(s1_part_bend_ranges(part,0.f,hi));
    inject_bend(owner,0);EXPECT(part->e.t[S1_DETM_ID]==base);
    inject_bend(owner,8192);
    /* The public 14-bit ingress normalizes into the existing float receiver.
     * Check its exact inverse for all words without queuing 32k USB packets. */
    unsigned mismatch=0;
    for(unsigned raw=0;raw<16384;++raw) {
        float n=raw < 8192 ? (float)raw/16384.f : .5f+(float)(raw-8192u)/16382.f;
        mismatch+=!s1_part_param(part,0,n) || part->bend_raw!=raw;
    }
    EXPECT(mismatch==0);
    inject_bend(owner,8192);
    engine_bass_off(owner);render(2);
}
#endif
int main(void)
{
    setup(V_PART,PART_303A);
    for(unsigned v=0;v<2;++v){
        current_part=NULL;
        EXPECT(!engine_bass_select_preset(v,ORIGINAL_SOUND_SYNTHONE,0,&proj.sound));
        for(unsigned i=0;i<1024&&engine_bass_preset_pending(v);++i)render(1);
        EXPECT(current_part && engine_bass_bank(v)==ORIGINAL_SOUND_SYNTHONE);
        parts[v]=current_part;
    }
    EXPECT(parts[0]!=parts[1]);
    wheel_witness(0);wheel_witness(1);
#if defined(X0X_S1_BEND_RANGE_PROBE) && X0X_S1_BEND_RANGE_PROBE
    bend_witness(0);bend_witness(1);
#endif
    if(failures)return 1;
    printf("PASS actual extracted SMK ADC wheels and runtime both owners, held ownership, legacy range (%u)\n",expectations);
    return 0;
}
