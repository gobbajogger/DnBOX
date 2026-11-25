/* SPDX-License-Identifier: GPL-3.0-only */
#include "synth_init_test_helpers.h"
#include "app/s1c_part.h"
#include <math.h>
const s1c *host_coarse_owner(unsigned owner);
enum { DETUNE=30, ROUTE=55 };
#ifdef S1_DT_LFO_OLD
int main(void)
{
    for(unsigned owner=0;owner<2;++owner) {
        setup(V_PART,PART_303A+owner);
        proj.set.bt|=PROJ_BT_CLK_IN;
        stored_project("detune-lfo",owner,PROJECT_MOUNT);
        unsigned w=store_writes;
        CHECK(project_load()<0 && store_writes==w);
        CHECK(project_save()<0 && store_writes==w);
        stored_project("detune-lfo",owner,PROJECT_VERIFY);
    }
    printf("PASS exact prior reader refuses both actual Sound objects and attempted save; %u checks\n",checks);
}
#else
static void prepare(unsigned owner)
{
    setup(V_PART,PART_303A+owner);
    proj.set.bt|=PROJ_BT_CLK_IN;
    select_patch(owner); select_patch(1-owner); build_pages();
    proj.sound.v[T_MIX][PART_303A][MX_PAN]=0;
    proj.sound.v[T_MIX][PART_303B][MX_PAN]=127;
    for(unsigned i=0;i<2;++i) {
        proj.sound.v[T_MIX][PART_303A+i][MX_REV]=0;
        proj.sound.v[T_MIX][PART_303A+i][MX_DLY]=0;
    }
    proj.sound.v[T_MST][0][MST_RATIO]=proj.sound.v[T_MST][0][MST_PUMP]=0;
    proj.sound.v[T_MST][0][MST_LIMIT]=proj.sound.v[T_MST][0][MST_MODE]=0;
}
static void edit_route(unsigned owner,int value)
{
    visit("PITCH MOD"); smc_peek_cancel(); smc_page_set(cur_page());
    unsigned ix=(unsigned)cur_page();
    CHECK(ix+1u<pg.n && pg.title[ix]==PT_PITCH_MOD && pg.title[ix+1]==PT_ARPEGGIATOR);
    pref_t r=pg.r[ix][0];
    CHECK(r.kind==R_S1PATCH && r.b==owner && s1_patch_at(r.c)->factory_id==ROUTE && smc_same(r,smc_knob_target(0)));
    int delta=value-read_slot(owner,ROUTE).position;
    if(delta) { now_ms+=250; knob(0,delta); }
    else CHECK(engine_s1_patch_set(owner,read_slot(owner,ROUTE).revision,ROUTE,value)==S1_EDIT_QUEUED);
    render(1);
    CHECK(read_slot(owner,ROUTE).physical==(float)value);
}
static float measure(unsigned owner,unsigned route,unsigned active)
{
    begin_pair(owner,60); render(320);
    const s1c *s=host_coarse_owner(owner);
    for(unsigned j=0;j<S1C_VOICES;++j) {
        const s1c_voice *v=&s->v[j];
        if(!v->gate) continue;
        float det=(active?4.f:0.f)*(v->freq/261.6255653f);
        float factor=route==1?s->lfo[0].o01:route==2?s->lfo[1].o01:route==3?
                     (s->lfo[0].o01+s->lfo[1].o01)*.5f:1.f;
        float f1=v->kd1*s->sr, f2=v->kd2*s->sr;
        CHECK(fabsf(f2-f1-det*factor)<.02f);
        fprintf(stderr,"owner=%u route=%u depth=%u actual Hz=%.5f factor=%.5f\n",owner,route,active,f2-f1,factor);
        return f2-f1;
    }
    CHECK(0);
    return 0;
}
int main(void)
{
    CHECK(!s1_part_programs());
    const char *dump=getenv("S1_DT_LFO_PCM");
    if(dump) {
        prepare(0); begin_pair(0,57); record(reference);
        FILE *f=fopen(dump,"wb");
        CHECK(f && fwrite(reference,sizeof reference,1,f)==1 && !fclose(f));
        return 0;
    }
#ifdef S1_DT_LFO_RED
    prepare(0); (void)read_slot(0,ROUTE); return 0;
#else
    uint8_t root[32], c[32];
    CHECK(original_sound_init(root,ORIGINAL_SOUND_SYNTHONE,0));
    for(unsigned code=0;code<256;++code) for(unsigned mark=0;mark<2;++mark) for(unsigned route=0;route<8;++route) {
        memcpy(c,root,32); c[30]=(uint8_t)code; c[24]=(uint8_t)(mark<<7);
        for(unsigned i=0;i<3;++i) c[3+i]=(uint8_t)((route>>i&1u)<<7);
        CHECK(original_sound_valid(c)==(mark==!!(code||route) && code<225 && route<=4));
        if(route && mark) CHECK(!original_sound_is_legacy_slot2_cell(c));
    }
    for(unsigned owner=0;owner<2;++owner) {
        prepare(owner);
        CHECK(read_slot(owner,ROUTE).physical==0.f && read_slot(owner,DETUNE).physical==0.f);
        CHECK(engine_s1_patch_set(owner,read_slot(owner,ROUTE).revision,ROUTE,4)==S1_EDIT_REJECTED);
        /* Solo oscillator 2 using real controls; equivalent repeats deterministic. */
        visit("OSC MIX"); detent(0,127);
        visit("OSCILLATOR"); detent(0,127);
        visit("TEMPO SYNC"); detent(1,127); detent(2,127);
        visit("LFO"); detent(1,1); detent(3,2);
        const float l1=read_slot(owner,14).physical, l2=read_slot(owner,46).physical;
        CHECK(l1==1.f && l2==1.f);
        visit("VOICE"); detent(3,40);
        CHECK(read_slot(owner,DETUNE).physical==4.f);
        (void)measure(owner,0,1); record(reference);
        float hz[4];
        for(unsigned mode=1;mode<=3;++mode) {
            edit_route(owner,(int)mode);
            hz[mode]=measure(owner,mode,1); record(changed);
            own_pcm(owner);
        }
        CHECK(fabsf(hz[1]-hz[2])>.03f && fabsf(hz[3]-(hz[1]+hz[2])*.5f)<.025f);
        visit("VOICE"); detent(3,-40);
        CHECK(read_slot(owner,DETUNE).physical==0.f);
        edit_route(owner,0);
        (void)measure(owner,0,0); record(reference);
        for(unsigned mode=1;mode<=3;++mode) {
            edit_route(owner,(int)mode);
            (void)measure(owner,mode,0); record(changed);
            CHECK(!memcmp(reference,changed,sizeof reference));
        }
        CHECK(read_slot(1-owner,ROUTE).physical==0.f);
        visit("TEMPO SYNC"); detent(1,-17); detent(2,-19);
        float set_l1=read_slot(owner,14).physical, set_l2=read_slot(owner,46).physical;
        CHECK(set_l1<1.f && set_l2<1.f && read_slot(owner,ROUTE).physical==3.f);
        visit("FX SWITCHES"); detent(0,1); detent(1,-1);
        CHECK(read_slot(owner,ROUTE).physical==3.f);
        visit("OUTPUT"); detent(1,-11);
        const float level=read_slot(owner,32).physical;
        visit("MOD ROUTE"); detent(2,2); detent(3,1);
        CHECK(read_slot(owner,54).physical==2.f && read_slot(owner,52).physical==1.f);
        CHECK(read_slot(owner,ROUTE).physical==3.f);
        stabilize(); CHECK(!project_save()); stored_project("detune-lfo",owner,PROJECT_EXPORT);
        CHECK(!project_load());
        CHECK(read_slot(owner,14).physical==set_l1 && read_slot(owner,46).physical==set_l2);
        CHECK(read_slot(owner,ROUTE).physical==3.f && read_slot(owner,41).physical==1.f && read_slot(owner,37).physical==0.f);
    CHECK(read_slot(owner,32).physical==level && read_slot(owner,54).physical==2.f &&
              read_slot(owner,52).physical==1.f);
        engine_note_lock_capture(1);
        CHECK(!engine_bass_note_capture(proj.arr.lane,0,owner,0,&proj.sound));
        engine_note_lock_capture(0);
        CHECK(!engine_bass_note_validate(&proj.arr.lane[0]));
        build_pages(); edit_route(owner,0);
        bpart_t *line=&proj.pat[0].bass[owner];
        for(unsigned i=0;i<NSTEPS;++i) line->step[i].flags=G_REST;
        line->len=16; line->rate=RATE_16; line->dir=DIR_FWD;
        line->step[0].flags=line->step[4].flags=G_NOTE;
        line->step[0].note=36; line->step[4].note=43;
        line->step[0].velocity=line->step[4].velocity=100;
        seq.ppat[TRK_BASS0+owner]=0;
        engine_note_lock_refresh(); seq_start(&seq);
        for(unsigned i=0;i<2000 && !read_slot(owner,ROUTE).note_locked;++i) render(1);
        CHECK(read_slot(owner,ROUTE).note_locked && read_slot(owner,ROUTE).physical==3.f);
        CHECK(engine_s1_patch_set(owner,read_slot(owner,ROUTE).revision,ROUTE,1)==S1_EDIT_BUSY);
        for(unsigned i=0;i<2000 && read_slot(owner,ROUTE).note_locked;++i) render(1);
        CHECK(!read_slot(owner,ROUTE).note_locked && read_slot(owner,ROUTE).physical==0.f);
        CHECK(read_slot(owner,14).physical==set_l1 && read_slot(owner,46).physical==set_l2);
        seq_stop(&seq); render(4);
    }
    printf("PASS physical PITCH MOD K1, both owners/real OSC2 law, zero-DETUNE null, peer exact and cold recall; %u checks\n",checks);
    return 0;
#endif
}
#endif
