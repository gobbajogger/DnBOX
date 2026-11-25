#define MIXED_ONLY 1
#include "s1_pb_mod_ui.c"
#include "original_runtime.h"

static unsigned age,step;
static void one_sample(int32_t pcm[2]) { engine_render(pcm,1); }
static void invoke(unsigned owner,unsigned factor,unsigned due) {
    ui.part=PART_303A+owner;set_view(V_SEQ);build_pages();
    EXPECT(!engine_bass_select_preset(owner, ORIGINAL_SOUND_SYNTHONE,0,&proj.sound));
    for(unsigned n=0;n<1024&&engine_bass_preset_pending(owner);++n)render(1);
    EXPECT(engine_bass_bank(owner)==ORIGINAL_SOUND_SYNTHONE);
    engine_arp_cfg cfg;
    engine_arp_cfg_default(&cfg);
    cfg.on=1;cfg.mode=ARP_SEQ;cfg.gate=100;cfg.on_mask=3;cfg.length=2;cfg.offset[1]=7;
    EXPECT(engine_arp_preset(owner,&cfg)==S1_EDIT_QUEUED);
    render(4);
    *page_of(V_SEQ)=3;build_pages(); EXPECT(arp_seq_page());
    EXPECT(pg.r[cur_page()][0].kind==R_ARP&&pg.r[cur_page()][2].a==ARP_F_GATE);
    turn(2,-1);turn(2,1);
    engine_arp_settings settings;
    EXPECT(engine_arp_settings_read(owner,&settings)&&settings.gate==100&&settings.length==2);
    set_view(V_PART);build_pages();
    const int arp=page_named("ARPEGGIATOR");
    EXPECT(arp>=0 && page_named("ARP PARAM")==arp+1);
    *page_of(V_PART)=(uint8_t)arp;build_pages();
    now_ms+=160;enc_acc[EN_SELECT]=1;input();
    EXPECT(!strcmp(page_title(&pg,cur_page()),"ARP PARAM"));
    EXPECT(engine_arp_source_get(owner,ARP_F_SOURCE_RATE)==120);
    EXPECT(engine_arp_source_get(owner,ARP_F_SOURCE_FACTOR)==9);
    static uint8_t before[sizeof proj.sound];
    memcpy(before,&proj.sound,sizeof before);
    for(unsigned n=0;n<240&&engine_arp_source_get(owner,ARP_F_SOURCE_RATE)<200;++n){ now_ms+=160;turn(0,1); }
    CHECK(arp_desc(ARP_F_SOURCE_FACTOR,owner)->def==9);
    turn(1,1);CHECK(engine_arp_source_get(owner,ARP_F_SOURCE_FACTOR)==8);
    turn(1,-1);CHECK(engine_arp_source_get(owner,ARP_F_SOURCE_FACTOR)==9);
    for(unsigned n=9;n<factor;++n)turn(1,-1);
    printf("owner %u ARP rate %d clock factor %d\n", owner, engine_arp_source_get(owner,ARP_F_SOURCE_RATE),engine_arp_source_get(owner,ARP_F_SOURCE_FACTOR));
    EXPECT(engine_arp_source_get(owner,ARP_F_SOURCE_RATE)==200);
    EXPECT(engine_arp_source_get(owner,ARP_F_SOURCE_FACTOR)==(int)factor);
    sound_t exact;
    memcpy(&exact,before,sizeof exact);
    memcpy(slot137(&exact,owner)+S1_ARP_CLOCK_BYTE,
        read137(&proj.sound,owner)+S1_ARP_CLOCK_BYTE,S1_ARP_CLOCK_BYTES);
    EXPECT(!memcmp(&exact,&proj.sound,sizeof exact));
    EXPECT(!engine_arp_source_set(owner,ARP_F_SOURCE_RATE,201));
    EXPECT(!engine_arp_source_set(owner,ARP_F_SOURCE_FACTOR,19));
    input(); render(4); input();
    const int key=WHITE_KEY[5];
    held_keys=1u<<key;++now_ms;input();
    int32_t pcm[2],audio=0;
    for(unsigned i=0;i<=due;++i) {
        one_sample(pcm);audio|=pcm[0]|pcm[1];
        EXPECT(engine_arp_view(owner,&step,&age)&&step==(i==due));
    }
    EXPECT(audio);
    // Start actual transported playback while the independent clock is sounding.
    seq.playing=1;
    for(unsigned n=0;n<128&&!seq.metro_view;++n)render(1);
    EXPECT(seq.metro_view);
    render(24);
    seq.playing=0;render(12);
    held_keys=0;++now_ms;input();render(3);
    EXPECT(!engine_arp_view(owner,&step,&age));
    // Re-press starts original phase; change stock GATE while a key is held, then release.
    held_keys=1u<<key;++now_ms;input();
    for(unsigned i=0;i<=due;++i) {
        one_sample(pcm); EXPECT(engine_arp_view(owner,&step,&age)&&step==(i==due));
    }
    EXPECT(engine_arp_settings_read(owner,&settings));
    EXPECT(engine_arp_set(owner,settings.revision,ARP_F_GATE,55)==S1_EDIT_QUEUED);
    render(8);
    held_keys=0;++now_ms;input();render(2);
    EXPECT(!engine_arp_view(owner,&step,&age));
    EXPECT(!engine_bass_keys_held());
    EXPECT(engine_arp_settings_read(owner,&settings)&&settings.gate==55);
    held_keys=0;++now_ms;input();render(2);
    EXPECT(!engine_arp_view(owner,&step,&age));
}

