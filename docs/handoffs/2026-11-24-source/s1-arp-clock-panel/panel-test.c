#define ARP_FIXTURE_MAIN retained_keyboard_main
#include "ui_arp_engine_test.c"
#include "original_runtime.h"

static unsigned age,step;
static void one_sample(int32_t pcm[2]) { engine_render(pcm,1); }
static void invoke(unsigned owner,unsigned factor,unsigned due) {
    setup(V_SEQ,PART_303A+owner);
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
    EXPECT(engine_arp_source_get(owner,ARP_F_SOURCE_FACTOR)==5);
    static uint8_t before[sizeof proj.sound];
    memcpy(before,&proj.sound,sizeof before);
    for(unsigned n=0;n<240&&engine_arp_source_get(owner,ARP_F_SOURCE_RATE)<200;++n){ now_ms+=160;turn(0,1); }
    for(unsigned n=5;n<factor;++n)turn(1,1);
    printf("owner %u ARP rate %d clock factor %d\n", owner, engine_arp_source_get(owner,ARP_F_SOURCE_RATE),engine_arp_source_get(owner,ARP_F_SOURCE_FACTOR));
    EXPECT(engine_arp_source_get(owner,ARP_F_SOURCE_RATE)==200);
    EXPECT(engine_arp_source_get(owner,ARP_F_SOURCE_FACTOR)==(int)factor);
    EXPECT(!memcmp(before,&proj.sound,sizeof before));
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
static void held_transport(int enabled) {
    setup(V_PART,PART_303A);
    EXPECT(!engine_bass_select_preset(0,ORIGINAL_SOUND_SYNTHONE,0,&proj.sound));
    for(unsigned n=0;n<1024&&engine_bass_preset_pending(0);++n)render(1);
    engine_arp_cfg c;
    engine_arp_cfg_default(&c);
    c.on=1;c.mode=ARP_SEQ;c.gate=100;c.on_mask=3;c.length=2;c.offset[1]=7;
    EXPECT(engine_arp_preset(0,&c)==S1_EDIT_QUEUED);
    render(4);
    seq.playing=1;
    for(unsigned n=0;n<128&&!seq.metro_view;++n)render(1);
    EXPECT(seq.metro_view);
    if(enabled) EXPECT(engine_host_s1_clock(0,200.f,18));
    input();render(4);input();
    held_keys=1u<<WHITE_KEY[5];++now_ms;input();
    char path[1024];
    snprintf(path,sizeof path,"%s/transport.pcm",getenv("SMC_UI_SHOTS"));
    FILE *f=fopen(path,"wb"); CHECK(f);
    int32_t pcm[512], audio=0;
    for(unsigned i=0;i<142;++i) {
        if (i==80) seq.playing=0; /* Keep the physical key held through STOP. */
        if (i==128){held_keys=0;++now_ms;input();}
        engine_render(pcm,256);
        for(unsigned j=0;j<512;++j)audio|=pcm[j];
        CHECK(fwrite(pcm,sizeof pcm,1,f)==1);
        if (i==79) EXPECT(seq.metro_view);
        if (i==127) EXPECT(engine_arp_view(0,&step,&age));
    }
    EXPECT(!seq.playing && audio && !engine_arp_view(0,&step,&age));
    CHECK(!fclose(f));
    if(enabled) {
        held_keys=1u<<WHITE_KEY[5];++now_ms;input();
        for(unsigned i=0;i<=137;++i){
            one_sample(pcm);EXPECT(engine_arp_view(0,&step,&age)&&step==(i==137));
        }
        held_keys=0;++now_ms;input();render(2);
    }
}
int main(void) {
    if(getenv("CLOCK_TRANSPORT")) {
        held_transport(!getenv("CLOCK_TRANSPORT_STOCK"));
        if(failures)return 1;
        puts("PASS held key across transported playback and STOP");
        return 0;
    }
    invoke(0,18,137);invoke(1,17,206);
    if(failures)return 1;
    printf("PASS original app ARP exact onsets 137/206, SMK ARP RATE/CLOCK FACTOR/GATE detents and live keys/audio, both owners: %u assertions\n",expectations);
}
