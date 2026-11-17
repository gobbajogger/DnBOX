/* SPDX-License-Identifier: GPL-3.0-only */
/* Semantic-caption geometry on real production frames reached by this walker.
 * Does not substitute for component/panel/glyph geometry or native fit. */
#define SMC_UI_MAIN retained_fixture_main
#include "../host/smc_ui_test.c"

static unsigned presence_checks,presence_missing;
static int seen_text(const char *text) {for(unsigned i=0;i<geo_nframe;i++)if(!strcmp(text,geo_frame[i].text))return 1;return 0;}
static void required_drum_content(const char *screen) {
 if(ui.view!=V_PART || ui.part>PART_808 || ui.overlay!=O_NONE || ui.help)return;
 int page=cur_page();
 for(int k=0;k<4;k++) {
  pref_t r=pg.r[page][k];if(r.kind==R_NONE)continue;
  const x0x_param_t *d=pref_desc(r);if(!d)continue;
  char caption[32],num[32],unit[8],value[40];unsigned j=0;
  for(;pref_name(r)[j]&&j<31;j++)caption[j]=pref_name(r)[j]>='a'&&pref_name(r)[j]<='z'?pref_name(r)[j]-32:pref_name(r)[j];caption[j]=0;
  if(!strcmp(d->name,"P.Dpth"))strcpy(caption,"DEPTH");
  pref_value_of(r,matrix_value(r),num,unit);snprintf(value,sizeof value,"%s%s",num,unit);
  unsigned glyph=0;for(unsigned z=0;z<presence_nglyph;z++)if(presence_glyph[z][0]==r.kind&&presence_glyph[z][1]==r.a&&presence_glyph[z][2]==r.b&&presence_glyph[z][3]==r.c)glyph++;
  presence_checks++;if(!glyph){presence_missing++;printf("MISSING_CONTENT screen=%s kind=glyph expected=[%s]\n",screen,caption);}
  const char *expected[2]={caption,value};
  for(int n=0;n<2;n++){presence_checks++;if(!seen_text(expected[n])){presence_missing++;printf("MISSING_CONTENT screen=%s kind=%s expected=[%s]\n",screen,n?"value+unit":"label",expected[n]);}}
 }
}

static unsigned list_presence_checks,synth_presence_checks;
static int seen_region(const char *text,int x0,int y0,int x1,int y1) {
 for(unsigned i=0;i<geo_nframe;i++){geo_label *e=&geo_frame[i];if(e->x>=x0&&e->x<x1&&e->y>=y0&&e->y<y1&&!strcmp(text,e->text))return 1;}return 0;
}
static void expect_region(const char *screen,const char *kind,const char *text,int x0,int y0,int x1,int y1) {
 presence_checks++;if(!seen_region(text,x0,y0,x1,y1)){presence_missing++;printf("MISSING_CONTENT screen=%s kind=%s expected=[%s] region=%d,%d..%d,%d\n",screen,kind,text,x0,y0,x1,y1);}
}
static void required_other_content(const char *screen) {
 if(ui.help)return;
 if(ui.overlay==O_LIST) {
  for(int row=ui.list_top;row<ui.list_n&&row<ui.list_top+6;row++) {
   pref_t r=list_rows[row];char label[64],num[64],unit[8];int y=HDR_H+(row-ui.list_top)*23;
   if(r.kind==R_HDR){hdr_name(r.a,label);expect_region(screen,"list-section",label,0,y,240,y+23);list_presence_checks++;continue;}
   snprintf(label,sizeof label,"%s",caps_of(pref_name(r)));expect_region(screen,"list-label",label,0,y,240,y+23);list_presence_checks++;
   if(r.kind==R_SECTION||r.kind==R_ACT||r.kind==R_BTNAME||r.kind==R_PARTON)continue;
   pref_value(r,num,unit);
   if(r.kind==R_BPRESET){const char *name=original_sound_name((unsigned)engine_bass_requested_bank(r.b),(unsigned)engine_bass_requested_preset(r.b));snprintf(num,sizeof num,"%s",caps_of(name?name:"USER"));}
   expect_region(screen,"list-value",num,0,y,240,y+23);list_presence_checks++;
   if(unit[0]){expect_region(screen,"list-unit",unit,0,y,240,y+23);list_presence_checks++;}
  }
 }
 if(ui.overlay==O_NONE&&ui.view==V_PART&&is_303()) {
  int page=cur_page(),row=page%4,three=matrix_three_rows(),height=three?66:49,step=three?67:50,y=HDR_H+4+row*step;
  for(int k=0;k<4;k++) {
   pref_t r=pg.r[page][k];if(!pref_desc(r))continue;char label[64],num[32],unit[8],joined[48];
   if(adsr_row(page)){snprintf(label,sizeof label,"K%d %c %d%%",UI_FIRST_KNOB+k,"ADSR"[k],(matrix_value(r)*100+63)/127);expect_region(screen,"synth-envelope",label,k*59,y,240,y+height);synth_presence_checks++;continue;}
   snprintf(label,sizeof label,"%s",matrix_name(r));expect_region(screen,"synth-label",label,k*59,y,(k+1)*59+4,y+height);synth_presence_checks++;
   if(r.kind==R_BPRESET)continue; /* wrapped preset names have their own tested formatter */
   pref_value_of(r,matrix_value(r),num,unit);snprintf(joined,sizeof joined,"%s%s",num,unit);
   int present=seen_region(joined,k*59,y,(k+1)*59+4,y+height);
   snprintf(joined,sizeof joined,"%s%s%s",num,unit[0]?" ":"",unit);
   present|=seen_region(joined,k*59,y,(k+1)*59+4,y+height);
   present|=seen_region(num,k*59,y,(k+1)*59+4,y+height)&&(!unit[0]||seen_region(unit,k*59,y,(k+1)*59+4,y+height));
   presence_checks++;synth_presence_checks++;
   if(!present){presence_missing++;printf("MISSING_CONTENT screen=%s kind=synth-value expected=[%s%s]\n",screen,num,unit);}
  }
 }
}