static void view_clock(unsigned owner,int rate,int factor,int ro)
{
    ui.overlay=O_NONE;ui.part=PART_303A+owner;set_view(V_PART);build_pages();
    visit("ARP PARAM");
    const pref_t r=pg.r[cur_page()][0], f=pg.r[cur_page()][1];
    CHECK(r.b==owner&&f.b==owner&&pref_get(r)+1==rate&&pref_get(f)==18-factor);
    CHECK(pref_read_only(r)==ro&&pref_read_only(f)==ro);
}
static void after_reopen(unsigned owner,unsigned due)
{
    engine_arp_settings settings;
    CHECK(engine_arp_settings_read(owner,&settings));
    CHECK(settings.gate==100);
    input();render(3);input();
    held_keys=1u<<WHITE_KEY[5];++now_ms;input();
    int32_t pcm[2],audio=0;
    for(unsigned i=0;i<=due;++i) {
        one_sample(pcm);audio|=pcm[0]|pcm[1];
        CHECK(engine_arp_view(owner,&step,&age)&&step==(i==due));
    }
    CHECK(audio);
    held_keys=0;++now_ms;input();render(2);
    CHECK(!engine_arp_view(owner,&step,&age));
}
int main(void) {
    setup(V_PART,PART_303A);proj.set.bt|=PROJ_BT_CLK_IN;
    select_patch(0);set_view(V_PART);build_pages();
    CHECK(page_named("ARP PARAM")<0&&!engine_arp_source_set(0,ARP_F_SOURCE_RATE,163));
    fresh();
    invoke(0,18,137);invoke(1,17,206);
    CHECK(!failures);
    printf("PASS 738 retained physical clock/phase/transition checks, both owners: actual count %u\n",expectations);
    view_clock(0,200,18,0);view_clock(1,200,17,0);
    undo_start();detent(0,-1);CHECK(engine_arp_source_get(1,ARP_F_SOURCE_RATE)==199);
    CHECK(undo_commit_stable());physical_history(0);view_clock(1,200,17,0);
    physical_history(1);view_clock(1,199,17,0);view_clock(0,200,18,0);
    project_capture_stage();uint8_t *p=slot137(&project_stage.sound,0)+S1_ARP_CLOCK_BYTE;
    p[1]=1;CHECK(project_validate_stage()<0);
    project_capture_stage();p=slot137(&project_stage.sound,0)+S1_ARP_CLOCK_BYTE;
    p[2]=20;CHECK(project_validate_stage()<0);
    for(unsigned v=0;v<NBASS;++v) {
        engine_arp_settings settings;
        CHECK(engine_arp_settings_read(v,&settings));
        CHECK(engine_arp_set(v,settings.revision,ARP_F_GATE,100)==S1_EDIT_QUEUED);
        render(3);
    }
    witness=proj;CHECK(!project_save()&&!project_load_stage());project_commit_stage();stabilize();undo_start();
    CHECK(!memcmp(&witness.sound,&proj.sound,sizeof proj.sound));
    view_clock(0,200,18,0);after_reopen(0,137);
    view_clock(1,199,17,0);after_reopen(1,207);
    // Export actual typed writer output, never a recreated fake row.
    project_capture_stage();uint8_t header[32];project_file_header(header);CHECK(!project_file_validate(header));
    FILE *external=fopen("out/media.bin","wb");CHECK(external);
    CHECK(fwrite(header,sizeof header,1,external)==1&&fwrite(&project_stage,sizeof project_stage,1,external)==1&&
          fwrite(store,sizeof store,1,external)==1&&fwrite(store_len,sizeof store_len,1,external)==1);
    CHECK(!fclose(external));
    // The recovered generation remains visible, but both UI knobs and save are blocked.
    uint8_t saved[PLAT_STORE_MAX];memcpy(saved,store[OBJ_SOUND],sizeof saved);
    detent(0,-1);detent(1,1);
    CHECK(engine_arp_source_get(1,ARP_F_SOURCE_RATE)==198&&engine_arp_source_get(1,ARP_F_SOURCE_FACTOR)==16);CHECK(!project_save());
    memcpy(store[OBJ_SOUND],saved,sizeof saved);unsigned writes=store_writes;
    CHECK(!project_load());stabilize();view_clock(0,200,18,1);view_clock(1,199,17,1);
    detent(0,-1);detent(1,-1);
    CHECK(pref_get(pg.r[cur_page()][0])+1==199&&pref_get(pg.r[cur_page()][1])==1);
    CHECK(project_save()==-9&&writes==store_writes);
    printf("PASS typed S1 original clock both owners, Undo/Redo, reopen and mixed-generation read-only: %u checks\n",checks);
    return 0;
}
