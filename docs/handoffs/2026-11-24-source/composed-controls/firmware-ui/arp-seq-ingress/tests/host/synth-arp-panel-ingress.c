/* SPDX-License-Identifier: GPL-3.0-only */
/* Real full UI button/encoder ingress. No direct page mutation or direct edit API. */
#define SMC_UI_MAIN retained_smc_fixture_main
#include "smc_ui_test.c"
static void ingress_audio(unsigned n) { while(n--) { int32_t out[512]; engine_render(out,256); } }
static void ingress_tap(unsigned b) {
    now_ms+=120; held_btn=1u<<b;input();now_ms+=20;held_btn=0;input();ingress_audio(2);ui_frame();
}
static void ingress_enc(unsigned k,int delta) {
    now_ms+=200;enc_acc[k]=delta;input();ingress_audio(2);ui_frame();
}
static void ingress_key(unsigned w,int down) {
    now_ms+=10;held_keys=down?1u<<WHITE_KEY[w]:0;input();ingress_audio(2);ui_frame();
}
static void perform(unsigned owner,unsigned bank) {
    engine_arp_settings s,t,peer0,peer1;
    engine_arp_steps a,b,ap,bp;
    setup(V_HOME,PART_303A+owner); /* fixture sets which owner is initially selected */
    for(unsigned voice=0;voice<2;voice++) {
        CHECK(!engine_bass_select_preset(voice,bank,0,&proj.sound));
        for(unsigned n=0;n<1000&&engine_bass_preset_pending(voice);n++)ingress_audio(1);
        CHECK(engine_bass_bank(voice)==(int)bank);
    }
    CHECK(ui.view==V_HOME && ui.overlay==O_NONE);

    ingress_tap(B_SEQ); CHECK(ui.view==V_SEQ && cur_page()==0 && !arp_seq_page());
    for(unsigned i=0;i<3;i++) { ingress_enc(EN_SELECT,1); CHECK(cur_page()==(int)i+1); }
    CHECK(arp_seq_page() && !strcmp(pg.title[cur_page()],"ARP SEQ"));
    CHECK(pg.r[cur_page()][2].kind==R_ARP);
    CHECK(pg.r[cur_page()][2].a==ARP_F_GATE && pg.r[cur_page()][2].b==owner);
    shot(bank==ORIGINAL_SOUND_JUNO?(owner?"sj6-B-enter":"sj6-A-enter"):(owner?"s1-B-enter":"s1-A-enter"));
    CHECK(engine_arp_settings_read(owner,&s) && engine_arp_settings_read(1-owner,&peer0));
    int delta=s.gate>1?-1:1;
    ingress_enc(EN_K1+2,delta);
    CHECK(engine_arp_settings_read(owner,&t) && t.gate==(int)s.gate+delta && t.revision!=s.revision);
    CHECK(t.length==s.length && t.mode==s.mode && t.interval==s.interval);
    CHECK(engine_arp_settings_read(1-owner,&peer1) && peer1.gate==peer0.gate && peer1.revision==peer0.revision);
    CHECK(pref_get(pg.r[cur_page()][2])==(int)t.gate-1);
    CHECK(engine_arp_steps_read(owner,&a) && engine_arp_steps_read(1-owner,&ap));
    ingress_key(5,1); ingress_enc(EN_K1,a.offset[5]>0?-1:1); ingress_key(5,0);
    CHECK(engine_arp_steps_read(owner,&b) && engine_arp_steps_read(1-owner,&bp));
    CHECK(b.offset[5]==a.offset[5]+(a.offset[5]>0?-1:1) && a.on_mask==b.on_mask);
    CHECK(ap.revision==bp.revision && ap.offset[5]==bp.offset[5]);
    shot(bank==ORIGINAL_SOUND_JUNO?(owner?"sj6-B-change":"sj6-A-change"):(owner?"s1-B-change":"s1-A-change"));
    ingress_tap(B_HOME); CHECK(ui.view==V_HOME && !arp_seq_page() && ui.overlay==O_NONE);
    CHECK(engine_arp_settings_read(owner,&s) && s.revision==b.revision && s.gate==t.gate);
    CHECK(arp_held<0);
    printf("PASS bank=%u owner=%u real HOME>SEQ>Select*3>K3/Hold K1>HOME gate=%u step6=%d\n",bank,owner,s.gate,b.offset[5]);
}
int main(void) {
    for(unsigned owner=0;owner<2;owner++) {
        perform(owner,ORIGINAL_SOUND_SYNTHONE);
        perform(owner,ORIGINAL_SOUND_JUNO);
    }
    return 0;
}
