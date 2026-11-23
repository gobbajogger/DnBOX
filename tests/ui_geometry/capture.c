#include <ctype.h>
/* SPDX-License-Identifier: GPL-3.0-only */
/* Semantic-caption geometry on real production frames reached by this walker.
 * Does not substitute for component/panel/glyph geometry or native fit. */
#define SMC_UI_MAIN retained_fixture_main
#include "../host/smc_ui_test.c"

/* Same frozen identity contract as the presence-only walker. */
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

static unsigned presence_checks,presence_missing;
/* Independent retained ID of the 909 Kick main-AMP decay (not production drum_amp_ref()). */
#define KICK_AMP(r) ((r).kind==R_ENG&&(r).a==T_909&&(r).b==DR_BD&&(r).c==2)
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
  /* 837530d99: the 909 Kick AMP bank is one "DECAY <value>" caption over the
   * lower LEVEL/TIME graph, and its KIT bank names the velocity depth. */
  if(ui.part!=PART_808 && ui.sel[ui.part]==DR_BD){
   if(KICK_AMP(r)){
    char physical[48];snprintf(physical,sizeof physical,"DECAY %s",value);  /* guideline: no K-number prefix */
    presence_checks++;if(!seen_text(physical)){presence_missing++;printf("MISSING_CONTENT screen=%s kind=kick-decay expected=[%s]\n",screen,physical);}
    continue;
   }
   if(pg.group[page]==GR_KIT && r.kind==R_ENG && r.b==NDRUM)strcpy(caption,"VELOCITY DEPTH");
  }
  if(ui.part==PART_808 && ui.sel[PART_808]==D8_CP){
   if(page==0 && (k==1||k==3)){
    /* one readout per stage (FINAL-REFERENCES): the physical 99% duration, lettered only in the
     * shared MARACAS AD row; the knob % only where the owner has no curve */
    int ma=matrix_value(PR(R_ENG,T_808,D8_CP,4))!=0;
    const char *title=ma?"MARACAS AD":k==1?"MARACAS ATTACK":"CLAP DECAY";
    drum_curve_t c;double ms=-1;
    if(k==1){float m;engine_param_si(T_808,D8_CP,3,matrix_value(r),&m);ms=m;}
    else if(ma){if(engine_drum808_param_curve_preview(D8_CP,2,1,matrix_value(r),&c)==D8S_MA+1)ms=c.fall*4.60517f;}
    else ms=350.*pow(8.,matrix_value(r)/127.)*.0385*log(100.);
    char physical[40],t[24];
    if(ms<0)snprintf(t,sizeof t,"%d%%",(matrix_value(r)*100+63)/127);
    else{char *q=t;q=put_d1(q,(float)ms,0);put_s(q,"ms");}
    if(ma)snprintf(physical,sizeof physical,"%c %s",k==1?'A':'D',t);else snprintf(physical,sizeof physical,"%s",t);
    const char *required[2]={title,physical};for(int n=0;n<2;n++){presence_checks++;if(!seen_text(required[n])){presence_missing++;printf("MISSING_CONTENT screen=%s kind=bound-stage expected=[%s]\n",screen,required[n]);}}
    continue;
   }
   if(page==0)snprintf(caption,sizeof caption,"%s",k==0?"TUNE":"TYPE");
   else strcpy(caption,"KIT ACCENT");
  }
  if(ui.part==PART_909 && ui.sel[PART_909]==DR_BD) {
   /* Approved 909 Kick layout (837530d9): the AMP bank is one physical-knob
    * caption with its value, and the kick KIT row names velocity depth. */
   if(drum_amp_ref(r)) {
    char joined[64];snprintf(joined,sizeof joined,"DECAY %s",value);
    presence_checks++;if(!seen_text(joined)){presence_missing++;printf("MISSING_CONTENT screen=%s kind=kick-amp-caption expected=[%s]\n",screen,joined);}
    continue;
   }
   if(r.kind==R_ENG && r.b==NDRUM && k==0)strcpy(caption,"VELOCITY DEPTH");
  }
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
/* A selected envelope row draws the stage caption "A " and its value as separate runs;
 * any other row keeps the joined "A 50" caption (no knob-number prefix). */
