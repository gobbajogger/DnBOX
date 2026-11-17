/* SPDX-License-Identifier: GPL-3.0-only */
/* Semantic-caption geometry on real production frames reached by this walker.
 * Does not substitute for component/panel/glyph geometry or native fit. */
#define SMC_UI_MAIN retained_fixture_main
#include "../host/smc_ui_test.c"

static void geo_audit(const char *screen) {
 geo_enabled=0;
 for(unsigned i=0;i<geo_nframe;i++) {
  geo_label e=geo_frame[i];unsigned expected=0,missing=0;
  cv_begin(240,138,0);cv_text(10,0,e.f,e.text,65535);
  for(int y=0;y<e.f->h;y++)for(int x=0;x<240;x++)if(cv_px[y*240+x]==65535) {
   expected++;int sx=e.x+x-10,sy=e.y+y;
   if(getenv("GEOMETRY_MUTATE") && sx>=0&&sx<240&&sy>=0&&sy<240)fb[sy*240+sx]^=65535;
   if(sx<0||sx>=240||sy<0||sy>=240||fb[sy*240+sx]!=swap16(e.col))missing++;
  }
  if(missing){geo_findings++;printf("GEOMETRY screen=%s text=[%s] at=%d,%d missing=%u/%u\n",screen,e.text,e.x,e.y,missing,expected);}
 }
}

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
                geo_nframe=0;geo_enabled=1;memset(blit_hash,0,sizeof blit_hash); memset(cv_tile_valid,0,sizeof cv_tile_valid); ui_frame();
                char name[80];snprintf(name,sizeof name,"view-%02d-part-%02d-voice-%02d-page-%02d",view,part,voice,page);
                shot(name);geo_audit(name);
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
            geo_nframe=0;geo_enabled=1;memset(blit_hash,0,sizeof blit_hash);memset(cv_tile_valid,0,sizeof cv_tile_valid);ui_frame();
            char name[80];snprintf(name,sizeof name,"global-section-%d-row-%02d",section,row);shot(name);geo_audit(name);
            printf("SCREEN {\"id\":\"%s\",\"view\":%d,\"section\":%d,\"row\":%d,\"width\":240,\"height\":240}\n",name,V_GLO,section,row);
            total++;
        }
    }
 unsigned lists=0;
 for(int view=0;view<NVIEWS;view++)for(int part=0;part<NPARTS;part++) {
  if(view==V_GLO)continue;
  if(view==V_GEN && (part<PART_303A || part>PART_303B))continue;
  int voices=(view==V_PART && part<NKIT)?NDRUM:1;
  for(int voice=0;voice<voices;voice++) {
   setup(view,part);if(part<NKIT)ui.sel[part]=(uint8_t)voice;
   build_pages();button_tap(B_SEL);if(ui.overlay!=O_LIST)continue;
   CHECK(ui.list_n>0 && ui.list_n<=LIST_MAX);lists++;
   for(int back=0;back<LIST_MAX;back++){int before=ui.list_sel;list_move(-1);if(ui.list_sel==before)break;}
   int expected=0,first=-1;uint64_t seen=0;
   for(int r=0;r<ui.list_n;r++)if(!list_skip(r)){expected++;if(first<0)first=r;}
   CHECK(ui.list_sel==first);int captured=0;
   for(int count=0;count<LIST_MAX;count++) {
    int row=ui.list_sel;CHECK(row<ui.list_n && !list_skip(row));
    CHECK(!(seen&(UINT64_C(1)<<row)));seen|=UINT64_C(1)<<row;captured++;
    geo_nframe=0;geo_enabled=1;memset(blit_hash,0,sizeof blit_hash);memset(cv_tile_valid,0,sizeof cv_tile_valid);ui_frame();
    char name[96];snprintf(name,sizeof name,"list-view-%02d-part-%02d-voice-%02d-row-%02d",view,part,voice,row);shot(name);geo_audit(name);
    printf("SCREEN {\"id\":\"%s\",\"view\":%d,\"part\":%d,\"voice\":%d,\"row\":%d,\"rows\":%d,\"width\":240,\"height\":240}\n",name,view,part,voice,row,ui.list_n);
    total++;list_move(1);if(ui.list_sel==row)break;
   }
   CHECK(captured==expected);
   printf("LIST_COVERAGE view=%d part=%d voice=%d first=%d selectable=%d captured=%d unique=%d\n",view,part,voice,first,expected,captured,captured);
   button_tap(B_HOME);CHECK(ui.overlay==O_NONE);
  }
 }

    for(unsigned kind=0;kind<4;kind++)for(int part=PART_303A;part<=PART_303B;part++) {
      setup(V_PART,part);
      CHECK(engine_bass_select_preset(part-PART_303A,kind,0,&proj.sound)==0);
      for(int block=0;block<12;block++){int32_t audio[512];engine_render(audio,256);}
      CHECK(!engine_bass_preset_pending(part-PART_303A));build_pages();int pages=pg.n;
      for(int page=0;page<pages;page++) {
       *page_of(V_PART)=(uint8_t)page;build_pages();geo_nframe=0;geo_enabled=1;
       memset(blit_hash,0,sizeof blit_hash);memset(cv_tile_valid,0,sizeof cv_tile_valid);ui_frame();
       char name[80];snprintf(name,sizeof name,"engine-%u-part-%d-page-%d",kind,part,page);shot(name);geo_audit(name);total++;
      }
    }

    for(int view=0;view<NVIEWS;view++)for(int part=0;part<NPARTS;part++)for(int button=0;button<NB;button++) {
      if(view==V_GLO || (view==V_GEN && (part<PART_303A||part>PART_303B)))continue;
      setup(view,part);held_btn=ui.btn=1u<<button;ui.help=button+1;
      geo_nframe=0;geo_enabled=1;memset(blit_hash,0,sizeof blit_hash);memset(cv_tile_valid,0,sizeof cv_tile_valid);ui_frame();
      CHECK(ui.help==button+1);char name[80];snprintf(name,sizeof name,"help-view-%d-part-%d-button-%d",view,part,button);shot(name);geo_audit(name);total++;
    }
    for(int modal=0;modal<4;modal++) {
      setup(V_HOME,PART_909);
      if(modal==0)ask(ACT_RESET,0,"RESET PROJECT?","SAMPLES ARE KEPT");
      if(modal==1)say("SAVED",0);
      if(modal==2)say("NEXT: PAT 16",0);
      if(modal==3)run_action(ACT_ABOUT,0);
      geo_nframe=0;geo_enabled=1;memset(blit_hash,0,sizeof blit_hash);memset(cv_tile_valid,0,sizeof cv_tile_valid);ui_frame();
      char name[80];snprintf(name,sizeof name,"modal-%d",modal);shot(name);geo_audit(name);total++;
    }
    printf("INVENTORY %u ordinary/global/list/engine/help/modal states\n",total);


    printf("GEOMETRY findings=%u screens=%u\n",geo_findings,total);
    return geo_findings?1:0;
}
