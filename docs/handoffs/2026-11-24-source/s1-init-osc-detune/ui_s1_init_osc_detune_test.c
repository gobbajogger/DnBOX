/* SPDX-License-Identifier: GPL-3.0-only */
#include "synth_init_test_helpers.h"
#include "app/s1c_part.h"
#include <math.h>
const s1c *host_coarse_owner(unsigned owner);
enum { DETUNE_ID=30 };
#if defined(S1_DETUNE_OLD)
int main(void)
{
    for(unsigned owner=0;owner<2;++owner) {
        setup(V_PART,PART_303A+owner);
        proj.set.bt|=PROJ_BT_CLK_IN;
        stored_project("detune-new",owner,PROJECT_MOUNT);
        unsigned w=store_writes;
        CHECK(project_load()<0 && store_writes==w);
        CHECK(project_save()<0 && store_writes==w);
        stored_project("detune-new",owner,PROJECT_VERIFY);
    }
    printf("PASS prior reader rejects actual owner projects without writes, %u checks\n",checks);
}
#else
static void init_pair(unsigned owner)
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
static void edit_detune(unsigned owner,int delta)
{
    visit("VOICE"); smc_peek_cancel(); smc_page_set(cur_page());
    pref_t ref=pg.r[cur_page()][3];
    CHECK(ref.kind==R_S1PATCH && ref.b==owner && s1_patch_at(ref.c)->factory_id==DETUNE_ID);
    CHECK(smc_same(ref,smc_knob_target(3)));
    while(delta) {
        int step=delta < -32 ? -32 : delta > 32 ? 32 : delta;
        now_ms += 200u+(unsigned)abs(step)*70u;
        knob(3,step); render(1); delta-=step;
    }
}
static void expect_detune(unsigned owner, float physical,unsigned pos)
{
    s1_patch_value d=read_slot(owner,DETUNE_ID);
    CHECK(d.physical==physical && d.position==pos && d.max_position==80);
}
static void pitch(unsigned owner,float out[3])
{
    const s1c *s=host_coarse_owner(owner);
    for(unsigned j=0;j<S1C_VOICES;++j) {
        const s1c_voice *v=&s->v[j];
        if(v->gate) {
            out[0]=s->sr*v->kd1; out[1]=s->sr*v->kd2; out[2]=v->freq;
            return;
        }
    }
    CHECK(0);
}
int main(void)
{
    CHECK(!s1_part_programs());
    const char *dump=getenv("S1_DETUNE_PCM");
    if(dump) {
        init_pair(0); begin_pair(0,57); record(reference);
        FILE *f=fopen(dump,"wb");
        CHECK(f && fwrite(reference,sizeof reference,1,f)==1 && !fclose(f));
        return 0;
    }
#ifdef S1_DETUNE_RED
    init_pair(0); (void)read_slot(0,DETUNE_ID);
    return 0;
#else
    uint8_t clean[32], c[32];
    CHECK(original_sound_init(clean,ORIGINAL_SOUND_SYNTHONE,0));
    for(unsigned a=0;a<128;++a) for(unsigned crush=0;crush<2;++crush) {
        memcpy(c,clean,32);
        if(crush) CHECK(original_sound_set(c,ORIGINAL_SYNTHONE_CRUSH,91));
        c[31]=(uint8_t)(a*2+crush);
        CHECK(original_sound_valid(c)==(a<=81));
        if(a) CHECK(!original_sound_is_legacy_slot2_cell(c));
    }
    for(unsigned owner=0;owner<2;++owner) {
        init_pair(owner);
        expect_detune(owner,0.f,40);
        CHECK(engine_s1_patch_set(owner,read_slot(owner,DETUNE_ID).revision,DETUNE_ID,81)==S1_EDIT_REJECTED);
        visit("OSC MIX"); detent(0,127);
        visit("OSCILLATOR"); detent(0,127);
        const unsigned notes[2]={60,72};
        for(unsigned i=0;i<2;++i) {
            float base[3], tuned[3];
            begin_pair(owner,notes[i]); render(320); pitch(owner,base); record(reference);
            edit_detune(owner,40); expect_detune(owner,4.f,80);
            begin_pair(owner,notes[i]); render(320); pitch(owner,tuned); record(changed);
            fprintf(stderr,"owner=%u note=%u base=%f delta=%f expected=%f route=%f target=%f\n",owner,notes[i],base[1],tuned[1]-base[1],4.f*(base[2]/261.6255653006f),host_coarse_owner(owner)->t[55],host_coarse_owner(owner)->t[30]);
            CHECK(fabsf(tuned[1]-base[1]-4.f*(base[2]/261.6255653006f))<.005f);
            CHECK(base[0]==tuned[0] && base[2]==tuned[2]);
            own_pcm(owner);
            edit_detune(owner,-100); expect_detune(owner,-4.f,0);
            begin_pair(owner,notes[i]); render(320); pitch(owner,tuned);
            CHECK(fabsf(tuned[1]-base[1]+4.f*(base[2]/261.6255653006f))<.005f);
            edit_detune(owner,40); expect_detune(owner,0.f,40);
        }
        edit_detune(owner,13); expect_detune(owner,1.3f,53);
        visit("VOICE"); detent(1,7); render(1);
        CHECK(read_slot(owner,3).physical==7.f && read_slot(owner,4).physical==0.f);
        visit("EFFECT"); detent(0,14); render(1);
        uint8_t *c=proj.sound.v[T_303][owner];
        CHECK((c[31]&1u) && c[ORIGINAL_SYNTHONE_CRUSH]);
        const unsigned crush=c[ORIGINAL_SYNTHONE_CRUSH];
        visit("FILTER"); detent(3,1); render(1);
        edit_detune(owner,-25); expect_detune(owner,-1.2f,28);
        CHECK((c[31]&1u) && c[ORIGINAL_SYNTHONE_CRUSH]==crush);
        CHECK(read_slot(owner,3).physical==7.f && read_slot(owner,S1_LINK_ID).physical==1.f);
        expect_detune(1-owner,0.f,40);
        stabilize(); CHECK(!project_save());
        stored_project("detune-new",owner,PROJECT_EXPORT);
        CHECK(!project_load());
        expect_detune(owner,-1.2f,28);
        CHECK(read_slot(owner,3).physical==7.f && read_slot(owner,S1_LINK_ID).physical==1.f);
        CHECK((c[31]&1u) && c[ORIGINAL_SYNTHONE_CRUSH]==crush);
        engine_note_lock_capture(1);
        CHECK(!engine_bass_note_capture(proj.arr.lane,0,owner,0,&proj.sound));
        engine_note_lock_capture(0);
        CHECK(!engine_bass_note_validate(&proj.arr.lane[0]));
        build_pages();
        edit_detune(owner,12); expect_detune(owner,0.f,40);
        bpart_t *line=&proj.pat[0].bass[owner];
        for(unsigned i=0;i<NSTEPS;++i) line->step[i].flags=G_REST;
        line->len=16; line->rate=RATE_16; line->dir=DIR_FWD;
        line->step[0].flags=line->step[4].flags=G_NOTE;
        line->step[0].note=36; line->step[4].note=43;
        line->step[0].velocity=line->step[4].velocity=100;
        seq.ppat[TRK_BASS0+owner]=0;
        engine_note_lock_refresh(); seq_start(&seq);
        for(unsigned i=0;i<2000 && !read_slot(owner,DETUNE_ID).note_locked;++i) render(1);
        CHECK(read_slot(owner,DETUNE_ID).note_locked);
        expect_detune(owner,-1.2f,28);
        CHECK(engine_s1_patch_set(owner,read_slot(owner,DETUNE_ID).revision,DETUNE_ID,40)==S1_EDIT_BUSY);
        for(unsigned i=0;i<2000 && read_slot(owner,DETUNE_ID).note_locked;++i) render(1);
        CHECK(!read_slot(owner,DETUNE_ID).note_locked);
        expect_detune(owner,0.f,40);
        CHECK((c[31]&1u) && c[ORIGINAL_SYNTHONE_CRUSH]==crush);
        CHECK(read_slot(owner,3).physical==7.f && read_slot(owner,S1_LINK_ID).physical==1.f);
        seq_stop(&seq); render(4);
    }
    printf("PASS real VOICE K4, both owners/notes physical law, peer PCM, crush/coarse/link, and cold reload; %u checks\n",checks);
    return 0;
#endif
}
#endif
