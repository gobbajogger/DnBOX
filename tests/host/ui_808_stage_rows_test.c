/* Full-width bound stages, not duplicate small envelope pictograms. */
#define SMC_UI_MAIN retained_fixture_main
#include "smc_ui_test.c"
static uint16_t screen_before[240*240];
static void expect_ink(int x,int y,const char *text,uint16_t color){
 CHECK(x>=0 && y>=0 && x+tw(&FONT_T,text)<=240 && y+FONT_T.h<=240);
 memcpy(screen_before,fb,sizeof fb);cv_begin(240,16,SOUND_BASE);cv_text(0,0,&FONT_T,text,color);int ink=0;
 for(int yy=0;yy<FONT_T.h;yy++)for(int xx=0;xx<tw(&FONT_T,text);xx++)if(cv_px[yy*240+xx]==swap16(color)){
  if(screen_before[(y+yy)*240+x+xx]!=swap16(color))fprintf(stderr,"FIXED_OWNER_STAGE_LABEL_MISSING %s\n",text);
  CHECK(screen_before[(y+yy)*240+x+xx]==swap16(color));ink++;
 }CHECK(ink);
}
static void draw_stage_screen(void){draw_header();draw_main();draw_knobs();draw_footer();}
static void advance_stage(int n){int32_t audio[512];while(n){int c=n>256?256:n;engine_render(audio,c);n-=c;}}
static void expect_owner_tick(const drum_curve_t *v,int top,int height){
 if(v->elapsed>=0){float level=v->level<0?0:v->level>1?1:v->level;int y=top+height-(int)(height*level+.5f);
  CHECK(y>=top && y<=top+height);
  for(int x=230;x<233;x++){if(fb[y*240+x]!=swap16(SOUND_INK))fprintf(stderr,"ACTUAL_OWNER_LEVEL_TICK_MISSING\n");CHECK(fb[y*240+x]==swap16(SOUND_INK));}
 }else for(int y=top;y<=top+height;y++)for(int x=230;x<233;x++){if(fb[y*240+x]!=swap16(SOUND_FACE))fprintf(stderr,"INACTIVE_OWNER_LEVEL_TICK_PRESENT\n");CHECK(fb[y*240+x]==swap16(SOUND_FACE));}
}
static void test_actual_owner_ticks(void){
 pref_set(PR(R_ENG,T_808,D8_CP,4),1);pref_set(PR(R_ENG,T_808,D8_CP,3),127);advance_stage(3072);
 engine_drum(PART_808,D8_CP,1);advance_stage(256);
 pref_set(PR(R_ENG,T_808,D8_CP,4),0);advance_stage(256);engine_drum(PART_808,D8_CP,1);advance_stage(128);
 drum_curve_t a,d;CHECK(engine_drum808_param_curve(D8_CP,3,&a)==D8S_MA+1);CHECK(engine_drum808_param_curve(D8_CP,2,&d)==D8S_CP+1);
 CHECK(a.elapsed>=0 && d.elapsed<0);draw_stage_screen();expect_owner_tick(&a,100,20);expect_owner_tick(&d,144,20);shot("clap-with-actual-fixed-ma-rise");
 advance_stage(2048);CHECK(engine_drum808_param_curve(D8_CP,2,&d)==D8S_CP+1);CHECK(d.elapsed>=0);draw_stage_screen();expect_owner_tick(&d,144,20);shot("clap-actual-decay-stage");
 advance_stage(88200);engine_drum808_param_curve(D8_CP,3,&a);engine_drum808_param_curve(D8_CP,2,&d);CHECK(a.elapsed<0 && d.elapsed<0);draw_stage_screen();expect_owner_tick(&a,100,20);expect_owner_tick(&d,144,20);
}
static void test_held_attack_preview(void){
 pref_t r=PR(R_ENG,T_808,D8_CP,3);
 pref_set(PR(R_ENG,T_808,D8_CP,4),0);pref_set(r,32);advance_stage(3072);
 int k=motion_alloc(proj.arr.lane,seq.ppat[PART_808],PART_808,T_808,D8_CP,3);CHECK(k>=0);
 proj.arr.lane[k].val[0]=127;ui.hold_drum=PART_808*NDRUM+D8_CP;ui.hold_pat=seq.ppat[PART_808];ui.hold_step=0;seq.playing=1;
 CHECK(lock_step()==0 && matrix_value(r)==127);
 drum_curve_t before,after;engine_drum808_param_curve(D8_CP,3,&before);draw_stage_screen();engine_drum808_param_curve(D8_CP,3,&after);CHECK(!memcmp(&before,&after,sizeof before));
 memcpy(screen_before,fb,sizeof fb);cv_begin(240,21,SOUND_FACE);float xy[66];double tau=1.2/pow(32./127.,2),end=tau*log(100.);
 for(int j=0;j<=32;j++){double t=end*j/32.;xy[2*j]=13.f+214.f*(float)t/90.f;xy[2*j+1]=20.f-20.f*(float)(1-exp(-t/tau));}cv_poly_aa(xy,33,part_col());
 for(int y=0;y<20;y++)for(int x=13;x<=227;x++){if(screen_before[(100+y)*240+x]!=cv_px[y*240+x])fprintf(stderr,"HELD_MA_TARGET_CURVE_MISMATCH\n");CHECK(screen_before[(100+y)*240+x]==cv_px[y*240+x]);}
 seq.playing=0;ui.hold_drum=-1;ui.hold_step=-1;engine_motion_clear(k);
}
static void test_stage_steps(void){
 dpart_t *dp=&cur_pat()->drum[PART_808];dp->len=17;ui.spage=1;memset(dp->hit[D8_CP],0,sizeof dp->hit[D8_CP]);sm_set(dp->hit[D8_CP],16);
 for(int playing=0;playing<2;playing++){seq.playing=playing;eng_step[TRK_DRUM+PART_808]=16;draw_stage_screen();
  for(int c=0;c<16;c++){uint16_t expected=c?SOUND_LINE:playing?C_WHITE:part_col();for(int y=214;y<218;y++)for(int x=9+c*14;x<20+c*14;x++){if(fb[y*240+x]!=swap16(expected))fprintf(stderr,"BOUND_STAGE_STEP_LANE_MISMATCH\n");CHECK(fb[y*240+x]==swap16(expected));}}
 }
 seq.playing=0;ui.spage=0;dp->len=16;
}
static void settle_stage(void){int32_t audio[512];for(int i=0;i<12;i++)engine_render(audio,256);}
static void test_cp_fall_geometry(void){
 drum_curve_t v={.peak=1,.rise_tau=2,.rise_end=7.824046f,.fall=25,.window=650,.elapsed=-1};
 cv_begin(240,100,SOUND_FACE);dv_bound_stage(&v,2,10,0,part_col());
 uint16_t actual[240*30];memcpy(actual,cv_px,sizeof actual);
 cv_begin(240,100,SOUND_FACE);float xy[66];
 for(int i=0;i<=32;i++) {double t=650.*i/32.;double level=(1.-exp(-v.rise_end/v.rise_tau))*exp(-t/v.fall);xy[2*i]=13.f+214.f*i/32.f;xy[2*i+1]=30.f-20.f*(float)level;}
 cv_poly_aa(xy,33,part_col());
 if(memcmp(actual,cv_px,sizeof actual))fprintf(stderr,"CP_NOMINAL_FALL_GEOMETRY_MISMATCH\n");
 CHECK(!memcmp(actual,cv_px,sizeof actual));
}
static void test_ma_ad_geometry(void){
 drum_curve_t v={.peak=1,.rise_tau=2,.rise_end=9.21034f,.fall=25,.window=350,.elapsed=-1};
 cv_begin(240,100,SOUND_FACE);dv_bound_stage(&v,0,10,0,part_col());
 uint16_t actual[240*58];memcpy(actual,cv_px,sizeof actual);
 cv_begin(240,100,SOUND_FACE);float xy[68];double times[34];
 for(int i=0;i<=32;i++)times[i]=350.*i/32.;times[33]=v.rise_end;
 for(int i=0;i<34;i++)for(int j=i+1;j<34;j++)if(times[j]<times[i]){double t=times[i];times[i]=times[j];times[j]=t;}
 for(int i=0;i<34;i++) {double t=times[i];double peak=1.-exp(-v.rise_end/v.rise_tau);double level=t<v.rise_end?1.-exp(-t/v.rise_tau):peak*exp(-(t-v.rise_end)/v.fall);xy[2*i]=13.f+214.f*(float)t/350.f;xy[2*i+1]=58.f-48.f*(float)level;}
 cv_poly_aa(xy,34,part_col());
 if(memcmp(actual,cv_px,sizeof actual))fprintf(stderr,"MA_AD_VERTICAL_RESOLUTION_MISMATCH\n");
 CHECK(!memcmp(actual,cv_px,sizeof actual));
}
static void test_unavailable_stage(void){
 drum_curve_t v={0};
 cv_begin(240,100,SOUND_FACE);uint16_t before[240*100];memcpy(before,cv_px,sizeof before);
 dv_bound_stage(&v,1,10,0,part_col());
 if(memcmp(before,cv_px,sizeof before))fprintf(stderr,"UNAVAILABLE_STAGE_DREW_PIXELS\n");
 CHECK(!memcmp(before,cv_px,sizeof before));
}
int main(void){
 setup(V_PART,PART_808);ui.sel[PART_808]=D8_CP;seq.playing=0;engine_guard_enable(0);settle_stage();
 pref_set(PR(R_ENG,T_808,D8_CP,4),0);pref_set(PR(R_ENG,T_808,D8_CP,3),32);pref_set(PR(R_ENG,T_808,D8_CP,2),64);settle_stage();build_pages();
 test_ma_ad_geometry();
 test_unavailable_stage();
 CHECK(pg.r[0][0].c==1 && pg.r[0][1].c==3 && pg.r[0][2].c==4 && pg.r[0][3].c==2);
 for(int light=0;light<2;light++){
  pref_set(PR(R_PALETTE,0,0,0),light?5:0);draw_header();draw_main();draw_knobs();draw_footer();
  expect_ink(13,88,"MARACAS ATTACK",SOUND_GRAY);
  expect_ink(13,132,"CLAP DECAY",SOUND_GRAY);
  expect_ink(112,88,"99% 5.5ms",SOUND_GRAY);
  /* Independent CP law: raw64 maps .35*8^(64/127) seconds;
   * its decay coefficient uses .0385 of that, 99% fall takes ln(100). */
  CHECK((int)(350.*pow(8.,64./127.)*.0385*log(100.)+.5)==177);
  expect_ink(112,132,"99% 177ms",SOUND_GRAY);
  expect_ink(11,200,"HANDCLAP 808/7",part_col());
  char name[64];snprintf(name,sizeof name,"clap-two-real-owners-theme%d",light);shot(name);
 }
 /* The alternate selected owner must retain one AD graph and both bindings. */
 pref_set(PR(R_ENG,T_808,D8_CP,4),1);settle_stage();
 pref_set(PR(R_ENG,T_808,D8_CP,2),64);settle_stage();build_pages();
 for(int light=0;light<2;light++){
  pref_set(PR(R_PALETTE,0,0,0),light?5:0);draw_header();draw_main();draw_knobs();draw_footer();
  expect_ink(13,88,"MARACAS AD",SOUND_GRAY);
  expect_ink(13,100,"K6 A 25%",SOUND_INK);
  expect_ink(227-tw(&FONT_T,"K8 D 50%"),100,"K8 D 50%",SOUND_INK);
  char name[64];snprintf(name,sizeof name,"maracas-shared-ad-theme%d",light);shot(name);
 }
 test_cp_fall_geometry();
 test_unavailable_stage();
 test_actual_owner_ticks();
 test_stage_steps();
 test_held_attack_preview();
 puts("PASS two actual owner stage rows preserve physical parameter identities");return 0;
}