static unsigned strip_presence_checks;
static void required_strip_content(const char *screen) {
 if(ui.view==V_PART&&(is_drum()||is_303()))return;
 if(ui.view==V_SEQ&&is_303()&&ui.held_step>=0)return;
 int page=cur_page();
 for(int k=0;k<4;k++) {
  pref_t r=pg.r[page][k];if(!pref_desc(r))continue;
  char label[64],num[32],unit[8];snprintf(label,sizeof label,"%s",caps_of(pref_name(r)));
  expect_region(screen,"strip-label",label,k*60,KNOB_Y,(k+1)*60,240);strip_presence_checks++;
  int val=pref_get(r);if(lock_step()>=0){int lv=lock_of(r,lock_step());if(lv>=0)val=lv;}else if(has_motion(r)){int mv=motion_now(r);if(mv>=0)val=mv;}
  if(r.kind==R_BDIR)continue; /* native direction uses FWD/REV/PING/RND glyph */
  pref_value_of(r,val,num,unit);
  if(r.kind==R_ENG&&r.a==T_MST&&r.c==MST_DJF){uint8_t mode,cut;int res;filter_display(&mode,&cut,&res);pref_value_of(PR(R_ENG,T_MST,0,MST_MODE),mode,num,unit);}
  expect_region(screen,"strip-value",num,k*60,KNOB_Y,(k+1)*60,240);strip_presence_checks++;
  if(unit[0]){expect_region(screen,"strip-unit",unit,k*60,KNOB_Y,(k+1)*60,240);strip_presence_checks++;}
 }
}

