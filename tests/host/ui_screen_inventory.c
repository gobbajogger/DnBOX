/* SPDX-License-Identifier: GPL-3.0-only */
/* Enumerate and render production pages, not a separate mock renderer.
 * Coverage: ordinary views, all selected parts/drum voices and emitted pages.
 * Popups, transient states and all synth engine selections need extra fixtures. */
#define SMC_UI_MAIN retained_fixture_main
#include "smc_ui_test.c"
int main(void)
{
    unsigned total=0;
    for(int view=0;view<NVIEWS;view++) for(int part=0;part<NPARTS;part++) {
        if(view==V_GLO) continue; /* enter GLOBAL through its real overlay route below */
        if(view==V_GEN && (part<PART_303A || part>PART_303B)) continue;
        int voices=(view==V_PART && part<NKIT)?NDRUM:1;
        for(int voice=0;voice<voices;voice++) {
            setup(view,part);
            if(part<NKIT) ui.sel[part]=(uint8_t)voice;
            build_pages(); int pages=pg.n;
            CHECK(pages>0 && pages<=32);
            for(int page=0;page<pages;page++) {
                *page_of(view)=(uint8_t)page; build_pages();
                memset(blit_hash,0,sizeof blit_hash); memset(cv_tile_valid,0,sizeof cv_tile_valid); ui_frame();
                char name[80];snprintf(name,sizeof name,"view-%02d-part-%02d-voice-%02d-page-%02d",view,part,voice,page);
                shot(name);
                printf("SCREEN {\"id\":\"%s\",\"view\":%d,\"part\":%d,\"voice\":%d,\"page\":%d,\"pages\":%d,\"width\":240,\"height\":240}\n",name,view,part,voice,page,pages);
                total++;
            }
        }
    }
    /* A bare V_GLO is not a reachable screen: GLOBAL requires O_LIST and its real rows. */
    setup(V_HOME,PART_303A);button_tap(B_GLO);
    CHECK(ui.view==V_GLO && ui.overlay==O_LIST);
    for(int section=-1;section<6;section++) {
        global_index();
        if(section>=0) {ui.list_sel=(uint8_t)section;button_tap(B_SEL);}
        CHECK(ui.overlay==O_LIST && ui.list_n>0);
        int rows=ui.list_n;
        for(int row=0;row<rows;row++) {
            ui.list_sel=(uint8_t)row;ui.list_top=(uint8_t)(row>5?row-5:0);
            memset(blit_hash,0,sizeof blit_hash);memset(cv_tile_valid,0,sizeof cv_tile_valid);ui_frame();
            char name[80];snprintf(name,sizeof name,"global-section-%d-row-%02d",section,row);shot(name);
            printf("SCREEN {\"id\":\"%s\",\"view\":%d,\"section\":%d,\"row\":%d,\"width\":240,\"height\":240}\n",name,V_GLO,section,row);
            total++;
        }
    }
    printf("INVENTORY %u ordinary/global states; other overlays/alternate engine coverage NOT_RUN\n",total);
    return 0;
}