static void expect_envelope(const char *screen,const char *kind,const char *label,int x0,int y0,int x1,int y1) {
 char prefix[64];const char *v=label;int spaces=0;
 while(*v&&spaces<1){if(*v==' ')spaces++;v++;}
 snprintf(prefix,sizeof prefix,"%.*s",(int)(v-label),label);
 presence_checks++;
 if(seen_region(label,x0,y0,x1,y1)||(*v&&seen_region(prefix,x0,y0,x1,y1)&&seen_region(v,x0,y0,x1,y1)))return;
 presence_missing++;printf("MISSING_CONTENT screen=%s kind=%s expected=[%s] region=%d,%d..%d,%d\n",screen,kind,label,x0,y0,x1,y1);
}
static void required_other_content(const char *screen) {
 if(ui.help)return;
 if(ui.overlay==O_LIST) {
  int visible=ui.view==V_PART && (is_drum() || is_303()) ? 9 : 6;
  int pitch=ui.list_n && ui.list_n<visible ? visible*23/ui.list_n : 23;
  int extra=visible==6?(pitch-23)/2:0;
  for(int row=ui.list_top;row<ui.list_n&&row<ui.list_top+visible;row++) {
   pref_t r=list_rows[row];char label[64],num[64],unit[8];int y=HDR_H+(row-ui.list_top)*pitch+(pitch-23-extra)/2;
   if(r.kind==R_HDR){hdr_name(r.a,label);expect_region(screen,"list-section",label,0,y,240,y+23+extra);list_presence_checks++;continue;}
   list_naming=1;snprintf(label,sizeof label,"%s",caps_of(pref_name(r)));list_naming=0;expect_region(screen,"list-label",label,0,y,240,y+23+extra);list_presence_checks++;
   if(r.kind==R_SECTION||r.kind==R_ACT||r.kind==R_BTNAME||r.kind==R_PARTON)continue;
   pref_value(r,num,unit);
   if(visible==9 && r.kind==R_ENG && pref_desc(r) && pref_desc(r)->names)
    for(char *c=num;*c;c++)if(*c>='a' && *c<='z')*c=(char)(*c-'a'+'A');
   if(r.kind==R_BPRESET){const char *name=original_sound_name((unsigned)engine_bass_requested_bank(r.b),(unsigned)engine_bass_requested_preset(r.b));snprintf(num,sizeof num,"%s",caps_of(name?name:"USER"));}
   expect_region(screen,"list-value",num,0,y,240,y+23+extra);list_presence_checks++;
   if(unit[0]){expect_region(screen,"list-unit",unit,0,y,240,y+23+extra);list_presence_checks++;}
  }
 }
 if(ui.overlay==O_NONE&&ui.view==V_PART&&is_303()) {
  int page=cur_page(), three=matrix_three_rows(), rows=three?3:4, row=page%4;
  int height=three?66:49,step=three?67:50;
  if(pg.n >= rows && pg.n-page+row < rows) row=rows-(pg.n-page);
  int y=HDR_H+4+row*step;
#if !X0X_SYNTHONE_PATCH
  if(engine_bass_bank(bidx())==ORIGINAL_SOUND_SYNTHONE && pg.r[page][0].kind==R_BENGINE) {
   CHECK(pg.n==2);
   const char *labels[]={BOARD_SMK37?"K5  ENGINE":"K1  ENGINE",
                         BOARD_SMK37?"K6  PRESET":"K2  PRESET","STATUS"};
   const char *values[]={"SYNTH ONE","INIT","READY"};
   for(int k=0;k<3;k++){
    int top=HDR_H+4+k*67;
    expect_region(screen,"stock-init-label",labels[k],6,top,100,top+26);
    expect_region(screen,"stock-init-value",values[k],32,top+33,231,top+66);
    synth_presence_checks+=2;
   }
   return;
  }
#endif
  int width=!X0X_ORIGINAL_SYNTH || pg.r[page][3].kind!=R_NONE ? 59 :
            pg.r[page][2].kind!=R_NONE ? 78 : pg.r[page][1].kind!=R_NONE ? 118 : 232;
  for(int k=0;k<4;k++) {
   pref_t r=pg.r[page][k];if(!pref_desc(r))continue;char label[64],num[32],unit[8],joined[48];
#if X0X_SYNTHONE_PATCH
   if(r.kind==R_S1PATCH && r.c>=5 && r.c<=12){
    int stage=(r.c-5)%4;
    CHECK(stage==k);
    pref_value_of(r,matrix_value(r),num,unit);
    snprintf(label,sizeof label,"%c %s%s","ADSR"[stage],num,
             !strcmp(unit,"%") ? "%" : "");
    expect_envelope(screen,"synthone-envelope",label,k*width,y,(k+1)*width+4,y+height);
    synth_presence_checks++;
    if(unit[0] && strcmp(unit,"%")){
     expect_region(screen,"synthone-envelope-unit",unit,k*width,y,(k+1)*width+4,y+height);
     synth_presence_checks++;
    }
    continue;
   }
#endif
   if(adsr_row(page)){
    /* Independent owner/control IDs, not the renderer's partial-stage slots.
     * Captions must retain physical knob identity as well as actual stage. */
    int bank=engine_bass_sound_bank(&proj.sound,bidx());
    int first=bank==ORIGINAL_SOUND_CHICAGO?7:bank==ORIGINAL_SOUND_JUNO?5:-1;
    int stage=(int)r.c-first;
    if(first<0 || stage<0 || stage>3){presence_missing++;printf("MISSING_CONTENT screen=%s kind=synth-envelope invalid-owner-or-stage\n",screen);continue;}
    if(bank==ORIGINAL_SOUND_JUNO) {
     pref_value_of(r,matrix_value(r),num,unit);
     snprintf(label,sizeof label,"%s%s",num,!strcmp(unit,"%") ? "%" : "");
     if(strcmp(unit,"%")) {
      expect_region(screen,"synth-envelope-unit",unit,k*width,y,(k+1)*width+4,y+height);
      synth_presence_checks++;
     }
    } else snprintf(label,sizeof label,"%c %d%%","ADSR"[stage],(matrix_value(r)*100+63)/127);
    expect_envelope(screen,"synth-envelope",label,k*width,y,(k+1)*width+4,y+height);synth_presence_checks++;continue;
   }
   snprintf(label,sizeof label,"%s",matrix_name(r));expect_region(screen,"synth-label",label,k*width,y,(k+1)*width+4,y+height);synth_presence_checks++;
   if(r.kind==R_BPRESET)continue; /* wrapped preset names have their own tested formatter */
   pref_value_of(r,matrix_value(r),num,unit);snprintf(joined,sizeof joined,"%s%s",num,unit);
   int present=seen_region(joined,k*width,y,(k+1)*width+4,y+height);
   snprintf(joined,sizeof joined,"%s%s%s",num,unit[0]?" ":"",unit);
   present|=seen_region(joined,k*width,y,(k+1)*width+4,y+height);
   present|=seen_region(num,k*width,y,(k+1)*width+4,y+height)&&(!unit[0]||seen_region(unit,k*width,y,(k+1)*width+4,y+height));
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
  unsigned pixels=0,missing=0,outside=0;int bx0=240,by0=240,bx1=-1,by1=-1;
  int x0=cells==1?6:6+228*k/cells,x1=cells==1?234:6+228*(k+1)/cells;
  if(ui.part!=PART_808&&ui.sel[ui.part]==DR_BD&&KICK_AMP(r)){x0=3;x1=237;height=240-y;} /* 837530d99 lower LEVEL/TIME panel */
  if(ui.part==PART_808&&ui.sel[PART_808]==D8_CP){
   if(page==1){x0=103;x1=140;y=177;height=20;}
   else if(k==0||k==2){x0=k?128:60;x1=k?183:108;y=50;height=26;}
   else {x0=11;x1=230;if(matrix_value(PR(R_ENG,T_808,D8_CP,4))){y=114;height=52;}else{y=k==1?98:142;height=24;}}
  }
  if(ui.part==PART_909&&ui.sel[PART_909]==DR_BD&&drum_amp_ref(r)&&pg.group[page]!=GR_KIT){
   /* Approved 909 Kick AMP: a 32 px K5 DECAY summary bank; its LEVEL/TIME
    * curve fills the lower panel below that bank, inside the screen. */
   x0=6;x1=234;y+=32;height=237-y;
  }
  for(int sy=0;sy<240;sy++)for(int sx=0;sx<240;sx++)if(pixel_frame_owner[sy*240+sx]==owner){
   pixels++;if(fb[sy*240+sx]!=pixel_frame_color[sy*240+sx])missing++;
   if(sx<bx0)bx0=sx;if(sx>bx1)bx1=sx;if(sy<by0)by0=sy;if(sy>by1)by1=sy;
   if(sx<x0||sx>=x1||sy<y||sy>=y+height)outside++;
  }
  pixel_checks++;
  if(!pixels){pixel_missing++;printf("GLYPH_PIXEL_MISSING screen=%s ref=%u:%u:%u:%u\n",screen,r.kind,r.a,r.b,r.c);}
  if(missing||outside){pixel_damaged++;printf("GLYPH_PIXEL_DAMAGED screen=%s ref=%u:%u:%u:%u pixels=%u overpaint=%u outside=%u region=%d,%d..%d,%d drawn=%d,%d..%d,%d\n",screen,r.kind,r.a,r.b,r.c,pixels,missing,outside,x0,y,x1,y+height,bx0,by0,bx1+1,by1+1);}
 }
}
/* L11: every reached top bar draws the 4-level battery (outline, tip, three
 * cells: level 0..3) in the final frame; the walk rotates the level. */
static unsigned battery_checks,battery_missing,battery_level_seen[4];
static void required_battery(const char *screen) {
 int level=plat_battery(),bars=level<0?0:level>3?3:level,bad=0;
 uint16_t ink=swap16(SOUND_GRAY),off=swap16(SOUND_LINE);
 for(int x=219;x<233;x++)bad+=fb[5*240+x]!=ink||fb[12*240+x]!=ink;
 for(int y=5;y<13;y++)bad+=fb[y*240+219]!=ink||fb[y*240+232]!=ink;
 for(int y=7;y<11;y++)for(int x=233;x<235;x++)bad+=fb[y*240+x]!=ink;
 for(int cell=0;cell<3;cell++)for(int y=7;y<11;y++)for(int x=221+cell*4;x<224+cell*4;x++)
  bad+=fb[y*240+x]!=(cell<bars?ink:off);
 battery_checks++;battery_level_seen[bars]++;
 if(bad){battery_missing++;printf("BATTERY_MISSING screen=%s level=%d pixels=%d\n",screen,level,bad);}
}
/* Row 7: every reached synth-sequencer screen is the R22 piano roll: both
 * main bands drawn by the roll and its note area inked in the final frame. */
static void required_r22_roll(const char *screen) {
 unsigned bands=r22_roll_bands;r22_roll_bands=0;
 if(ui.view!=V_SEQ||!is_303()||ui.overlay!=O_NONE||ui.help)return;
 unsigned ink=0;
 for(int y=HDR_H+4;y<HDR_H+2*BAND_H-4;y++)for(int x=0;x<240;x++)ink+=fb[y*240+x]!=swap16(C_BLACK);
 r22_roll_screens++;
 if(bands!=2||ink<200){r22_roll_missing++;printf("R22_ROLL_MISSING screen=%s bands=%u ink=%u\n",screen,bands,ink);}
}
/* Row 7: a value drawn on its own must have its unit drawn in the same frame. */
static void required_units(const char *screen) {
 unsigned n=unit_nseen;unit_nseen=0;
 for(unsigned i=0;i<n;i++) {
  const unit_fmt *u=&unit_seen[i];int drawn=0,has_unit=0;
  for(unsigned k=0;k<geo_nframe;k++){if(!strcmp(geo_frame[k].text,u->num))drawn=1;if(strstr(geo_frame[k].text,u->unit))has_unit=1;}
  if(!drawn)continue;
  unit_checks++;
  if(!has_unit){unit_missing++;printf("UNIT_MISSING screen=%s ref=%08x value=[%s] unit=[%s]\n",screen,u->ref,u->num,u->unit);}
 }
}
/* Page compliance (UI-SOURCE-OF-TRUTH "Pan/units", "Keep a consistent header"). */
static void required_no_raw(const char *screen) {
 unsigned n=raw_nseen;raw_nseen=0;
 for(unsigned i=0;i<n;i++){int drawn=0;for(unsigned k=0;k<geo_nframe;k++)if(!strcmp(geo_frame[k].text,raw_seen[i].num))drawn=1;
  if(!drawn)continue;raw_checks++;
  if(!raw_seen[i].raw)continue;
  if(strstr(raw_seen[i].name,"VEL")||strstr(raw_seen[i].name,"Vel"))continue;   /* genuine MIDI velocity */
  raw_bad++;printf("RAW_VALUE screen=%s ref=%08x name=%s value=[%s]\n",screen,raw_seen[i].ref,raw_seen[i].name,raw_seen[i].num);}
}
static void required_unit_names(const char *screen) {
 static const char *const ok[]={"st","ms","s","Hz","kHz","dB","%","bpm","bar","steps","ct","oct","x"};
 for(unsigned i=0;i<unit_nseen;i++){const char *u=unit_seen[i].unit;int good=0;
  for(unsigned k=0;k<sizeof ok/sizeof*ok;k++)if(!strcmp(u,ok[k]))good=1;
  unitname_checks++;if(!good){unitname_bad++;printf("UNIT_NAME screen=%s ref=%08x value=[%s] unit=[%s]\n",screen,unit_seen[i].ref,unit_seen[i].num,u);}}
}
static void required_header(const char *screen) {
 /* Overlays (hold-button help, modals) may use the context slot; partial edge
  * redraws do not repaint the bar. Every other page: NAME, context, BPM, battery. */
 if(!strncmp(screen,"help-",5)||!strncmp(screen,"modal-",6)||!strncmp(screen,"edge-",5))return;
 int name=0,ctx=0,bpm=0;
 for(unsigned k=0;k<geo_nframe;k++){const geo_label *e=&geo_frame[k];if(e->y>=HDR_H)continue;
  if(e->x<=8&&e->f==&FONT_B)name=1;
  else if(e->x>=120&&e->x<180&&(e->text[0]=='P'||e->text[0]=='S'||!strncmp(e->text,"STEP",4)))ctx=1;
  else if(e->x>=150){const char *q=e->text;int d=*q!=0;for(;*q;q++)if(*q<'0'||*q>'9')d=0;if(d)bpm=1;}}
 header_checks++;
 if(!name||!ctx||!bpm){header_bad++;printf("HEADER_MISSING screen=%s name=%d context=%d bpm=%d\n",screen,name,ctx,bpm);}
}
/* #62 type scale at 240x240 (UI-DEFECT-TESTS-SPEC): a label's cap height >= 8 px, a value's
 * (a number the walk formatted through pref_value) >= 11 px. Cap = measured ink rows of "0H". */
static unsigned type_checks,type_bad,type_census[5][32];
static int font_cap(const felucca_font_t *f) {
 static const felucca_font_t *seen[16];static int cap[16];static unsigned n;
 for(unsigned i=0;i<n;i++)if(seen[i]==f)return cap[i];
 unsigned was=geo_enabled;geo_enabled=0;cv_begin(240,40,0);cv_text(2,2,f,"0H",65535);
 int top=-1,bot=-1;for(int y=0;y<40;y++)for(int x=0;x<60;x++)if(cv_px[y*240+x]){if(top<0)top=y;bot=y;}
 geo_enabled=was;int c=top<0?0:bot-top+1;if(n<16){seen[n]=f;cap[n++]=c;}return c;
}
static void required_type_scale(const char *screen) {
 if(!strncmp(screen,"help-",5))return;     /* hold-button help sheets: documented exception */
 for(unsigned k=0;k<geo_nframe;k++){const geo_label *e=&geo_frame[k];if(e->y<HDR_H||!e->text[0])continue;
  int cap=font_cap(e->f),fn=e->value;
  /* primary knob readouts (cell_value) and large drum readouts are values: >= 11 px. Compact
   * strip/cell readouts (fn 2..4, small drum) are secondary context in approved layouts and
   * share the label floor; they are counted separately in the census. */
  int primary=fn==1||(fn==4&&e->f!=&FONT_T),need=primary?11:8;
  type_census[fn][cap<32?cap:31]++;type_checks++;
  if(cap<need){type_bad++;if(type_bad<=40)printf("TYPE_SCALE_BAD screen=%s fn=%d text=[%s] cap=%d need=%d x=%d y=%d\n",screen,fn,e->text,cap,need,e->x,e->y);}}
}
/* Calm pages (FINAL-REFERENCES 23 Nov, MASTER-level): the selected value shown once, no filler
 * values repeated for uniformity, no label printed twice, no knob-number prefix, no listed
 * abbreviation. Text below the header bar only; help sheets are exempt. */
static unsigned dupread_checks,dupread_bad,duplabel_checks,duplabel_bad,kprefix_checks,kprefix_bad,abbrev_checks,abbrev_bad;
static int letters(const char *t){int n=0;for(;*t;t++)n+=(*t>='A'&&*t<='Z')||(*t>='a'&&*t<='z');return n;}
static int has_word(const char *t,const char *w){size_t n=strlen(w);
 for(const char *q=t;(q=strstr(q,w));q++){int l=q==t||!isalnum((unsigned char)q[-1]),r=!isalnum((unsigned char)q[n]);if(l&&r)return 1;}return 0;}
static void required_calm_text(const char *screen) {
 if(!strncmp(screen,"help-",5))return;
 static const char *const abbrev[]={"BRK","CHI","MIA","CMPLX","Cmplx","RTG2X","Rtg2x","RELEAS","TRANSP","A LEN","B LEN","BChnc","LEN","DIR","FDBK"};
 for(unsigned k=0;k<geo_nframe;k++){const geo_label *e=&geo_frame[k];if(e->y<HDR_H||!e->text[0])continue;
  const char *t=e->text;
  kprefix_checks++;
  for(const char *q=t;*q;q++)if(*q=='K'&&q[1]>='1'&&q[1]<='8'&&(q==t||!isalnum((unsigned char)q[-1]))&&(q[2]==' '||isalpha((unsigned char)q[2]))){
   kprefix_bad++;printf("KNOB_PREFIX screen=%s text=[%s]\n",screen,t);break;}
  abbrev_checks++;
  for(unsigned i=0;i<sizeof abbrev/sizeof*abbrev;i++)if(has_word(t,abbrev[i])){abbrev_bad++;printf("ABBREV screen=%s word=%s text=[%s]\n",screen,abbrev[i],t);break;}
  for(unsigned j=0;j<k;j++){const geo_label *o=&geo_frame[j];if(o->y<HDR_H||strcmp(o->text,t)||(o->x==e->x&&o->y==e->y))continue;
   if(e->value&&o->value&&e->ref&&e->ref==o->ref){dupread_bad++;printf("DUP_READOUT screen=%s ref=%08x text=[%s]\n",screen,e->ref,t);break;}
   /* a repeated name inside one section/row band (24 px); group-scoped names under different
    * titled groups (Miami CARRIER WAVE vs XMOD WAVE) are separate controls */
   static const char *const benign[]={"kHz","Hz","ms","dB","bpm","steps","oct","ON","OFF"};   /* units and list values */
   int ok=0;for(unsigned i=0;i<sizeof benign/sizeof*benign;i++)if(!strcmp(t,benign[i]))ok=1;
   if(!ok&&!e->value&&!o->value&&letters(t)>=3&&abs(o->y-e->y)<=24){duplabel_bad++;printf("DUP_LABEL screen=%s text=[%s]\n",screen,t);break;}}
  if(e->value)dupread_checks++;else duplabel_checks++;}
/* uniformity filler: one value text drawn by strip readouts on 6+ strips (edge walks set
  * every control to its min/max on purpose, so they are exempt) */
 if(!strncmp(screen,"edge-",5))return;
 for(unsigned k=0;k<geo_nframe;k++){const geo_label *e=&geo_frame[k];if(e->y<HDR_H||(e->value!=2&&e->value!=3))continue;
  unsigned n=0,first=1;for(unsigned j=0;j<geo_nframe;j++){const geo_label *o=&geo_frame[j];if((o->value==2||o->value==3)&&o->y>=HDR_H&&!strcmp(o->text,e->text)){n++;if(j<k)first=0;}}
  if(first&&n>=6){dupread_bad++;printf("DUP_UNIFORM screen=%s text=[%s] strips=%u\n",screen,e->text,n);}}
}
/* Row 63 dead space: the largest rectangle of background (the frame's two most common colours)
 * inside the main area stays under 25% of the 240x240 screen. Data canvases that are empty only
 * because the walk has no data are exempt and listed: SONG rows (empty song), synth SEQ rolls (no
 * notes), the MASTER scope (silence); help sheets and partial edge redraws too. */
static unsigned dead_checks,dead_bad;
static int u16cmp(const void *a,const void *b){return (int)*(const uint16_t*)a-(int)*(const uint16_t*)b;}
static void required_dead_space(const char *screen) {
 static const char *const exempt[]={"help-","edge-","view-06-","list-view-06-","engine-seq-","view-08-part-02-","view-08-part-03-","home-"};
 for(unsigned i=0;i<sizeof exempt/sizeof*exempt;i++)if(!strncmp(screen,exempt[i],strlen(exempt[i])))return;
 if(ui.view==V_MIX&&cur_page()==3)return;                       /* MASTER scope */
 enum{Y0=HDR_H,H=240-HDR_H};static uint16_t px[240*H];memcpy(px,fb+Y0*240,sizeof px);
 qsort(px,240*H,2,u16cmp);uint16_t c1=0,c2=0;int n1=0,n2=0;
 for(int i=0;i<240*H;){int j=i;while(j<240*H&&px[j]==px[i])j++;int n=j-i;
  if(n>n1){c2=c1;n2=n1;c1=px[i];n1=n;}else if(n>n2){c2=px[i];n2=n;}i=j;}
 static int hist[240];memset(hist,0,sizeof hist);int best=0,bx=0,by=0,bw=0,bh=0;
 for(int y=0;y<H;y++){for(int x=0;x<240;x++){uint16_t c=fb[(Y0+y)*240+x];hist[x]=(c==c1||c==c2)?hist[x]+1:0;}
  int st[241],sh[241],top=0;
  for(int x=0;x<=240;x++){int cur=x<240?hist[x]:0,start=x;
   while(top&&sh[top-1]>=cur){top--;int a=sh[top]*(x-st[top]);if(a>best){best=a;bx=st[top];bw=x-st[top];bh=sh[top];by=y-bh+1;}start=st[top];}
   st[top]=start;sh[top]=cur;top++;}}
 dead_checks++;
 if(best*4>240*240){dead_bad++;printf("DEAD_SPACE screen=%s area=%.3f box=%d,%d %dx%d\n",screen,best/(240.*240.),bx,by+Y0,bw,bh);}
}
/* KNOB_GROUPS (#48/#71): every engine page binds 1..4 controls to K5-K8, no control
 * twice, and every control of the engine's table is reachable on some page. */
static unsigned knob_group_checks,knob_group_bad;
static void required_knob_groups(unsigned kind,int part) {
 unsigned want=kind==ORIGINAL_SOUND_CHICAGO?20u:kind==ORIGINAL_SOUND_SYNTHONE?0u:16u;
 uint32_t seen=0;int dup=0;
 for(int i=0;i<pg.n;i++){
  int n=0;
  for(int k=0;k<4;k++){pref_t r=pg.r[i][k];if(r.kind==R_NONE)continue;n++;
   if(r.kind==R_ENG&&r.a==T_303&&r.c<32){if(seen>>r.c&1u)dup++;seen|=1u<<r.c;}}
  knob_group_checks++;
  if(n<1||n>4){knob_group_bad++;printf("KNOB_GROUPS kind=%u part=%d page=%s bound=%d\n",kind,part,pg.title[i],n);}
 }
 if(dup){knob_group_bad++;printf("KNOB_GROUPS kind=%u part=%d duplicate=%d\n",kind,part,dup);}
 for(unsigned id=0;id<want;id++){knob_group_checks++;
  if(!(seen>>id&1u)){knob_group_bad++;printf("KNOB_GROUPS kind=%u part=%d unreachable=%s\n",kind,part,ui_original_name(kind,id));}}
}
static unsigned audit_frames;
static void geo_audit(const char *screen) {
 static unsigned calls;
 if(calls++==0 && getenv("AUDIT_MUTATE_SKIP"))return;
 unsigned audited=0,before=geo_findings;
 required_drum_pixels(screen);
 required_strip_content(screen);
 required_other_content(screen);
 required_drum_content(screen);
 required_battery(screen);
 required_r22_roll(screen);
 required_type_scale(screen);
 required_calm_text(screen);
 required_dead_space(screen);
 required_no_raw(screen);
 required_unit_names(screen);
 required_header(screen);
 required_units(screen);
 geo_enabled=0;
 for(unsigned i=0;i<geo_nframe;i++) {
  geo_label e=geo_frame[i];unsigned expected=0,missing=0;
  cv_begin(240,138,0);cv_text(10,0,e.f,e.text,65535);
  for(int y=0;y<e.f->h;y++)for(int x=0;x<240;x++)if(cv_px[y*240+x]==65535) {
   expected++;int sx=e.x+x-10,sy=e.y+y;
   if(getenv("GEOMETRY_MUTATE") && sx>=0&&sx<240&&sy>=0&&sy<240)fb[sy*240+sx]^=65535;
   if(sx<0||sx>=240||sy<0||sy>=240||fb[sy*240+sx]!=swap16(e.col))missing++;
  }
  if(expected)audited++;
  if(expected && !missing && getenv("FWUI_TEXT_TRACE"))
      printf("UI_TEXT\t%s\t%d\t%d\t%s\n", screen, e.x, e.y, e.text);
  if(missing){geo_findings++;printf("GEOMETRY screen=%s text=[%s] at=%d,%d missing=%u/%u\n",screen,e.text,e.x,e.y,missing,expected);}
 }
 if(audit_frames==0 && getenv("AUDIT_MUTATE_ZERO"))audited=0;
 printf("AUDIT {\"kind\":\"screen\",\"id\":\"%s\",\"texts\":%u,\"findings\":%u}\n",screen,audited,geo_findings-before);
 audit_frames++;
 host_battery_level=(int)(audit_frames%4u); /* next reached screen shows the next level */
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
                walkshot(name);geo_audit(name);
                printf("SCREEN {\"id\":\"%s\",\"view\":%d,\"part\":%d,\"voice\":%d,\"page\":%d,\"pages\":%d,\"width\":240,\"height\":240}\n",name,view,part,voice,page,pages);
                total++;
                for(int edge=0;edge<2;edge++) {
                    int old[4];pref_t refs[4];
                    for(int k=0;k<4;k++){refs[k]=pg.r[page][k];old[k]=pref_get(refs[k]);const x0x_param_t *d=pref_desc(refs[k]);if(refs[k].kind==R_ENG && d)pref_set(refs[k],edge?d->max:0);}
                    geo_nframe=0;memset(pixel_frame_owner,0,sizeof pixel_frame_owner);presence_nglyph=0;geo_enabled=1;memset(blit_hash,0,sizeof blit_hash);memset(cv_tile_valid,0,sizeof cv_tile_valid);ui_frame();
                    snprintf(name,sizeof name,"edge-%d-view-%02d-part-%02d-voice-%02d-page-%02d",edge,view,part,voice,page);walkshot(name);geo_audit(name);total++;
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
            char name[80];snprintf(name,sizeof name,"global-section-%d-row-%02d",section,row);walkshot(name);geo_audit(name);
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
    char name[96];snprintf(name,sizeof name,"list-view-%02d-part-%02d-voice-%02d-row-%02d",view,part,voice,row);walkshot(name);geo_audit(name);
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
      required_knob_groups(kind,part);
      for(int page=0;page<pages;page++) {
       *page_of(V_PART)=(uint8_t)page;build_pages();geo_nframe=0;memset(pixel_frame_owner,0,sizeof pixel_frame_owner);presence_nglyph=0;geo_enabled=1;
       memset(blit_hash,0,sizeof blit_hash);memset(cv_tile_valid,0,sizeof cv_tile_valid);ui_frame();
       char name[80];snprintf(name,sizeof name,"engine-%u-part-%d-page-%d",kind,part,page);walkshot(name);geo_audit(name);total++;
      }
    }

    /* Owner-review routes omitted by the ordinary default-bank traversal. */
    for(unsigned kind=0;kind<4;kind++)for(int part=PART_303A;part<=PART_303B;part++) {
      setup(V_SEQ,part);
      CHECK(engine_bass_select_preset(part-PART_303A,kind,0,&proj.sound)==0);
      for(int block=0;block<12;block++){int32_t audio[512];engine_render(audio,256);}
      CHECK(!engine_bass_preset_pending(part-PART_303A));build_pages();int pages=pg.n;
      int has_arp=X0X_ARP_HOOKS &&
          (kind==ORIGINAL_SOUND_SYNTHONE || kind==ORIGINAL_SOUND_JUNO);
      CHECK(pages==3+has_arp);
      if(has_arp) {
       CHECK(!strcmp(pg.title[3],"ARP SEQ"));
       CHECK(pg.r[3][0].kind==R_ARP && pg.r[3][0].a==ARP_F_LENGTH);
       CHECK(pg.r[3][1].kind==R_ARP && pg.r[3][1].a==ARP_F_INTERVAL);
       CHECK(pg.r[3][2].kind==R_ARP && pg.r[3][2].a==ARP_F_GATE);
      }
      for(int page=0;page<pages;page++) {
       *page_of(V_SEQ)=(uint8_t)page;build_pages();geo_nframe=0;memset(pixel_frame_owner,0,sizeof pixel_frame_owner);presence_nglyph=0;geo_enabled=1;
       memset(blit_hash,0,sizeof blit_hash);memset(cv_tile_valid,0,sizeof cv_tile_valid);ui_frame();
       char name[80];snprintf(name,sizeof name,"engine-seq-%u-part-%d-page-%d",kind,part,page);walkshot(name);geo_audit(name);total++;
      }
    }
    setup(V_HOME,PART_909);button_tap(B_PLAY);{int32_t audio[512];engine_render(audio,256);}CHECK(seq.playing);
    {geo_nframe=0;memset(pixel_frame_owner,0,sizeof pixel_frame_owner);presence_nglyph=0;geo_enabled=1;
     memset(blit_hash,0,sizeof blit_hash);memset(cv_tile_valid,0,sizeof cv_tile_valid);ui_frame();
     const char *name="home-playing";walkshot(name);geo_audit(name);total++;
    }
    button_tap(B_PLAY);{int32_t audio[512];engine_render(audio,256);}CHECK(!seq.playing);
    setup(V_HOME,PART_909);button_tap(B_GLO);CHECK(ui.overlay==O_LIST);
    ui.list_sel=4;button_tap(B_SEL);CHECK(ui.glo_section==4);
    int clear_row=-1;
    for(int row=0;row<ui.list_n;row++)if(list_rows[row].kind==R_ACT && list_rows[row].a==ACT_CLEAR_PAT)clear_row=row;
    CHECK(clear_row>=0);ui.list_sel=(uint8_t)clear_row;button_tap(B_SEL);CHECK(ui.overlay==O_ASK);
    {geo_nframe=0;memset(pixel_frame_owner,0,sizeof pixel_frame_owner);presence_nglyph=0;geo_enabled=1;
     memset(blit_hash,0,sizeof blit_hash);memset(cv_tile_valid,0,sizeof cv_tile_valid);ui_frame();
     const char *name="modal-clear-pattern";walkshot(name);geo_audit(name);total++;
    }

    for(int view=0;view<NVIEWS;view++)for(int part=0;part<NPARTS;part++)for(int button=0;button<NB;button++) {
      if(view==V_GLO || (view==V_GEN && (part<PART_303A||part>PART_303B)))continue;
      setup(view,part);held_btn=ui.btn=1u<<button;ui.help=button+1;
      geo_nframe=0;memset(pixel_frame_owner,0,sizeof pixel_frame_owner);presence_nglyph=0;geo_enabled=1;memset(blit_hash,0,sizeof blit_hash);memset(cv_tile_valid,0,sizeof cv_tile_valid);ui_frame();
      CHECK(ui.help==button+1);char name[80];snprintf(name,sizeof name,"help-view-%d-part-%d-button-%d",view,part,button);walkshot(name);geo_audit(name);total++;
    }
    for(int modal=0;modal<4;modal++) {
      setup(V_HOME,PART_909);
      if(modal==0)ask(ACT_RESET,0,"RESET PROJECT","SAMPLES KEPT; EDITS LOST");
      if(modal==1)say("SAVED",0);
      if(modal==2)say("NEXT: PAT 16",0);
      if(modal==3)run_action(ACT_ABOUT,0);
      geo_nframe=0;memset(pixel_frame_owner,0,sizeof pixel_frame_owner);presence_nglyph=0;geo_enabled=1;memset(blit_hash,0,sizeof blit_hash);memset(cv_tile_valid,0,sizeof cv_tile_valid);ui_frame();
      char name[80];snprintf(name,sizeof name,"modal-%d",modal);walkshot(name);geo_audit(name);total++;
    }
    printf("INVENTORY %u ordinary/global/list/engine/help/modal states\n",total);


    printf("WALK {\"kind\":\"complete\",\"screens\":%u}\n",total);
    printf("AUDIT {\"kind\":\"complete\",\"screens\":%u}\n",audit_frames);
    printf("GEOMETRY findings=%u screens=%u\n",geo_findings,total);
    printf("STRIP_PRESENCE checks=%u\n",strip_presence_checks);printf("FAMILY_PRESENCE list=%u synth=%u\n",list_presence_checks,synth_presence_checks);printf("PRESENCE checks=%u missing=%u\n",presence_checks,presence_missing);printf("GLYPH_PIXELS checks=%u missing=%u damaged=%u\n",pixel_checks,pixel_missing,pixel_damaged);
    printf("BATTERY checks=%u missing=%u levels=%u,%u,%u,%u\n",battery_checks,battery_missing,battery_level_seen[0],battery_level_seen[1],battery_level_seen[2],battery_level_seen[3]);
    printf("PAN_FORMAT checks=%u bad=%u\n",pan_format_checks,pan_format_bad);
    printf("R22_ROLL screens=%u missing=%u\n",r22_roll_screens,r22_roll_missing);
    printf("UNITS checks=%u missing=%u\n",unit_checks,unit_missing);
    printf("RAW_VALUES checks=%u bad=%u\n",raw_checks,raw_bad);
    for(int v=0;v<5;v++){static const char *const N[5]={"label","cell_value","strip_value","strip_value_fit","dv_value_text"};printf("TYPE_CENSUS %s:",N[v]);for(int c=0;c<32;c++)if(type_census[v][c])printf(" cap%d=%u",c,type_census[v][c]);printf("\n");}
    printf("TYPE_SCALE checks=%u bad=%u\n",type_checks,type_bad);
    printf("DUP_READOUT checks=%u bad=%u\n",dupread_checks,dupread_bad);
    printf("DUP_LABEL checks=%u bad=%u\n",duplabel_checks,duplabel_bad);
    printf("KNOB_PREFIX checks=%u bad=%u\n",kprefix_checks,kprefix_bad);
    printf("ABBREV checks=%u bad=%u\n",abbrev_checks,abbrev_bad);
    printf("DEAD_SPACE checks=%u bad=%u\n",dead_checks,dead_bad);
    printf("KNOB_GROUPS checks=%u bad=%u\n",knob_group_checks,knob_group_bad);
    printf("UNIT_NAMES checks=%u bad=%u\n",unitname_checks,unitname_bad);
    printf("HEADERS checks=%u bad=%u\n",header_checks,header_bad);
    return knob_group_bad||!knob_group_checks||type_bad||dupread_bad||duplabel_bad||kprefix_bad||abbrev_bad||dead_bad||raw_bad||unitname_bad||!unitname_checks||header_bad||!header_checks||unit_missing||!unit_checks||geo_findings||presence_missing||pixel_missing||pixel_damaged||battery_missing||pan_format_bad||!pan_format_checks||r22_roll_missing||r22_roll_screens<8?1:0;
}
