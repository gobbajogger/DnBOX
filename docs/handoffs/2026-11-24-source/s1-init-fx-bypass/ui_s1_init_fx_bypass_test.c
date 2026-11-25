/* SPDX-License-Identifier: GPL-3.0-only */
#include "synth_init_test_helpers.h"
#include "app/s1c_part.h"
#include <math.h>
const s1c *host_coarse_owner(unsigned owner);
enum { DLY=41, REV=37, DM=44, RM=40 };
#ifdef S1_FX_OLD
int main(void)
{
    for(unsigned owner=0;owner<2;++owner) {
        setup(V_PART,PART_303A+owner);
        proj.set.bt|=PROJ_BT_CLK_IN;
        stored_project("fx-new",owner,PROJECT_MOUNT);
        unsigned w=store_writes;
        CHECK(project_load()<0 && store_writes==w);
        CHECK(project_save()<0 && store_writes==w);
        stored_project("fx-new",owner,PROJECT_VERIFY);
    }
    printf("PASS older reader refuses both real persisted projects and attempted save; %u checks\n",checks);
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
static void edit_gate(unsigned owner,unsigned id,int value)
{
#ifndef S1_FX_BACKEND
    unsigned col=id==DLY?0u:1u;
    visit("FX SWITCHES"); smc_peek_cancel(); smc_page_set(cur_page());
#ifdef S1_FX_TYPED
    unsigned ix=(unsigned)cur_page();
    CHECK(ix>=2 && ix+2<pg.n && pg.title[ix]==PT_FX_BYPASS);
    CHECK(pg.title[ix-2]==PT_ARPEGGIATOR && pg.title[ix-1]==PT_EFFECT &&
          pg.title[ix+1]==PT_OUTPUT && pg.title[ix+2]==PT_SOUND);
#endif
    pref_t r=pg.r[cur_page()][col];
    CHECK(r.kind==R_S1PATCH && r.b==owner && s1_patch_at(r.c)->factory_id==id && smc_same(r,smc_knob_target(col)));
    int delta=value-read_slot(owner,id).position;
    if(delta) { now_ms+=260; knob(col,delta); }
    else CHECK(engine_s1_patch_set(owner,read_slot(owner,id).revision,id,value)==S1_EDIT_QUEUED);
#else
    CHECK(engine_s1_patch_set(owner,read_slot(owner,id).revision,id,value)==S1_EDIT_QUEUED);
#endif
    render(1);
    CHECK(read_slot(owner,id).physical==(float)value);
}
static void held(unsigned owner)
{
    /* A restart, same selected gate/mix combination and identical onset. */
    begin_pair(owner,57); render(200);
}
int main(void)
{
    CHECK(!s1_part_programs());
    const char *dump=getenv("S1_FX_PCM");
    if(dump) {
        prepare(0); begin_pair(0,57); record(reference);
        FILE *f=fopen(dump,"wb");
        CHECK(f && fwrite(reference,sizeof reference,1,f)==1 && !fclose(f));
        return 0;
    }
#ifdef S1_FX_RED
    prepare(0); (void)read_slot(0,DLY); return 0;
#else
    uint8_t root[32], c[32];
    CHECK(original_sound_init(root,ORIGINAL_SOUND_SYNTHONE,0));
    for(unsigned code=0;code<256;++code) for(unsigned mark=0;mark<2;++mark) {
        memcpy(c,root,32); c[30]=(uint8_t)code; c[24]=(uint8_t)(mark<<7);
        CHECK(original_sound_valid(c)==(mark==!!code && code<225));
    }
    for(unsigned route=0;route<25;++route) for(unsigned a=0;a<2;++a) for(unsigned b=0;b<2;++b) {
        memcpy(c,root,32); c[30]=(uint8_t)route; c[24]=route?128u:0u;
        CHECK(original_sound_s1_set(c,S1_DELAY_ON,a)); CHECK(original_sound_s1_set(c,S1_REVERB_ON,b));
        CHECK(c[30]%25==route && c[30]==route+25*(a+1)+75*(b+1));
    }
    for(unsigned owner=0;owner<2;++owner) {
        prepare(owner);
        CHECK(read_slot(owner,DLY).physical==0.f && read_slot(owner,REV).physical==1.f);
        CHECK(engine_s1_patch_set(owner,read_slot(owner,DLY).revision,DLY,2)==S1_EDIT_REJECTED);
        visit("EFFECT");
        detent(1,100); detent(2,100);
        const float dm=read_slot(owner,DM).physical, rm=read_slot(owner,RM).physical;
        CHECK(dm>.4f && rm>.4f);
        edit_gate(owner,REV,0); edit_gate(owner,DLY,0);
        held(owner);
        const s1c *s=host_coarse_owner(owner);
        CHECK(s->delay_history && s->ddec>1 && s->dl[0].len>16);
        const int dd=s->ddec, len=s->dl[0].len, idx=s->dl[0].w;
        CHECK(s->dl[0].buf>= (const int16_t*)s->pool &&
              s->dl[0].buf+len <= (const int16_t*)(s->pool+s->pool_bytes));
        edit_gate(owner,DLY,1);
        CHECK(s->ddec==dd && s->dl[0].len==len && s->dl[0].w==(idx+256/dd+(s->dk<256%dd))%len);
        edit_gate(owner,DLY,0);
        CHECK(s->ddec==dd && s->dl[0].len==len);
        held(owner); record(reference);
        edit_gate(owner,DLY,1);
        held(owner); record(changed);
        own_pcm(owner);
        CHECK(read_slot(owner,DM).physical==dm && read_slot(owner,RM).physical==rm);
        edit_gate(owner,DLY,0);
        held(owner); record(reference);
        edit_gate(owner,REV,1);
        held(owner); record(changed);
        own_pcm(owner);
        edit_gate(owner,REV,0);
        CHECK(read_slot(owner,REV).physical==0.f && read_slot(owner,RM).physical==rm);
        CHECK(read_slot(1-owner,DLY).physical==0.f && read_slot(1-owner,REV).physical==1.f);
        edit_gate(owner,DLY,1);
        visit("OUTPUT"); detent(2,1); detent(3,21); render(1);
        visit("VOICE"); detent(1,3); detent(3,9); render(1);
        visit("FILTER"); detent(3,1); render(1);
        stabilize(); CHECK(!project_save());
        stored_project("fx-new",owner,PROJECT_EXPORT);
        CHECK(!project_load());
        CHECK(read_slot(owner,DLY).physical==1.f && read_slot(owner,REV).physical==0.f);
        CHECK(read_slot(owner,DM).physical==dm && read_slot(owner,RM).physical==rm);
        CHECK(read_slot(owner,20).physical==1.f && read_slot(owner,21).physical==.105f &&
              read_slot(owner,3).physical==3.f && read_slot(owner,30).physical==.9f &&
              read_slot(owner,S1_LINK_ID).physical==1.f);
        engine_note_lock_capture(1);
        CHECK(!engine_bass_note_capture(proj.arr.lane,0,owner,0,&proj.sound));
        engine_note_lock_capture(0);
        CHECK(!engine_bass_note_validate(&proj.arr.lane[0]));
        build_pages();
        edit_gate(owner,DLY,0); edit_gate(owner,REV,1);
        bpart_t *line=&proj.pat[0].bass[owner];
        for(unsigned i=0;i<NSTEPS;++i) line->step[i].flags=G_REST;
        line->len=16; line->rate=RATE_16; line->dir=DIR_FWD;
        line->step[0].flags=line->step[4].flags=G_NOTE;
        line->step[0].note=36; line->step[4].note=43;
        line->step[0].velocity=line->step[4].velocity=100;
        seq.ppat[TRK_BASS0+owner]=0;
        engine_note_lock_refresh(); seq_start(&seq);
        for(unsigned i=0;i<2000 && !read_slot(owner,DLY).note_locked;++i) render(1);
        CHECK(read_slot(owner,DLY).note_locked && read_slot(owner,DLY).physical==1.f &&
              read_slot(owner,REV).physical==0.f);
        CHECK(engine_s1_patch_set(owner,read_slot(owner,REV).revision,REV,1)==S1_EDIT_BUSY);
        for(unsigned i=0;i<2000 && read_slot(owner,DLY).note_locked;++i) render(1);
        CHECK(!read_slot(owner,DLY).note_locked && read_slot(owner,DLY).physical==0.f &&
              read_slot(owner,REV).physical==1.f);
        CHECK(read_slot(owner,DM).physical==dm && read_slot(owner,RM).physical==rm);
        seq_stop(&seq); render(4);
    }
    printf("PASS independent delay/reverb switches, owner-only PCM, preserved selected mixes and delay history and saved state; %u checks\n",checks);
    return 0;
#endif
}
#endif
