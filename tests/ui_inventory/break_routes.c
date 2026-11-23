/* SPDX-License-Identifier: GPL-3.0-only */
#define SMC_UI_MAIN smc_fixture_main
#include "../host/smc_ui_test.c"

static void capture(const char *name)
{
    for(int block=0;block<8;++block)audio_block();
    now_ms+=90;
    ui_frame();
    shot(name);
}

static void turn(int encoder)
{
    now_ms+=100;
    enc_acc[encoder]=1;
    input();
}

static void enter_break(void)
{
    setup(V_HOME,PART_909);
    for(int attempt=0;ui.part!=PART_BRK && attempt<NPARTS;++attempt)turn(EN_ALGO);
    CHECK(ui.part==PART_BRK && ui.overlay==O_NONE);
}

int main(void)
{
    enter_break();
    capture("home-break");
    button_tap(B_EDIT);
    CHECK(ui.view==V_PART && cur_page()==0);
    capture("loops");
#ifdef SHIPPING_026
    /* The old knobs apply selection immediately; the preview state arrived later. */
    for(int bank=0;bank<2;++bank) {
        int before=bank?cur_pat()->brk.slot_b:cur_pat()->brk.slot_a;
        const int encoder=bank?EN_K2:EN_K1;
        now_ms+=100;
        enc_acc[encoder]=before? -1:1;
        input();
        CHECK((bank?cur_pat()->brk.slot_b:cur_pat()->brk.slot_a)!=before);
        capture(bank?"preview-b-choose":"preview-a-choose");
    }
#else
    for(int bank=0;bank<2;++bank) {
        turn(bank?EN_K2:EN_K1);
        const char *preview=bank?"preview-b":"preview-a";
        CHECK(engine_brk_preview_active() || ui.brk_choice_on);
        capture(preview);
        button_tap(B_HOME);
        CHECK(!ui.brk_choice_on);
        capture(bank?"preview-b-cancel":"preview-a-cancel");
        CHECK(ui.view==V_PART);
        turn(bank?EN_K2:EN_K1);
        int slot=ui.brk_choice_slot;
        button_tap(B_SEL);
        CHECK(!ui.brk_choice_on);
        CHECK((bank?cur_pat()->brk.slot_b:cur_pat()->brk.slot_a)==slot);
        capture(bank?"preview-b-choose":"preview-a-choose");
    }
#endif
    button_tap(B_SEL);
    CHECK(ui.overlay==O_LIST);
    capture("loops-more");

    enter_break();
    button_tap(B_SEQ);
    CHECK(ui.view==V_SEQ);
    const char *const pages[]={"sequence","groove","retrig"};
    CHECK(pg.n==3);
    for(int page=0;page<3;++page) {
        CHECK(cur_page()==page);
        capture(pages[page]);
        if(page<2)button_tap(B_SEQ);
    }
    button_tap(B_SEL);
    CHECK(ui.overlay==O_LIST);
    capture("sequence-more");

    enter_break();
    button_tap(B_SEQ);
    cur_pat()->brk.steps=1u<<10;
    held_keys=1u<<WHITE_KEY[10];
    ++now_ms;
    input();
    CHECK(ui.brk_held==10);
    capture("sequence-held");
#ifdef SHIPPING_026
    held_keys|=1u<<BLACK_KEY[2];
    ++now_ms;
    input();
    CHECK(cur_pat()->brk.slice[10]==3);
    held_keys=0;
    ++now_ms;
    input();
    CHECK(ui.brk_held<0);
    now_ms+=1800;
    capture("sequence-released-slice");
#else
    button_tap(B_EDIT);
    CHECK(ui.view==V_PART && lock_step()==10);
    turn(EN_K4);
    CHECK(lock_of(PR(R_ENG,T_BRK,0,1),10)>=0);
    capture("loops-held-lock");
    held_keys=0;
    ++now_ms;
    input();
    CHECK(lock_step()<0);
    capture("loops-released-lock");
#endif
    printf("BREAK physical route checks: %u\n",checks);
    return 0;
}
