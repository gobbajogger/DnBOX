/* SPDX-License-Identifier: GPL-3.0-only */
#define main retained_legacy_main
#include "vibrato-mixed-helpers.h"
#undef main
#include "../../firmware/src/app/s1c_part.h"

static void go_page(int view,int kind,int knob)
{
    set_view(view); build_pages();
    for(int n=0;n<pg.n;n++) {
        if(pg.r[cur_page()][knob].kind==kind)return;
        now_ms+=160;enc_acc[EN_SELECT]=1;input();
    }
    CHECK(0 && "Expected physical page");
}
static void physical_detent(int k,int delta)
{
    now_ms+=160;
    turn(k,delta);
}
static void label(pref_t r,const char *wanted)
{
    char num[32],unit[16];
    pref_value(r,num,unit);
    CHECK(!strcmp(num,wanted));
}
static void settle(unsigned owner)
{
    for(unsigned i=0;i<1024&&engine_bass_preset_pending(owner);i++)render(1);
    CHECK(!engine_bass_preset_pending(owner));
}
static void pattern_case(unsigned owner)
{
    fresh_setup(owner);
    proj.set.bt |= PROJ_BT_CLK_IN;
    select_bank(owner,ORIGINAL_SOUND_SYNTHONE);
    go_page(V_SEQ,R_BPATSND,0);
    pref_t r=pg.r[cur_page()][0];
    CHECK(r.b==owner);
    uint8_t *s=&proj.pat[0].rsv[owner];
    CHECK(!*s);
    physical_detent(0,1);
    CHECK(*s==0xc1u);
    ui.dirty=0;
    physical_detent(0,1);                 /* historical regression: INIT wrapped to OFF */
    CHECK(*s==0xc1u && !ui.dirty);
    label(r,"INIT");
    physical_detent(0,-1);
    CHECK(!*s);label(r,"OFF");
    ui.dirty=0;
    physical_detent(0,-1);
    CHECK(!*s && !ui.dirty);
    CHECK(pref_desc(r)->max==1);
    for(unsigned code=0xc1;code<=0xc4;code++) {
        *s=(uint8_t)code; label(r,"INIT");
        ui.dirty=0;physical_detent(0,1);CHECK(*s==code && !ui.dirty);
        /* Saved historical bytes remain unchanged, never normalized to C1. */
        save_and_reload();
        CHECK(*s==code);
        physical_detent(0,-1);CHECK(!*s);
    }
    /* Cue a different pattern's saved historical INIT while another
       preset is selected; the production lookahead requests INIT. */
    proj.pat[1].rsv[owner]=0xc4;
    CHECK(!engine_bass_select_preset(owner,ORIGINAL_SOUND_SYNTHONE,2,&proj.sound));
    settle(owner);
    CHECK(engine_bass_preset_index(owner)==2);
    save_and_reload();
    engine_apply_sound(&proj.sound);render(4);
    seq.playing=1;
    seq_cue_part(&seq,PART_303A+owner,1);
    CHECK(seq_cue_of(&seq,PART_303A+owner)==1);
    pattern_sound_tick();
    CHECK(engine_bass_requested_preset(owner)==0);
    CHECK(engine_bass_preset_gate(owner)==1);
    CHECK(engine_bass_preset_index(owner)==2); /* old pattern remains untouched */
    /* Start directly into the cued pattern: first real scheduled onset
       must already use INIT rather than the previous factory ordinal. */
    seq.playing=0;
    proj.pat[1].bass[owner].step[0].flags=G_NOTE;
    proj.pat[1].bass[owner].step[0].note=60;
    proj.pat[1].bass[owner].step[0].velocity=110;
    seq_cue_part(&seq,PART_303A+owner,1);
    seq_start(&seq);
    int started=0;
    for(unsigned i=0;i<32 && !started;i++) {
        render(1);
        unsigned gate=0;
        float level=0,modulation=0; unsigned state=0;
        started=engine_bass_voice_view(owner,&level,&modulation,&state,&gate)&&gate;
    }
    CHECK(started && engine_bass_preset_index(owner)==0);
    seq_stop(&seq);render(8);
    CHECK(proj.pat[1].rsv[owner]==0xc4);
    for(unsigned i=0;i<3;i++){
        const unsigned bank[]={ORIGINAL_SOUND_CHICAGO,ORIGINAL_SOUND_MIAMI,ORIGINAL_SOUND_JUNO};
        const char *name[]={"C1","M1","J1"};
        select_bank(owner,bank[i]);
        go_page(V_SEQ,R_BPATSND,0);
        r=pg.r[cur_page()][0];
        CHECK(pref_desc(r)->max==original_sound_factories(bank[i]));
        proj.pat[1].rsv[owner]=0;
        physical_detent(0,1);
        CHECK(proj.pat[1].rsv[owner]==bass_pattern_preset_encode(bank[i],1));
        label(r,name[i]);
        physical_detent(0,-1);
        CHECK(!proj.pat[1].rsv[owner]);
    }
}
static void high_picker(unsigned owner)
{
    fresh_setup(owner);select_bank(owner,ORIGINAL_SOUND_SYNTHONE);
    const int lower[]={255,693};
    for(unsigned n=0;n<2;n++){
        CHECK(!engine_bass_select_preset(owner,ORIGINAL_SOUND_SYNTHONE,lower[n],&proj.sound));
        settle(owner);
        go_page(V_PART,R_BPRESET,1);
        physical_detent(1,1);settle(owner);
        CHECK(engine_bass_preset_index(owner)==lower[n]+1);
        CHECK(original_sound_s1_program(proj.sound.v[T_303][owner])==(unsigned)lower[n]+1);
        if(n) { physical_detent(1,1);settle(owner);CHECK(engine_bass_preset_index(owner)==694); }
        physical_detent(1,-1);settle(owner);CHECK(engine_bass_preset_index(owner)==lower[n]);
    }
}
int main(void)
{
    static uint8_t pack[61440] __attribute__((aligned(4)));
    const char *p=getenv("S1_EDITOR_TEST_PACK");
    FILE *f=p?fopen(p,"rb"):NULL;CHECK(f);
    size_t n=fread(pack,1,sizeof pack,f);fclose(f);
    CHECK(n==61208 && s1_part_set_pack(pack,n)==694);
    for(unsigned owner=0;owner<2;owner++){pattern_case(owner);high_picker(owner);}
    printf("PASS real PAT/WIRE/save/cue and 695-part picker across both bass owners; checks=%u\n",checks);
    return 0;
}
