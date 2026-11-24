#define ARP_FIXTURE_MAIN retained_arp_main
#include "ui_arp_engine_test.c"
static void detent(int dir) {
    now_ms += 200; enc_acc[EN_SELECT]=dir; input(); ui_frame();
}
static void take_frame(unsigned owner,const char *key,int page,const char *title) {
    char name[64];
    CHECK(cur_page()==page && pg.n==15);
    CHECK(!strcmp(pg.title[cur_page()],title));
    if(page==14) {
        CHECK(pg.r[page][0].kind==R_BENGINE && pg.r[page][0].b==owner);
        CHECK(pg.r[page][1].kind==R_BPRESET && pg.r[page][1].b==owner);
        CHECK(pg.r[page][2].kind==R_BSTATUS && pg.r[page][2].b==owner);
    }
    ui_frame(); snprintf(name,sizeof name,"%c-%s",'A'+owner,key); shot(name);
    printf("%s %d/%d %s\n",name,page+1,pg.n,title);
}
int main(void) {
    for(unsigned o=0;o<2;o++){
        setup(V_PART,PART_303A+o); proj.set.bt|=PROJ_BT_CLK_IN;
        CHECK(!engine_bass_select_preset(o,ORIGINAL_SOUND_JUNO,0,&proj.sound));
        for(int n=0;n<1024&&engine_bass_preset_pending(o);n++)render(1);
        CHECK(engine_bass_bank(o)==ORIGINAL_SOUND_JUNO); build_pages();
        for(int tries=0;cur_page()!=0&&tries<15;tries++)detent(1);
        take_frame(o,"start",0,"OSCILLATOR");
        for(int i=1;i<=13;i++) { detent(1); CHECK(cur_page()==i); }
        take_frame(o,"forward-arp",13,"ARPEGGIATOR");
        detent(1);take_frame(o,"forward-sound",14,"SOUND");
        detent(1);take_frame(o,"wrapped-first",0,"OSCILLATOR");
        detent(-1);take_frame(o,"backward-sound",14,"SOUND");
        detent(-1);take_frame(o,"backward-arp",13,"ARPEGGIATOR");
        for(int i=12;i>=0;i--) { detent(-1); CHECK(cur_page()==i); }
        take_frame(o,"returned",0,"OSCILLATOR");
        printf("PASS owner %c all 15 pages reached in both directions\n",'A'+o);
    }
    return 0;
}
