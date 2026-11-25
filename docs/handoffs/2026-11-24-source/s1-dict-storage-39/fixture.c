#include "synth_init_test_helpers.h"
#include "app/s1c_part.h"
#include "dsp/s1c/s1c_front.h"
static int portable_dict(const char *name,int emit)
{
    const char *base=getenv("SMC_UI_SHOTS");CHECK(base);
    char path[1024];snprintf(path,sizeof path,"%s/%s.x0xp",base,name);
    FILE *f=fopen(path,emit?"wb":"rb");CHECK(f);
    uint8_t h[32];
    if(emit) {
        project_capture_stage();project_file_header(h);
        CHECK(fwrite(h,sizeof h,1,f)==1&&fwrite(&project_stage,sizeof project_stage,1,f)==1);
    } else {
        CHECK(fread(h,sizeof h,1,f)==1&&fread(&project_stage,sizeof project_stage,1,f)==1);
    }
    CHECK(!fclose(f));
    return project_file_validate(h);
}
#ifdef DICT_PREVIOUS
int main(void)
{
    setup(V_PART,PART_303A);
    stored_project("with-dict",0,PROJECT_MOUNT);
    unsigned writes=store_writes;
    CHECK(project_load()==-9 && project_save()==-9 && store_writes==writes);
    CHECK(portable_dict("with-dict",0)<0 && store_writes==writes);
    project_device_bt_set(!(proj.set.bt&1u));
    CHECK(project_device_bt_save()==-9 && store_writes==writes);
    stored_project("with-dict",0,PROJECT_VERIFY);
    stored_project("without-dict",0,PROJECT_MOUNT);
    CHECK(!project_load() && !portable_dict("without-dict",0));
    puts("PASS exact prior carrier rejects b39 before USB/native object/BT writes; same typed carrier clears and admits");
}
#else
static s1_dry_part *current,*parts[NBASS];
void __real_s1_part_init(s1_dry_part *);
void __wrap_s1_part_init(s1_dry_part *p) {__real_s1_part_init(p);current=p;}
static void history(int redo)
{
    held_btn=1u<<B_HOME;now_ms++;input();
    held_btn|=1u<<(redo?B_PLAY:B_REC);now_ms++;input();
    held_btn&=~(1u<<(redo?B_PLAY:B_REC));now_ms++;input();
    held_btn=0;now_ms++;input();render(3);
}
static pref_t delay_page(unsigned owner)
{
    if(smc.part!=(int)(PART_303A+owner)) {
        int strip=0;while(strip<NPARTS&&part_at(strip)!=(int)(PART_303A+owner))strip++;
        CHECK(strip<NPARTS);message(0x90,24+strip,127);message(0x80,24+strip,0);
    }
    CHECK(smc.part==(int)(PART_303A+owner));
    set_view(V_PART);ui.part=PART_303A+owner;build_pages();visit("DELAY");
    smc_peek_cancel();smc_page_set(cur_page());
    pref_t r=pg.r[cur_page()][1];
    CHECK(r.kind==R_S1DICT && r.a==T_303 && r.b==owner && smc_same(r,smc_knob_target(1)));
#if X0X_S1_FEEDBACK_PROBE
    CHECK(pg.r[cur_page()][0].kind==R_S1DLFB);
#else
    CHECK(pg.r[cur_page()][0].kind==R_NONE);
#endif
    return r;
}
static void stage(void)
{
    project_reset_stage();project_stage.set.snd_rev=REV137;
    for(unsigned i=0;i<NBASS;i++){
        CHECK(original_sound_init(project_stage.sound.v[T_303][i],ORIGINAL_SOUND_SYNTHONE,0));
        engine_bass_sound_refresh(&project_stage.sound,i);
        slot137(&project_stage.sound,i)[0]=137;
        slot137(&project_stage.sound,i)[1]=i;
    }
    CHECK(!project_validate_stage());project_commit_stage();
    for(unsigned i=0;i<NBASS;i++){
        current=NULL;select_patch(i);CHECK(current);parts[i]=current;
    }
    stabilize();undo_start();
}
static float knob_ratio(unsigned pos) {return (float)(50+pos)*.01f;}
static void read_position(unsigned o,unsigned pos)
{
    pref_t r=delay_page(o);
    char num[16],unit[8],expected[16];
    pref_value(r,num,unit);snprintf(expected,sizeof expected,"%u",pos+50);
    CHECK(pref_get(r)==(int)pos && !strcmp(num,expected)&&!strcmp(unit,"%"));
    CHECK(parts[o]->e.t[137]==knob_ratio(pos) && s1_dict_read(&proj.sound,o)==s1_dict_encode(pos));
}
#ifdef DICT_TRIAL
int main(void)
{
    setup(V_PART,PART_303A);
    stored_project("with-dict",0,PROJECT_MOUNT);
    unsigned writes=store_writes;
    CHECK(!project_load() && s1_typed_tuple_editable());
    stabilize();
    for(unsigned i=0;i<NBASS;i++) {
        pref_t r=delay_page(i);
        CHECK(pref_get(r)==(i?50:0));
        now_ms+=250;knob(1,i?-1:1);
        CHECK(pref_get(r)==(i?49:1));
    }
    CHECK(project_save()==-9 && ui_project_save()==-9 && store_writes==writes);
    stored_project("with-dict",0,PROJECT_VERIFY);
    puts("PASS native trial policy without Feedback: old K1 empty, both owner real K2 editable in RAM, no saves");
}
#else
int main(void)
{
    setup(V_PART,PART_303A);proj.set.bt|=PROJ_BT_CLK_IN;
    CHECK(!s1_typed_dict_choose(0,1)); /* ordinary project, no claimed slot */
    stage();
    for(unsigned p=0;p<=S1_DICT_MAX;p++)CHECK(s1_dict_decode(s1_dict_encode(p))==p);
    sound_t start=proj.sound,after;
    unsigned writes=store_writes;
    for(unsigned o=0;o<2;o++){
        pref_t r=delay_page(o);read_position(o,25);
        CHECK(!s1_typed_dict_choose(NBASS,1) && !s1_typed_dict_choose(o,51));
        s1c *e=&parts[o]->e;
        CHECK(s1c_front_set(e,16,16000.f));
        CHECK(s1c_front_set(e,44,1.f));
        CHECK(s1c_front_set(e,43,.12f));
        CHECK(s1c_front_set(e,40,0.f));
        s1c_set_delay_gate(e,1.f,1);
        CHECK(s1_part_note(parts[o],96,109));
        float l[256],rr[256];for(unsigned i=0;i<96;i++)s1_part_render(parts[o],l,rr,256);
        CHECK(e->dl[0].buf&&e->ddec>0&&e->pool_bytes>0);
        const float hist_before=e->dl_fc;
        uint8_t history_bytes[4096];
        CHECK((unsigned)e->pool_bytes<sizeof history_bytes);
        memcpy(history_bytes,e->pool,e->pool_bytes);
        unsigned peer=s1_dict_read(&proj.sound,1-o),dec=e->ddec;
        int16_t *buf=e->dl[0].buf;
        const unsigned pos=o?50:0;
        for(unsigned j=0;j<25;j++){now_ms+=250;knob(1,o?1:-1);}
        CHECK(pref_get(r)==(int)pos && e->t[137]==knob_ratio(pos));
        CHECK(peer==s1_dict_read(&proj.sound,1-o));
        CHECK(e->t[41]==1.f&&e->t[44]==1.f&&(unsigned)e->ddec==dec &&e->dl[0].buf==buf);
        CHECK(!memcmp(history_bytes,e->pool,e->pool_bytes));
        for(unsigned i=0;i<96;i++)s1_part_render(parts[o],l,rr,256);
        CHECK(o?e->dl_fc<hist_before:e->dl_fc>hist_before);
        CHECK(s1_part_note(parts[o],96,0));
        read_position(o,pos);
    }
    CHECK(store_writes==writes && undo_commit_stable());
    after=proj.sound;
    s1_dict_store(&after,0,0);s1_dict_store(&after,1,0);
    CHECK(!memcmp(&start,&after,sizeof start));
    history(0);read_position(0,25);read_position(1,25);
    history(1);read_position(0,0);read_position(1,50);
    CHECK(store_writes==writes);
    project_capture_stage();s1_dict_store(&project_stage.sound,0,51);CHECK(project_validate_stage()<0);
    project_capture_stage();slot137(&project_stage.sound,1)[1]=0;CHECK(project_validate_stage()<0);
    CHECK(!project_save());stored_project("with-dict",0,PROJECT_EXPORT);CHECK(!portable_dict("with-dict",1));
    CHECK(!project_load_stage());project_commit_stage();stabilize();
    read_position(0,0);read_position(1,50);
    /* Dormant saved control is retained when only the selected base engine changes.
       It is not exposed to, actuated on, or overwritten by the other engine. */
    CHECK(!engine_bass_select_preset(0,ORIGINAL_SOUND_CHICAGO,0,&proj.sound));
    for(unsigned i=0;i<1000&&engine_bass_preset_pending(0);i++)render(1);
    CHECK(engine_bass_bank(0)==ORIGINAL_SOUND_CHICAGO);
    CHECK(s1_typed_dict_position(0)<0&&!s1_typed_dict_choose(0,25)&&s1_dict_read(&proj.sound,0)==1u);
    select_patch(0);read_position(0,0);
    s1_dict_store(&proj.sound,0,0);s1_dict_store(&proj.sound,1,0);
    CHECK(!project_save());stored_project("without-dict",0,PROJECT_EXPORT);CHECK(!portable_dict("without-dict",1));
    printf("PASS ID137 both owners SMK and Delay receiver; exact untouched history at edit, undo/redo/recall, wrong engine and forged stage data refused: %u checks\n",checks);
}
#endif
#endif
