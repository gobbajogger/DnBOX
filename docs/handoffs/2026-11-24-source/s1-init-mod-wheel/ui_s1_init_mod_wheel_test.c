/* SPDX-License-Identifier: GPL-3.0-only */
#include "synth_init_test_helpers.h"
#include "app/s1c_part.h"
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
static float expect_cut(unsigned owner,unsigned cc)
{
    double cutoff=3.0*exp2(log2(120.0)+(log2(7600.0)-log2(120.0))*(1.0-(double)cc/127.0));
    float expected=(float)fmin(22050.,cutoff);
    s1_patch_value v=read_slot(owner,CUT);
    CHECK(fabsf(v.physical-expected)<.015f && v.physical==parts[owner]->e.t[CUT]);
    return v.physical;
}
static void prepare(void)
{
    setup(V_PART,PART_303A); proj.set.bt|=PROJ_BT_CLK_IN;
    for(unsigned owner=0;owner<2;++owner) {
        current=NULL; select_patch(owner); CHECK(current); parts[owner]=current;
    }
    CHECK(parts[0]!=parts[1]);
    proj.sound.v[T_MIX][PART_303A][MX_PAN]=0;
    proj.sound.v[T_MIX][PART_303B][MX_PAN]=127;
    for(unsigned i=0;i<2;++i) {
        proj.sound.v[T_MIX][PART_303A+i][MX_REV]=0;
        proj.sound.v[T_MIX][PART_303A+i][MX_DLY]=0;
    }
    proj.sound.v[T_MST][0][MST_RATIO]=proj.sound.v[T_MST][0][MST_PUMP]=0;
    proj.sound.v[T_MST][0][MST_LIMIT]=proj.sound.v[T_MST][0][MST_MODE]=0;
}
int main(void)
{
    CHECK(!s1_part_programs());
    prepare();
    const char *dump=getenv("S1_MOD_PCM");
    if(dump) {
        begin_pair(0,57); record(reference);
        FILE *f=fopen(dump,"wb");
        CHECK(f && fwrite(reference,sizeof reference,1,f)==1 && !fclose(f));
        return 0;
    }
    adc(615);
    CHECK(read_slot(0,CUT).physical==20000.f && read_slot(1,CUT).physical==20000.f);
    for(unsigned owner=0;owner<2;++owner) {
        unsigned peer=1-owner;
        /* Preserve whole serialized-project base and note lanes. */
        ui.rec=1; seq_start(&seq);
        const sound_t start=proj.sound;
        lane_t lanes[sizeof proj.arr.lane/sizeof *proj.arr.lane];
        memcpy(lanes,proj.arr.lane,sizeof lanes);
        unsigned writes=store_writes;
        float peer_cut=read_slot(peer,CUT).physical;
        float bend=parts[owner]->e.t[BEND], l1=parts[owner]->e.t[LFO1], l2=parts[owner]->e.t[LFO2];
        ui.part=(uint8_t)(PART_303A+owner);
        adc(485);
        CHECK(expect_cut(owner,127)==360.f);
        CHECK(read_slot(peer,CUT).physical==peer_cut);
        ui.part=(uint8_t)(PART_303A+peer);
        adc(545);
        unsigned mid=(615u-545u-12u)*127u/(615u-485u-12u);
        float middle=expect_cut(owner,mid);
        CHECK(read_slot(peer,CUT).physical==peer_cut);
        adc(615);
        CHECK(expect_cut(owner,0)==22050.f && read_slot(peer,CUT).physical==peer_cut);
        CHECK(engine_bass_mod_param(owner)<0); /* not a bogus R_ENG route */
        midi(owner,mid);
        CHECK(expect_cut(owner,mid)==middle && read_slot(peer,CUT).physical==peer_cut);
        for(unsigned k=0;k<128;++k) {
            midi(owner,k);
            (void)expect_cut(owner,k);
        }
        CHECK(parts[owner]->e.t[BEND]==bend && parts[owner]->e.t[LFO1]==l1 && parts[owner]->e.t[LFO2]==l2);
        CHECK(!memcmp(&proj.sound,&start,sizeof start) && !memcmp(proj.arr.lane,lanes,sizeof lanes) && store_writes==writes);
        seq_stop(&seq); ui.rec=0;
        /* A resting ADC sample must not overwrite the genuine editor. */
        CHECK(engine_s1_patch_set(owner,read_slot(owner,CUT).revision,CUT,71)==S1_EDIT_QUEUED);
        render(1);
        s1_patch_value knob=read_slot(owner,CUT);
        CHECK(knob.position==71);
        adc(615); adc(615);
        CHECK(read_slot(owner,CUT).physical==knob.physical);
        begin_pair(owner,57); render(320); record(reference);
        midi(owner,127); render(320); record(changed);
        /* Compare independently reset phases on each side, not sequential notes. */
        begin_pair(owner,57); midi(owner,0); render(320); record(reference);
        begin_pair(owner,57); midi(owner,127); render(320); record(changed);
        own_pcm(owner);
        /* Cold apply restores user's committed cutoff, not last CC1. */
        stabilize(); CHECK(read_slot(owner,CUT).physical==knob.physical);
        fprintf(stderr,"owner=%u ADC/MIDI mid CC%u = %.5f Hz, held part retained, untouched project\n",owner,mid,middle);
    }
    printf("PASS actual extracted physical ADC and real channel CC1, original default cutoff, no writes/lanes, both owners; %u checks\n",checks);
    return 0;
}
