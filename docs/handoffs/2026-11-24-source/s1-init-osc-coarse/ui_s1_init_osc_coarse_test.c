/* SPDX-License-Identifier: GPL-3.0-only */
#include "synth_init_test_helpers.h"
#include "app/s1c_part.h"
#include <math.h>
const s1c *host_coarse_owner(unsigned owner);
enum { O1=3,O2=4 };
#ifdef S1_COARSE_OLD
int main(void)
{
    for(unsigned owner=0;owner<2;++owner) {
        setup(V_PART,PART_303A+owner);
        proj.set.bt|=PROJ_BT_CLK_IN;
        stored_project("coarse-new",owner,PROJECT_MOUNT);
        unsigned w=store_writes;
        CHECK(project_load()<0 && store_writes==w);
        CHECK(project_save()<0 && store_writes==w);
        stored_project("coarse-new",owner,PROJECT_VERIFY);
    }
    printf("PASS prior reader refuses both actual projects without save, %u checks\n",checks);
}
#else
static void expect_pitch(unsigned owner, unsigned osc,unsigned pos)
{
    s1_patch_value got=read_slot(owner,O1+osc);
    CHECK(got.position==pos && got.physical==(float)((int)pos-12) && got.max_position==36);
}
static void apply_tune(unsigned owner,unsigned osc,int delta)
{
    visit("VOICE");
    smc_peek_cancel(); smc_page_set(cur_page());
    pref_t ref=pg.r[cur_page()][1+osc];
    CHECK(ref.kind==R_S1PATCH && ref.b==owner && smc_same(ref,smc_knob_target(1+osc)));
    while(delta) {
        int step=delta < -32 ? -32 : delta > 32 ? 32 : delta;
        now_ms += 200u+(unsigned)abs(step)*70u;
        knob(1+osc,step); render(1);
        delta-=step;
    }
}
static void init_pair(unsigned owner)
{
    setup(V_PART,PART_303A+owner);
    proj.set.bt|=PROJ_BT_CLK_IN;
    select_patch(owner);
    select_patch(1-owner);
    build_pages();
    proj.sound.v[T_MIX][PART_303A][MX_PAN]=0;
    proj.sound.v[T_MIX][PART_303B][MX_PAN]=127;
    for(unsigned i=0;i<2;++i) {
        proj.sound.v[T_MIX][PART_303A+i][MX_REV]=0;
        proj.sound.v[T_MIX][PART_303A+i][MX_DLY]=0;
    }
    proj.sound.v[T_MST][0][MST_RATIO]=proj.sound.v[T_MST][0][MST_PUMP]=0;
    proj.sound.v[T_MST][0][MST_LIMIT]=proj.sound.v[T_MST][0][MST_MODE]=0;
}
static float hz(unsigned owner,unsigned osc)
{
    const s1c *s=host_coarse_owner(owner);
    for(unsigned j=0;j<S1C_VOICES;++j) {
        const s1c_voice *v=&s->v[j];
        if(v->gate) return s->sr*(osc ? v->kd2 : v->kd1);
    }
    CHECK(0);
    return 0;
}
int main(void)
{
    CHECK(!s1_part_programs());
    const char *dump=getenv("S1_COARSE_PCM");
    if (dump) {
        init_pair(0);
        begin_pair(0,57);
        record(reference);
        FILE *f=fopen(dump,"wb");
        CHECK(f && fwrite(reference,sizeof reference,1,f)==1 && !fclose(f));
        return 0;
    }
#ifdef S1_COARSE_RED
    init_pair(0);
    (void)read_slot(0,O1);
    return 0;
#else
    uint8_t root[32], c[32];
    CHECK(original_sound_init(root,ORIGINAL_SOUND_SYNTHONE,0));
    for(unsigned a=0;a<64;++a) for(unsigned b=0;b<64;++b){
        memcpy(c,root,sizeof c);
        for(unsigned i=0;i<6;++i) {
            c[8+i]=(uint8_t)((a>>i & 1u)<<7);
            c[14+i]=(uint8_t)((b>>i & 1u)<<7);
        }
        CHECK(original_sound_valid(c)==(a<=37 && b<=37));
        if(a||b) CHECK(!original_sound_is_legacy_slot2_cell(c));
    }
    for(unsigned owner=0;owner<2;++owner) {
        init_pair(owner);
        expect_pitch(owner,0,12); expect_pitch(owner,1,12);
        CHECK(engine_s1_patch_set(owner,read_slot(owner,O1).revision,O1,37)==S1_EDIT_REJECTED);
        for(unsigned osc=0;osc<2;++osc){
            /* Isolate only the edited oscillator with the real BAL control. */
            visit("OSC MIX"); detent(0,127);
            visit("OSCILLATOR"); detent(1,127);
            detent(0,osc ? 127 : -127);
            render(1);
            begin_pair(owner,57);
            float base=hz(owner,osc), sibling=hz(owner,1-osc);
            record(reference);
            unsigned peer_before=store_writes;
            apply_tune(owner,osc,12);
            expect_pitch(owner,osc,24);
            expect_pitch(owner,1-osc,12);
            begin_pair(owner,57);
            CHECK(fabsf(hz(owner,osc)-2.f*base)<.01f);
            CHECK(fabsf(hz(owner,1-osc)-sibling)<.01f);
            record(changed);
            own_pcm(owner);
            CHECK(store_writes==peer_before);
            apply_tune(owner,osc,-100);
            expect_pitch(owner,osc,0);
            begin_pair(owner,57);
            CHECK(fabsf(hz(owner,osc)-.5f*base)<.01f);
            apply_tune(owner,osc,100);
            expect_pitch(owner,osc,36);
            begin_pair(owner,57);
            CHECK(fabsf(hz(owner,osc)-4.f*base)<.01f);
            apply_tune(owner,osc,-24);
            expect_pitch(owner,osc,12);
        }
        apply_tune(owner,0,7); apply_tune(owner,1,-7);
        visit("FILTER"); detent(3,1); render(1);
        CHECK(read_slot(owner,S1_LINK_ID).physical==1);
        visit("AMP ADSR"); detent(0,19); render(1);
        const float attack=read_slot(owner,26).physical;
        expect_pitch(owner,0,19); expect_pitch(owner,1,5);
        CHECK(read_slot(1-owner,O1).physical==0 && read_slot(1-owner,O2).physical==0);
        stabilize();
        CHECK(!project_save());
        stored_project("coarse-new",owner,PROJECT_EXPORT);
        CHECK(!project_load());
        expect_pitch(owner,0,19); expect_pitch(owner,1,5);
        CHECK(read_slot(owner,26).physical==attack && read_slot(owner,S1_LINK_ID).physical==1);
        /* Authored owner lock takes precedence, then restores current patch. */
        engine_note_lock_capture(1);
        CHECK(!engine_bass_note_capture(proj.arr.lane,0,owner,0,&proj.sound));
        engine_note_lock_capture(0);
        CHECK(!engine_bass_note_validate(&proj.arr.lane[0]));
        build_pages();
        apply_tune(owner,0,-7); apply_tune(owner,1,7);
        expect_pitch(owner,0,12); expect_pitch(owner,1,12);
        bpart_t *line=&proj.pat[0].bass[owner];
        for (unsigned i=0;i<NSTEPS;++i) line->step[i].flags=G_REST;
        line->len=16; line->rate=RATE_16; line->dir=DIR_FWD;
        line->step[0].flags=line->step[4].flags=G_NOTE;
        line->step[0].note=36; line->step[4].note=43;
        line->step[0].velocity=line->step[4].velocity=100;
        seq.ppat[TRK_BASS0+owner]=0;
        engine_note_lock_refresh(); seq_start(&seq);
        for(unsigned i=0;i<2000 && !read_slot(owner,O1).note_locked;++i) render(1);
        CHECK(read_slot(owner,O1).note_locked);
        expect_pitch(owner,0,19); expect_pitch(owner,1,5);
        CHECK(engine_s1_patch_set(owner,read_slot(owner,O1).revision,O1,12)==S1_EDIT_BUSY);
        for(unsigned i=0;i<2000 && read_slot(owner,O1).note_locked;++i) render(1);
        CHECK(!read_slot(owner,O1).note_locked);
        expect_pitch(owner,0,12); expect_pitch(owner,1,12);
        CHECK(read_slot(owner,26).physical==attack && read_slot(owner,S1_LINK_ID).physical==1);
        seq_stop(&seq); render(4);
    }
    printf("PASS Init-only physical K2/K3, independent owner pitch and peer PCM, persistent endpoints/link; %u checks\n",checks);
    return 0;
#endif
}
#endif
