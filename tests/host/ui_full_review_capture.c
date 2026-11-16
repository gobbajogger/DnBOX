/* SPDX-License-Identifier: GPL-3.0-only */
/* Review-only exhaustive constructed pages through the actual firmware renderer.
 * Hardware/clip test substitutes are inherited and are not device evidence. */
#define SMC_UI_MAIN unused_smc_ui_main
#include "smc_ui_test.c"

static void review_render(const char *name,int view,int part,int bank,int voice,int page)
{
    draw_main();draw_knobs();draw_footer();draw_header();shot(name);
    printf("SCREEN,%s,%d,%d,%d,%d,%d\n",name,view,part,bank,voice,page);
}
int main(void)
{
    unsigned captured=0,unavailable=0;
    const int banks[]={ORIGINAL_SOUND_MIAMI,ORIGINAL_SOUND_CHICAGO,ORIGINAL_SOUND_JUNO,ORIGINAL_SOUND_DRONE};
    for(int view=0;view<NVIEWS;view++)for(int part=0;part<NPARTS;part++) {
        int bass=part==PART_303A||part==PART_303B;
        int nb=bass?4:1;
        int nv=(part<2&&(view==V_PART||view==V_SEQ||view==V_MIX))?NDRUM:1;
        for(int bi=0;bi<nb;bi++)for(int voice=0;voice<nv;voice++) {
            setup(view,part);
            int bank=bass?banks[bi]:-1;
            if(bass) {
                int b=part-PART_303A;
                int result=engine_bass_select_preset(b,bank,0,&proj.sound);
                if(result!=0) {printf("UNAVAILABLE,%d,%d,%d,%d\n",view,part,bank,result);unavailable++;continue;}
                int32_t audio[512];
                for(int j=0;j<1024&&engine_bass_preset_pending(b);j++)engine_render(audio,256);
                CHECK(!engine_bass_preset_pending(b));CHECK(engine_bass_bank(b)==bank);
            }
            ui.sel[part<2?part:0]=(uint8_t)voice;
            if(part<2)for(int step=0;step<16;step+=4)sm_set(cur_pat()->drum[part].hit[voice],step);
            ui.part=(uint8_t)part;set_view(view);build_pages();
            int pages=pg.n;
            if(pages<1)pages=1;
            for(int page=0;page<pages;page++) {
                *page_of(view)=(uint8_t)page;build_pages();ui.touched=-1;ui.msg_until=0;
                for(int i=0;i<12;i++){now_ms+=16;ui_frame();}
                char name[128];snprintf(name,sizeof name,"view%02d-part%d-bank%d-voice%02d-page%02d",view,part,bank,voice,page);
                review_render(name,view,part,bank,voice,page);captured++;
                /* Equivalent maximum raw control state exposes value clipping.
                 * Only actual engine parameters change; no actions are invoked. */
                int saved[4];uint8_t changed[4]={0};
                for(int k=0;k<4;k++) {
                    pref_t r=pg.r[page][k];const x0x_param_t *d=pref_desc(r);
                    if(r.kind==R_ENG&&d) {saved[k]=pref_get(r);changed[k]=1;pref_set(r,d->max);}
                }
                ui_ease_reset();ui.dirty=1;ui_frame();
                strncat(name,"-max",sizeof name-strlen(name)-1);
                review_render(name,view,part,bank,voice,page);captured++;
                for(int k=0;k<4;k++)if(changed[k])pref_set(pg.r[page][k],saved[k]);
                ui_ease_reset();
            }
        }
    }
    /* Real modal and global-section states, without executing destructive actions. */
    setup(V_HOME,PART_909);ask(ACT_CLEAR_PAT,0,"CLEAR THIS PATTERN?","ALL FIVE PARTS");
    review_render("ask-clear-pattern",V_HOME,0,-1,0,0);captured++;
    setup(V_HOME,PART_909);ask(ACT_RESET,0,"RESET PROJECT?","SAMPLES ARE KEPT");
    review_render("ask-reset-project",V_HOME,0,-1,0,0);captured++;
    /* All real global sections, every selected row and scrolled endpoint. */
    setup(V_GLO,PART_909);global_index();review_render("global-index",V_GLO,0,-1,0,0);captured++;
    for(int section=0;section<6;section++) {
        setup(V_GLO,PART_909);global_section(section);
        for(int row=0;row<ui.list_n;row++) {
            ui.list_sel=(uint8_t)row;
            if(row>3)ui.list_top=(uint8_t)(row-3);
            char name[80];snprintf(name,sizeof name,"global-section%d-row%02d",section,row);
            review_render(name,V_GLO,0,-1,0,row);captured++;
        }
    }
    /* Every SMC overlay view/part; drawing is the real SMC renderer, while
     * hardware and clip-grid substitutions remain explicit limitations. */
    for(int view=0;view<NVIEWS;view++)for(int part=0;part<NPARTS;part++) {
        setup(view,part);smc.view=(uint8_t)view;smc.part=smc.mixsel=(uint8_t)part;
        fader(0,0);smc_ui_draw();
        char name[80];snprintf(name,sizeof name,"smc-view%02d-part%d",view,part);shot(name);captured++;
    }
    printf("COVERAGE,%u,%u,%u\n",captured,unavailable,checks);
    return 0;
}
