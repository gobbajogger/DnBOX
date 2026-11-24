/* SPDX-License-Identifier: GPL-3.0-only */
#include "synth_init_test_helpers.h"
#include "app/s1c_part.h"
#include <math.h>
const s1c *host_coarse_owner(unsigned owner);
enum { MONO_ID=20,GLIDE_ID=21 };
#if defined(S1_GLIDE_OLD)
int main(void)
{
    for(unsigned i=0;i<4;++i) {
        const unsigned owner=i&1u;
        const char *tag=i<2 ? "glide-new" : "glide-only";
        setup(V_PART,PART_303A+owner);
        proj.set.bt|=PROJ_BT_CLK_IN;
        stored_project(tag,owner,PROJECT_MOUNT);
        unsigned w=store_writes;
        CHECK(project_load()<0 && store_writes==w);
        CHECK(project_save()<0 && store_writes==w);
        stored_project(tag,owner,PROJECT_VERIFY);
    }
    printf("PASS exact prior parser rejects live projects and subsequent save without writes, %u checks\n",checks);
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
static void edit_voice(unsigned owner,unsigned id,int delta)
{
    const unsigned col=id==MONO_ID?2u:3u;
    visit("OUTPUT"); smc_peek_cancel(); smc_page_set(cur_page());
    pref_t ref=pg.r[cur_page()][col];
    CHECK(ref.kind==R_S1PATCH && ref.b==owner && s1_patch_at(ref.c)->factory_id==id);
    CHECK(smc_same(ref,smc_knob_target(col)));
    while(delta) {
        int step=delta < -32 ? -32 : delta > 32 ? 32 : delta;
        now_ms+=200u+(unsigned)abs(step)*70u;
        knob(col,step); render(1); delta-=step;
    }
}
static unsigned gated(const s1c *s)
{
    unsigned n=0; for(unsigned j=0;j<S1C_VOICES;++j) n+=s->v[j].gate!=0;
    return n;
}
static void render_transition(unsigned owner,int32_t *pcm,unsigned mono,unsigned glide)
{
    stabilize();
    uint32_t generation, context; int arp;
    engine_live_key_state(owner,&generation,&arp,&context);
    CHECK(!arp);
    const s1c *s=host_coarse_owner(owner);
    CHECK(engine_live_key(owner,generation,1,60,100,0,0));
    engine_bass_on(1-owner,55,0,0);
    render(440);
    CHECK(gated(s)==1);
    float start=s->monos;
    CHECK(engine_live_key(owner,generation,2,72,100,0,0));
    render(1);
    CHECK(gated(s)==(mono?1:2) && s->nheld==2);
    if(mono) {
        CHECK(s->v[0].root==72 && s->monof>start);
        if(glide) {
            float target=s->monof;
            float expected=target+(start-target)*powf(.5f,256.f/(s->sr*.2f));
            CHECK(fabsf(s->monos-expected)<.04f);
            CHECK(s->v[0].kd1*s->sr<target && fabsf(s->v[0].kd1*s->sr-s->monos)<.4f);
        } else CHECK(s->monos==s->monof);
    }
    record(pcm);
    if(mono && glide) {
        render(17); /* 256 + 4096 + 17*256 = 8704 physical samples */
        float expected=s->monof+(start-s->monof)*powf(.5f,8704.f/(s->sr*.2f));
        CHECK(fabsf(s->monos-expected)<.35f);
        fprintf(stderr,"owner=%u half-time near sample 8704: %.3f Hz vs %.3f expected\n",owner,s->monos,expected);
    }
    CHECK(engine_live_key(owner,generation,2,72,0,0,0));
    render(1);
    if(mono) CHECK(gated(s)==1 && s->v[0].root==60 && s->monof<500.f);
    CHECK(engine_live_key(owner,generation,1,60,0,0,0));
    render(1);
    CHECK(!s->nheld);
}
int main(void)
{
    CHECK(!s1_part_programs());
    const char *dump=getenv("S1_GLIDE_PCM");
    if(dump) {
        init_pair(0); begin_pair(0,57); record(reference);
        FILE *f=fopen(dump,"wb");
        CHECK(f && fwrite(reference,sizeof reference,1,f)==1 && !fclose(f));
        return 0;
    }
#ifdef S1_GLIDE_RED
    init_pair(0); (void)read_slot(0,GLIDE_ID);
    return 0;
#else
    uint8_t clean[32], c[32];
    CHECK(original_sound_init(clean,ORIGINAL_SOUND_SYNTHONE,0));
    for(unsigned shape=0;shape<128;++shape) for(unsigned link=0;link<2;++link) {
        memcpy(c,clean,32);
        if(link) CHECK(original_sound_s1_set(c,S1_PATCH_COUNT,1));
        c[S1_LINK_BYTE]=(uint8_t)(2*shape+link);
        if(shape) CHECK(!original_sound_valid(c));
        c[20] |= shape ? 128u : 0u;
        CHECK(original_sound_valid(c)==(shape<126));
        if(shape) CHECK(!original_sound_is_legacy_slot2_cell(c));
    }
    for(unsigned owner=0;owner<2;++owner) {
        init_pair(owner);
        CHECK(read_slot(owner,MONO_ID).physical==0.f && read_slot(owner,GLIDE_ID).physical==0.f);
        CHECK(engine_s1_patch_set(owner,read_slot(owner,GLIDE_ID).revision,GLIDE_ID,41)==S1_EDIT_REJECTED);
        render_transition(owner,reference,0,0);
        edit_voice(owner,GLIDE_ID,40);
        CHECK(read_slot(owner,GLIDE_ID).physical==.2f);
        stabilize(); CHECK(!project_save());
        stored_project("glide-only",owner,PROJECT_EXPORT);
        render_transition(owner,changed,0,1);
        CHECK(!memcmp(reference,changed,sizeof reference)); /* mono slide not a pitch CV in poly */
        edit_voice(owner,MONO_ID,1);
        CHECK(read_slot(owner,MONO_ID).physical==1.f);
        edit_voice(owner,GLIDE_ID,-40);
        render_transition(owner,reference,1,0);
        edit_voice(owner,GLIDE_ID,40);
        render_transition(owner,changed,1,1);
        own_pcm(owner);
        visit("VOICE"); detent(3,13); render(1);
        visit("FILTER"); detent(3,1); render(1);
        visit("VOICE"); detent(0,1); render(1);
        CHECK(read_slot(owner,30).physical==1.3f && read_slot(owner,120).physical==1.f &&
              read_slot(owner,S1_LINK_ID).physical==1.f);
        visit("AMP ADSR"); detent(3,21); render(1);
        const float release=read_slot(owner,29).physical;
        CHECK((proj.sound.v[T_303][owner][20]&128u) && (proj.sound.v[T_303][owner][20]&127u));
        edit_voice(owner,GLIDE_ID,-11);
        CHECK(read_slot(owner,29).physical==release);
        CHECK(read_slot(owner,MONO_ID).physical==1.f && read_slot(owner,GLIDE_ID).physical==.145f);
        CHECK(read_slot(1-owner,MONO_ID).physical==0.f && read_slot(1-owner,GLIDE_ID).physical==0.f);
        stabilize(); CHECK(!project_save());
        stored_project("glide-new",owner,PROJECT_EXPORT);
        CHECK(!project_load() && read_slot(owner,29).physical==release && read_slot(owner,MONO_ID).physical==1.f && read_slot(owner,GLIDE_ID).physical==.145f);
        CHECK(read_slot(owner,30).physical==1.3f && read_slot(owner,120).physical==1.f &&
              read_slot(owner,S1_LINK_ID).physical==1.f);
    engine_note_lock_capture(1);
        CHECK(!engine_bass_note_capture(proj.arr.lane,0,owner,0,&proj.sound));
        engine_note_lock_capture(0);
        CHECK(!engine_bass_note_validate(&proj.arr.lane[0]));
        build_pages(); edit_voice(owner,MONO_ID,-1); edit_voice(owner,GLIDE_ID,-29);
        bpart_t *line=&proj.pat[0].bass[owner];
        for(unsigned i=0;i<NSTEPS;++i) line->step[i].flags=G_REST;
        line->len=16; line->rate=RATE_16; line->dir=DIR_FWD;
        line->step[0].flags=line->step[4].flags=G_NOTE;
        line->step[0].note=36; line->step[4].note=43;
        line->step[0].velocity=line->step[4].velocity=100;
        seq.ppat[TRK_BASS0+owner]=0;
        engine_note_lock_refresh(); seq_start(&seq);
        for(unsigned i=0;i<2000 && !read_slot(owner,MONO_ID).note_locked;++i) render(1);
        CHECK(read_slot(owner,MONO_ID).note_locked && read_slot(owner,MONO_ID).physical==1.f);
        CHECK(read_slot(owner,GLIDE_ID).physical==.145f);
        CHECK(gated(host_coarse_owner(owner))==1);
        for(unsigned i=0;i<2000 && read_slot(owner,MONO_ID).note_locked;++i) render(1);
        CHECK(!read_slot(owner,MONO_ID).note_locked);
        CHECK(read_slot(owner,MONO_ID).physical==0.f && read_slot(owner,GLIDE_ID).physical==0.f);
        CHECK(gated(host_coarse_owner(owner))>=1);
        seq_stop(&seq); render(4);
    }
    printf("PASS both real OUTPUT K3/K4, poly no-op/mono 200 ms law, peer PCM, detune/legato/link and cold reload; %u checks\n",checks);
    return 0;
#endif
}
#endif