static unsigned pixel_checks,pixel_missing,pixel_damaged;
static void required_drum_pixels(const char *screen) {
 if(ui.view!=V_PART||!is_drum()||ui.overlay!=O_NONE||ui.help)return;
 int page=cur_page(),y=26,cells=0;
 for(int p=0;p<page;p++){int n=0;for(int k=0;k<4;k++)if(pg.r[p][k].kind!=R_NONE)n++;y+=n>1?42:32;}
 for(int k=0;k<4;k++)if(pg.r[page][k].kind!=R_NONE)cells++;
 int height=cells==1?56:72;
 for(int k=0;k<4;k++) {
  pref_t r=pg.r[page][k];if(r.kind==R_NONE)continue;
  uint32_t owner=1u+((uint32_t)r.kind<<24)+((uint32_t)r.a<<16)+((uint32_t)r.b<<8)+r.c;
  unsigned pixels=0,missing=0,outside=0;
  int x0=cells==1?6:6+228*k/cells,x1=cells==1?234:6+228*(k+1)/cells;
  for(int sy=0;sy<240;sy++)for(int sx=0;sx<240;sx++)if(pixel_frame_owner[sy*240+sx]==owner){
   pixels++;if(fb[sy*240+sx]!=pixel_frame_color[sy*240+sx])missing++;
   if(sx<x0||sx>=x1||sy<y||sy>=y+height)outside++;
  }
  pixel_checks++;
  if(!pixels){pixel_missing++;printf("GLYPH_PIXEL_MISSING screen=%s ref=%u:%u:%u:%u\n",screen,r.kind,r.a,r.b,r.c);}
  if(missing||outside){pixel_damaged++;printf("GLYPH_PIXEL_DAMAGED screen=%s ref=%u:%u:%u:%u pixels=%u overpaint=%u outside=%u region=%d,%d..%d,%d\n",screen,r.kind,r.a,r.b,r.c,pixels,missing,outside,x0,y,x1,y+height);}
 }
}
static void geo_audit(const char *screen) {
 required_drum_pixels(screen);
 required_strip_content(screen);
 required_other_content(screen);
 required_drum_content(screen);
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
                geo_nframe=0;memset(pixel_frame_owner,0,sizeof pixel_frame_owner);presence_nglyph=0;geo_enabled=1;memset(blit_hash,0,sizeof blit_hash); memset(cv_tile_valid,0,sizeof cv_tile_valid); ui_frame();
                char name[80];snprintf(name,sizeof name,"view-%02d-part-%02d-voice-%02d-page-%02d",view,part,voice,page);
                shot(name);geo_audit(name);
                printf("SCREEN {\"id\":\"%s\",\"view\":%d,\"part\":%d,\"voice\":%d,\"page\":%d,\"pages\":%d,\"width\":240,\"height\":240}\n",name,view,part,voice,page,pages);
                total++;
                for(int edge=0;edge<2;edge++) {
                    int old[4];pref_t refs[4];
                    for(int k=0;k<4;k++){refs[k]=pg.r[page][k];old[k]=pref_get(refs[k]);const x0x_param_t *d=pref_desc(refs[k]);if(refs[k].kind==R_ENG && d)pref_set(refs[k],edge?d->max:0);}
                    geo_nframe=0;memset(pixel_frame_owner,0,sizeof pixel_frame_owner);presence_nglyph=0;geo_enabled=1;memset(blit_hash,0,sizeof blit_hash);memset(cv_tile_valid,0,sizeof cv_tile_valid);ui_frame();
                    snprintf(name,sizeof name,"edge-%d-view-%02d-part-%02d-voice-%02d-page-%02d",edge,view,part,voice,page);shot(name);geo_audit(name);total++;
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
            geo_nframe=0;memset(pixel_frame_owner,0,sizeof pixel_frame_owner);presence_nglyph=0;geo_enabled=1;memset(blit_hash,0,sizeof blit_hash);memset(cv_tile_valid,0,sizeof cv_tile_valid);ui_frame();
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
    geo_nframe=0;memset(pixel_frame_owner,0,sizeof pixel_frame_owner);presence_nglyph=0;geo_enabled=1;memset(blit_hash,0,sizeof blit_hash);memset(cv_tile_valid,0,sizeof cv_tile_valid);ui_frame();
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
       *page_of(V_PART)=(uint8_t)page;build_pages();geo_nframe=0;memset(pixel_frame_owner,0,sizeof pixel_frame_owner);presence_nglyph=0;geo_enabled=1;
       memset(blit_hash,0,sizeof blit_hash);memset(cv_tile_valid,0,sizeof cv_tile_valid);ui_frame();
       char name[80];snprintf(name,sizeof name,"engine-%u-part-%d-page-%d",kind,part,page);shot(name);geo_audit(name);total++;
      }
    }

    for(int view=0;view<NVIEWS;view++)for(int part=0;part<NPARTS;part++)for(int button=0;button<NB;button++) {
      if(view==V_GLO || (view==V_GEN && (part<PART_303A||part>PART_303B)))continue;
      setup(view,part);held_btn=ui.btn=1u<<button;ui.help=button+1;
      geo_nframe=0;memset(pixel_frame_owner,0,sizeof pixel_frame_owner);presence_nglyph=0;geo_enabled=1;memset(blit_hash,0,sizeof blit_hash);memset(cv_tile_valid,0,sizeof cv_tile_valid);ui_frame();
      CHECK(ui.help==button+1);char name[80];snprintf(name,sizeof name,"help-view-%d-part-%d-button-%d",view,part,button);shot(name);geo_audit(name);total++;
    }
    for(int modal=0;modal<4;modal++) {
      setup(V_HOME,PART_909);
      if(modal==0)ask(ACT_RESET,0,"RESET PROJECT?","SAMPLES ARE KEPT");
      if(modal==1)say("SAVED",0);
      if(modal==2)say("NEXT: PAT 16",0);
      if(modal==3)run_action(ACT_ABOUT,0);
      geo_nframe=0;memset(pixel_frame_owner,0,sizeof pixel_frame_owner);presence_nglyph=0;geo_enabled=1;memset(blit_hash,0,sizeof blit_hash);memset(cv_tile_valid,0,sizeof cv_tile_valid);ui_frame();
      char name[80];snprintf(name,sizeof name,"modal-%d",modal);shot(name);geo_audit(name);total++;
    }
    printf("INVENTORY %u ordinary/global/list/engine/help/modal states\n",total);


    printf("GEOMETRY findings=%u screens=%u\n",geo_findings,total);
    printf("STRIP_PRESENCE checks=%u\n",strip_presence_checks);printf("FAMILY_PRESENCE list=%u synth=%u\n",list_presence_checks,synth_presence_checks);printf("PRESENCE checks=%u missing=%u\n",presence_checks,presence_missing);printf("GLYPH_PIXELS checks=%u missing=%u damaged=%u\n",pixel_checks,pixel_missing,pixel_damaged);return geo_findings||presence_missing||pixel_missing||pixel_damaged?1:0;
}
