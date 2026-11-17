/* SPDX-License-Identifier: GPL-3.0-only */
/* Production host frames, not visual approval or golden-reference generation. */
#define SMC_UI_MAIN retained_fixture_main
#include "smc_ui_test.c"
static void walkshot(const char *name) {
 const char *family="ordinary";
 if(!strncmp(name,"global-",7))family="global";
 if(!strncmp(name,"list-",5))family="list";
 if(!strncmp(name,"engine-",7))family="engine";
 if(!strncmp(name,"help-",5))family="help";
 if(!strncmp(name,"modal-",6))family="modal";
 shot(name);
 printf("WALK {\"kind\":\"screen\",\"id\":\"%s\",\"family\":\"%s\"}\n",name,family);
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
                memset(blit_hash,0,sizeof blit_hash); memset(cv_tile_valid,0,sizeof cv_tile_valid); ui_frame();
                char name[80];snprintf(name,sizeof name,"view-%02d-part-%02d-voice-%02d-page-%02d",view,part,voice,page);
                walkshot(name);
                printf("SCREEN {\"id\":\"%s\",\"view\":%d,\"part\":%d,\"voice\":%d,\"page\":%d,\"pages\":%d,\"width\":240,\"height\":240}\n",name,view,part,voice,page,pages);
                total++;
                for(int edge=0;edge<2;edge++) {
                    int old[4];pref_t refs[4];
                    for(int k=0;k<4;k++){refs[k]=pg.r[page][k];old[k]=pref_get(refs[k]);const x0x_param_t *d=pref_desc(refs[k]);if(refs[k].kind==R_ENG && d)pref_set(refs[k],edge?d->max:0);}
                    memset(blit_hash,0,sizeof blit_hash);memset(cv_tile_valid,0,sizeof cv_tile_valid);ui_frame();
                    snprintf(name,sizeof name,"edge-%d-view-%02d-part-%02d-voice-%02d-page-%02d",edge,view,part,voice,page);walkshot(name);total++;
                    for(int k=0;k<4;k++)if(refs[k].kind==R_ENG)pref_set(refs[k],old[k]);
                }
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
            char name[80];snprintf(name,sizeof name,"global-section-%d-row-%02d",section,row);walkshot(name);
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
    memset(blit_hash,0,sizeof blit_hash);memset(cv_tile_valid,0,sizeof cv_tile_valid);ui_frame();
    char name[96];snprintf(name,sizeof name,"list-view-%02d-part-%02d-voice-%02d-row-%02d",view,part,voice,row);walkshot(name);
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
       *page_of(V_PART)=(uint8_t)page;build_pages();
       memset(blit_hash,0,sizeof blit_hash);memset(cv_tile_valid,0,sizeof cv_tile_valid);ui_frame();
       char name[80];snprintf(name,sizeof name,"engine-%u-part-%d-page-%d",kind,part,page);walkshot(name);total++;
      }
    }

    /* Owner-review routes omitted by the ordinary default-bank traversal. */
    for(unsigned kind=0;kind<4;kind++)for(int part=PART_303A;part<=PART_303B;part++) {
      setup(V_SEQ,part);
      CHECK(engine_bass_select_preset(part-PART_303A,kind,0,&proj.sound)==0);
      for(int block=0;block<12;block++){int32_t audio[512];engine_render(audio,256);}
      CHECK(!engine_bass_preset_pending(part-PART_303A));build_pages();int pages=pg.n;
      CHECK(pages==3);
      for(int page=0;page<pages;page++) {
       *page_of(V_SEQ)=(uint8_t)page;build_pages();
       memset(blit_hash,0,sizeof blit_hash);memset(cv_tile_valid,0,sizeof cv_tile_valid);ui_frame();
       char name[80];snprintf(name,sizeof name,"engine-seq-%u-part-%d-page-%d",kind,part,page);walkshot(name);total++;
      }
    }
    setup(V_HOME,PART_909);button_tap(B_PLAY);{int32_t audio[512];engine_render(audio,256);}CHECK(seq.playing);
    {
     memset(blit_hash,0,sizeof blit_hash);memset(cv_tile_valid,0,sizeof cv_tile_valid);ui_frame();
     const char *name="home-playing";walkshot(name);total++;
    }
    button_tap(B_PLAY);{int32_t audio[512];engine_render(audio,256);}CHECK(!seq.playing);
    setup(V_HOME,PART_909);button_tap(B_GLO);CHECK(ui.overlay==O_LIST);
    ui.list_sel=4;button_tap(B_SEL);CHECK(ui.glo_section==4);
    int clear_row=-1;
    for(int row=0;row<ui.list_n;row++)if(list_rows[row].kind==R_ACT && list_rows[row].a==ACT_CLEAR_PAT)clear_row=row;
    CHECK(clear_row>=0);ui.list_sel=(uint8_t)clear_row;button_tap(B_SEL);CHECK(ui.overlay==O_ASK);
    {
     memset(blit_hash,0,sizeof blit_hash);memset(cv_tile_valid,0,sizeof cv_tile_valid);ui_frame();
     const char *name="modal-clear-pattern";walkshot(name);total++;
    }

    for(int view=0;view<NVIEWS;view++)for(int part=0;part<NPARTS;part++)for(int button=0;button<NB;button++) {
      if(view==V_GLO || (view==V_GEN && (part<PART_303A||part>PART_303B)))continue;
      setup(view,part);held_btn=ui.btn=1u<<button;ui.help=button+1;
      memset(blit_hash,0,sizeof blit_hash);memset(cv_tile_valid,0,sizeof cv_tile_valid);ui_frame();
      CHECK(ui.help==button+1);char name[80];snprintf(name,sizeof name,"help-view-%d-part-%d-button-%d",view,part,button);walkshot(name);total++;
    }
    for(int modal=0;modal<4;modal++) {
      setup(V_HOME,PART_909);
      if(modal==0)ask(ACT_RESET,0,"RESET PROJECT?","SAMPLES ARE KEPT");
      if(modal==1)say("SAVED",0);
      if(modal==2)say("NEXT: PAT 16",0);
      if(modal==3)run_action(ACT_ABOUT,0);
      memset(blit_hash,0,sizeof blit_hash);memset(cv_tile_valid,0,sizeof cv_tile_valid);ui_frame();
      char name[80];snprintf(name,sizeof name,"modal-%d",modal);walkshot(name);total++;
    }
    printf("WALK {\"kind\":\"complete\",\"screens\":%u}\n",total);
    return 0;
}
