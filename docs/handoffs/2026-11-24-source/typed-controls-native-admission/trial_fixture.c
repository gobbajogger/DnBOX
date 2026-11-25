#include "synth_init_test_helpers.h"
enum { B = 2 };
static uint8_t portable[32+sizeof(project_t)];
static void file(const char *name,int write)
{
    const char *base = getenv("TRIAL_INPUT"); CHECK(base);
    char path[1024]; snprintf(path,sizeof path,"%s/%s.x0xp",base,name);
    FILE *f=fopen(path,write?"wb":"rb");CHECK(f);
    CHECK(write ? fwrite(portable,sizeof portable,1,f)==1 : fread(portable,sizeof portable,1,f)==1);
    CHECK(!fclose(f));
}
static void portable_make(const char *name)
{
    project_capture_stage();
    project_file_header(portable);
    memcpy(portable+32,&project_stage,sizeof project_stage);
    file(name,1);
}
static int portable_accept(const char *name)
{
    file(name,0);
    memcpy(&project_stage,portable+32,sizeof project_stage);
    return project_file_validate(portable);
}
#ifdef TRIAL_WRITER
static void make_typed(const char *name,int unsupported)
{
    project_reset_stage(); project_stage.set.snd_rev=REV137;
    for(unsigned i=0;i<NBASS;i++){
        CHECK(original_sound_init(project_stage.sound.v[T_303][i],ORIGINAL_SOUND_SYNTHONE,0));
        engine_bass_sound_refresh(&project_stage.sound,i);
        uint8_t *b=slot137(&project_stage.sound,i);
        b[0]=137;b[1]=i;b[3]=(uint8_t)(i+1);
        b[S1_A4_BYTE]=s1_a4_code(i?437:446);
        b[S1_PAN_AMOUNT_BYTE]=i?48:32;
        s1_pan_rate_put(b,i?.5f:.25f);
        if(unsupported)b[2]=7;
    }
    CHECK(!project_validate_stage());project_commit_stage();stabilize();
    if(!unsupported) for(unsigned i=0;i<NBASS;i++){
        unsigned pos,max;int sync;float hz;
        CHECK(s1_typed_pan_read(i,&pos,&max,&sync,&hz));
        printf("writer owner %u pan stores %u / %.9f applied %.9f sync=%d\n",i,
            read137(&proj.sound,i)[S1_PAN_AMOUNT_BYTE],s1_pan_rate_hz(s1_pan_rate_word(read137(&proj.sound,i))),hz,sync);
    }
    CHECK(!project_save());
    stored_project(name,0,PROJECT_EXPORT);
    portable_make(name);
}
int main(void)
{
    setup(V_PART,PART_303A);
    project_reset_stage(); project_commit_stage(); stabilize();
    CHECK(!project_save());stored_project("ordinary",0,PROJECT_EXPORT);
    portable_make("ordinary");
    make_typed("admitted",0);make_typed("unsupported",1);
    puts("PASS exact writer produced coherent physical-project tuple and inner files using production stage/save");
}
#elif defined(TRIAL_OLD)
int main(void)
{
    setup(V_PART,PART_303A);
    CHECK(!portable_accept("ordinary"));
    stored_project("ordinary",0,PROJECT_MOUNT);CHECK(!project_load());
    CHECK(proj.set.snd_rev==PROJ_SND_REV);
    CHECK(!project_save()); /* no trial flags, ordinary writer still active */
    CHECK(portable_accept("admitted"));
    stored_project("admitted",0,PROJECT_MOUNT);
    unsigned w=store_writes;
    CHECK(project_load()==-9 && project_save()==-9 && store_writes==w);
    project_device_bt_set(!(proj.set.bt&1u));CHECK(project_device_bt_save()==-9 && store_writes==w);
    stored_project("admitted",0,PROJECT_VERIFY);
    puts("PASS flag-off stock-equivalent reader accepts ordinary and rejects typed newer media, zero writes");
}
#else
static void no_writes(void)
{
    unsigned old=store_writes;
    CHECK(project_save()==-9 && ui_project_save()==-9);
    project_device_bt_set(!(proj.set.bt&1u));
    CHECK(project_device_bt_save()==-9);
    ui.dirty=1;now_ms+=80000;autosave();device_bt_save();
    CHECK(store_writes==old);
}
static void dial_owner(unsigned owner)
{
    if(smc.part!=PART_303A+owner) {
        unsigned s=0;for(;s<NPARTS&&part_at(s)!=(int)(PART_303A+owner);s++);
        CHECK(s<NPARTS);message(0x90,24+s,127);message(0x80,24+s,0);
    }
    CHECK(smc.part==PART_303A+owner);
    ui.part=PART_303A+owner;set_view(V_PART);build_pages();visit("AUTO PAN");
    smc_peek_cancel();smc_page_set(cur_page());
    CHECK(smc_same(pg.r[cur_page()][0],smc_knob_target(0)));
    CHECK(pg.r[cur_page()][0].kind==R_S1PAN);
    now_ms+=250;knob(0,1);
}
int main(void)
{
    setup(V_PART,PART_303A);
    CHECK(!portable_accept("ordinary"));
    stored_project("ordinary",0,PROJECT_MOUNT);
    CHECK(!project_load());stabilize();
    CHECK(proj.set.snd_rev==PROJ_SND_REV);no_writes();
    stored_project("ordinary",0,PROJECT_VERIFY);
    CHECK(portable_accept("unsupported"));
    stored_project("unsupported",0,PROJECT_MOUNT);
    unsigned w=store_writes;
    CHECK(project_load()==-9 && store_writes==w); no_writes();
    stored_project("unsupported",0,PROJECT_VERIFY);

    stored_project("admitted",0,PROJECT_MOUNT);
    CHECK(!project_load());stabilize();
    CHECK(proj.set.snd_rev==REV137 && s1_typed_tuple_editable());
    CHECK(!portable_accept("admitted"));
    CHECK(!ui_project_begin()); /* explicit import, no probing or default seeding */
    CHECK(!portable_accept("admitted"));ui_project_end(1);stabilize();
    for(unsigned o=0;o<NBASS;o++){
        CHECK(live_s1_mod(&proj.sound,o) && read137(&proj.sound,o)[3]==o+1);
        unsigned pos,max;int sync;float hz;
        CHECK(s1_typed_pan_read(o,&pos,&max,&sync,&hz));
        CHECK(s1_pan_rate_hz(s1_pan_rate_word(read137(&proj.sound,o)))==(o?.5f:.25f));
        CHECK(hz>0.f);
        printf("trial owner %u recalled applied pan rate %.9f sync=%d\n",o,hz,sync);
        const unsigned peer=read137(&proj.sound,1-o)[S1_PAN_AMOUNT_BYTE];
        unsigned own=read137(&proj.sound,o)[S1_PAN_AMOUNT_BYTE];
        dial_owner(o);
        CHECK(read137(&proj.sound,o)[S1_PAN_AMOUNT_BYTE]==own+1 && read137(&proj.sound,1-o)[S1_PAN_AMOUNT_BYTE]==peer);
    }
    engine_bass_on(0,60,0,0);engine_bass_on(1,67,0,0);render(6);
    record(reference);
    CHECK(energy(reference,0)>0 && energy(reference,1)>0);
    no_writes();stored_project("admitted",0,PROJECT_VERIFY);

    /* Mix two individually valid generations. No matching persisted tuple is
     * present in this fixture's single-generation store. Refuse, never migrate. */
    uint8_t typed_sound[PLAT_STORE_MAX];
    const int n=plat_store_load(OBJ_SOUND,typed_sound,sizeof typed_sound);
    CHECK(n==4+sizeof(sound_t)); sound_t foreign;memcpy(&foreign,typed_sound+4,sizeof foreign);
    slot137(&foreign,0)[3]=2;CHECK(canonical137(&foreign));memcpy(typed_sound+4,&foreign,sizeof foreign);
    CHECK(!plat_store_save(OBJ_SOUND,typed_sound,(unsigned)n));
    w=store_writes; CHECK(!project_load() && store_writes==w && !s1_typed_tuple_editable());
    CHECK(read137(&proj.sound,0)[3]==1u);
    no_writes();
    for(unsigned i=0;i<OBJ_NOBJ;i++) store_previous_len[i]=-1;
    w=store_writes;CHECK(project_load()==-9 && store_writes==w);no_writes();
    puts("PASS trial policy real portable/flash admission, unsupported/torn refusal, both-owner RAM editing and playback, no saves");
}
#endif
