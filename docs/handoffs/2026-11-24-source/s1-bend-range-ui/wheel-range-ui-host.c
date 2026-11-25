/* SPDX-License-Identifier: GPL-3.0-only */
#include "synth_init_test_helpers.h"
#include "app/s1c_part.h"
#include <math.h>

static struct { unsigned frames; int wheel[2]; uint16_t wheel_mod_seq; } fm1_in;
static int clamp(int v,int lo,int hi) { return v<lo?lo:v>hi?hi:v; }
enum { S1_DETM_ID=31 };
#include "mod-wheels.inc"
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
static void midi(unsigned owner,unsigned raw)
{
    unsigned ch=seq.ch[TRK_BASS0+owner];
    host_midi_put(0xeu | ((0xe0u|ch)<<8) | ((raw&127u)<<16) | (raw>>7)<<24);
    render(1);
}
static void matches(unsigned owner,int lo,int hi)
{
    CHECK(read_slot(owner,141).physical==lo && read_slot(owner,142).physical==hi);
    CHECK(read_slot(owner,141).max_position==24 && read_slot(owner,142).max_position==24);
}
static void actual(unsigned owner,unsigned raw,float lo,float hi)
{
    s1_dry_part *p=parts[owner];
    float st=raw<8192?lo*(8191.f-raw)/8191.f:hi*(raw-8192.f)/8192.f;
    CHECK(p->bend_raw==raw);
    CHECK(fabsf(p->e.t[S1_DETM_ID]/p->detm-exp2f(st/12.f))<.000004f);
    if(raw==8191 || raw==8192) CHECK(p->e.t[S1_DETM_ID]==p->detm);
}
int main(void)
{
    setup(V_PART,PART_303A); proj.set.bt|=PROJ_BT_CLK_IN;
    for(unsigned owner=0;owner<2;++owner) {
        current_part=NULL; select_patch(owner); CHECK(current_part); parts[owner]=current_part;
    }
    CHECK(parts[0]!=parts[1]);
    stabilize();
    int save_rc=project_save();
    CHECK(save_rc==0);
    const sound_t start=proj.sound;
    unsigned writes=store_writes;
    for(unsigned owner=0;owner<2;++owner) {
        unsigned peer=1-owner;
        ui.part=PART_303A+owner; build_pages(); visit("WHEELS");
        CHECK(pg.r[cur_page()][0].b==owner && pg.r[cur_page()][1].b==owner);
        const float peer_detm=parts[peer]->e.t[S1_DETM_ID];
        matches(owner,-12,12);
        detent(0,-127); render(1); matches(owner,-24,12);
        /* Real detents change lower and upper independently, including zero. */
        detent(1,-127); render(1); matches(owner,-24,0);
        detent(1,1); render(1); matches(owner,-24,1);
        engine_bass_on(owner,48,0,0); render(4);
        adc(500); adc(250); actual(owner,0,-24,1);
        ui.part=PART_303A+peer;
        adc(750); actual(owner,16383,-24,1);
        adc(500); actual(owner,8192,-24,1);
        const unsigned witness[]={0,1,8190,8191,8192,8193,16382,16383};
        for(unsigned i=0;i<sizeof witness/sizeof witness[0];++i){
            midi(owner,witness[i]); actual(owner,witness[i],-24,1);
            CHECK(parts[peer]->e.t[S1_DETM_ID]==peer_detm);
        }
        ui.part=PART_303A+owner; build_pages(); visit("WHEELS");
        detent(1,1); render(1); matches(owner,-24,2);
        actual(owner,16383,-24,2); /* edit updates currently bent held note without another MIDI event */
        detent(1,-1); render(1); matches(owner,-24,1);
        actual(owner,16383,-24,1);
        midi(owner,8192); engine_bass_off(owner); render(2);
    }
    CHECK(!memcmp(&proj.sound,&start,sizeof start) && store_writes==writes);
    int load_rc=project_load(); fprintf(stderr,"project load status %d\n",load_rc); CHECK(!load_rc);
    stabilize();
    for(unsigned owner=0;owner<2;++owner) {matches(owner,-12,12); actual(owner,8192,-12,12);}
    printf("PASS original independent WHEELS detents, ADC/MIDI, held recalculation/reverse edit, cold Init; %u checks\n",checks);
}
