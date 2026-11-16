/* SPDX-License-Identifier: GPL-3.0-only */
#define SMC_UI_MAIN unused_smc_ui_main
#include "smc_ui_test.c"
int main(void)
{
    setup(V_PART,PART_303A);
    CHECK(engine_bass_select_preset(0,ORIGINAL_SOUND_JUNO,0,&proj.sound)==0);
    int32_t audio[512];
    for(int j=0;j<1024&&engine_bass_preset_pending(0);j++)engine_render(audio,256);
    CHECK(engine_bass_bank(0)==ORIGINAL_SOUND_JUNO);build_pages();
    for(int p=0;p<pg.n;p++) {*page_of(V_PART)=(uint8_t)p;ui.dirty=1;for(int j=0;j<4;j++)blit_hash[j]=0;ui_frame();char name[40];snprintf(name,sizeof name,"juno-page-%d",p);shot(name);}
    for(int id=0;id<16;id++)CHECK(orig_strip_slot(PR(R_ENG,T_303,0,id)));
    char n[16],u[8];pref_value_of(PR(R_ENG,T_303,0,2),0,n,u);CHECK(!strcmp(n,"35"));CHECK(!strcmp(u,"Hz"));
    pref_value_of(PR(R_ENG,T_303,0,10),0,n,u);CHECK(!strcmp(n,"0.1"));CHECK(!strcmp(u,"Hz"));
    puts("Juno R22 routing/units PASS");return 0;
}
