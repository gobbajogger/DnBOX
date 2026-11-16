/* SPDX-License-Identifier: GPL-3.0-only */
/* Full-size actual host UI, intentionally incomplete WIP presentation. */
#define SMC_UI_MAIN unused_wip_smc_main
#include "smc_ui_test.c"
static void take(const char *name)
{
    ui.touched=-1;ui.msg_until=0;
    for(int j=0;j<20;j++){now_ms+=16;int32_t audio[512];engine_render(audio,256);ui_frame();}
    for(int j=0;j<4;j++)blit_hash[j]=0;
    ui.dirty=1;ui_frame();shot(name);printf("WIP,%s,view%d,part%d,page%d\n",name,ui.view,ui.part,cur_page());
}
static void drum(const char *name,int kit,int voice,int page)
{
    setup(V_PART,kit);ui.sel[kit]=(uint8_t)voice;build_pages();*page_of(V_PART)=(uint8_t)page;
    for(int step=0;step<16;step+=4)sm_set(cur_pat()->drum[kit].hit[voice],step);
    take(name);
}
static void bass(const char *name,int bank,int page)
{
    setup(V_PART,PART_303A);CHECK(engine_bass_select_preset(0,bank,0,&proj.sound)==0);
    int32_t audio[512];for(int j=0;j<1024&&engine_bass_preset_pending(0);j++)engine_render(audio,256);
    CHECK(engine_bass_bank(0)==bank);build_pages();*page_of(V_PART)=(uint8_t)page;take(name);
}
int main(void)
{
    drum("01-909-kick-main",PART_909,0,0);drum("02-909-kick-pitch",PART_909,0,1);
    drum("03-909-clap-two-controls",PART_909,6,0);drum("04-808-kick",PART_808,0,0);drum("05-808-cymbal",PART_808,8,0);
    bass("06-juno-oscillator",ORIGINAL_SOUND_JUNO,0);bass("07-juno-filter",ORIGINAL_SOUND_JUNO,1);
    bass("08-juno-envelope",ORIGINAL_SOUND_JUNO,2);bass("09-juno-lfo-chorus",ORIGINAL_SOUND_JUNO,3);
    bass("10-chicago-oscillator",ORIGINAL_SOUND_CHICAGO,0);bass("11-chicago-envelope",ORIGINAL_SOUND_CHICAGO,3);
    bass("12-miami-carrier",ORIGINAL_SOUND_MIAMI,0);bass("13-miami-wobble",ORIGINAL_SOUND_MIAMI,3);
    bass("14-drone",ORIGINAL_SOUND_DRONE,0);
    setup(V_HOME,PART_909);take("15-home");setup(V_FX,PART_909);*page_of(V_FX)=1;take("16-fx-delay");
    setup(V_MIX,PART_909);take("17-drum-mixer");setup(V_SEQ,PART_303A);take("18-bass-sequencer");
    printf("WIP_CAPTURE_PASS,18,%u\n",checks);return 0;
}
