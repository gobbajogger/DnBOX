/* Real BD/SD main-amp graphs; other drum roles remain explicitly unqualified. */
#define SMC_UI_MAIN unused_smc_ui_main
#include "smc_ui_test.c"
static void tick_ms(unsigned ms){int32_t a[512];unsigned n=ms*44100/1000;while(n){unsigned z=n>256?256:n;engine_render(a,z);n-=z;}}
static uint32_t graph_hash(int p){
 cv_begin(240,82,C_BLACK);if(!getenv("DRUM_GRAPH_MISSING"))draw_adsr_inline(p,1,4,69,0);
 uint32_t h=2166136261u;for(int y=26;y<68;y++)for(int x=13;x<=227;x++)h=(h^cv_px[y*240+x])*16777619u;
 return h;
}
static uint32_t screen_curve(int ep){
 *page_of(V_PART)=ep;draw_main();
 /* First mixed row remains 42px; the isolated active amp row follows it.
  * Exclude all text, include actual compositor curve pixels only. */
 uint32_t h=2166136261u;int ink=0;
 for(int y=92;y<119;y++)for(int x=13;x<=227;x++){h=(h^fb[y*240+x])*16777619u;ink+=fb[y*240+x]==swap16(matrix_col());}
 CHECK(ink>0);return h;
}
static void physical_amp_caption(int ep)
{
 static const int raw[3]={0,64,127};
 static const char *const expected[3]={"K5 D 0%","K5 D 50%","K5 D 100%"};
 uint16_t saved[120*12];CHECK(UI_FIRST_KNOB==5);
 for(int theme=0;theme<2;theme++)for(int i=0;i<3;i++){
  proj.set.palette=theme?5:0;palette_set(proj.set.palette);CHECK(ui_theme_light==(unsigned)theme);ui.touched=-1;pref_set(pg.r[ep][0],raw[i]);
  cv_begin(240,82,C_BLACK);draw_adsr_inline(ep,1,4,69,0);
  for(int y=0;y<12;y++)memcpy(saved+y*120,cv_px+(y+12)*240,120*sizeof(uint16_t));
  cv_begin(240,82,C_BLACK);text_c(75,12,&FONT_T,expected[i],SOUND_GRAY);
  for(int y=0;y<12;y++){
   if(memcmp(saved+y*120,cv_px+(y+12)*240,120*sizeof(uint16_t)))fprintf(stderr,"AMP_PHYSICAL_SLOT_CAPTION_MISSING\n");
   CHECK(!memcmp(saved+y*120,cv_px+(y+12)*240,120*sizeof(uint16_t)));
  }
 }
 proj.set.palette=0;palette_set(0);
}
int main(void){
 drum909_amp_snapshot v;CHECK(!engine_drum909_amp_view(-1,&v));CHECK(!engine_drum909_amp_view(DR_LT,&v));CHECK(!engine_drum909_amp_view(DR_BD,0));
 for(int voice=DR_BD;voice<=DR_SD;voice++){
  setup(V_PART,PART_909);ui.sel[PART_909]=voice;build_pages();
  int ep=-1,seen[NPARAMS_MAX]={0};
  for(int p=0;p<pg.n;p++)for(int k=0;k<4;k++){
   pref_t r=pg.r[p][k];if(r.kind!=R_ENG||r.a!=T_909||r.b!=voice)continue;
   CHECK(r.c<NPARAMS_MAX);seen[r.c]++;
   if(drum_amp_ref(r)){CHECK(ep<0);ep=p;CHECK(adsr_row(p));for(int j=1;j<4;j++)CHECK(pg.r[p][j].kind==R_NONE);}
   else CHECK(!adsr_row(p));
  }
  CHECK(ep>=0);physical_amp_caption(ep);
  for(int i=0;i<engine_nparams(T_909,voice);i++){
   const x0x_param_t *d=engine_param(T_909,voice,i);
   if(name_is(d,"Drive")||name_is(d,"Dist")||name_is(d,"Level")||name_is(d,"Pan")||name_is(d,"Rev")||name_is(d,"Dly"))continue;
   CHECK(seen[i]==1); /* no lost Tune/Attack/Pitch/Snappy controls */
  }
  pref_t r=pg.r[ep][0];pref_set(r,0);tick_ms(12);uint32_t lo=graph_hash(ep);
  if(!getenv("DRUM_GRAPH_STATIC"))pref_set(r,127);tick_ms(12);CHECK(graph_hash(ep)!=lo);
  CHECK(engine_drum909_amp_view(voice,&v));CHECK(v.target_decay_ms>0);
  uint32_t idle=graph_hash(ep);engine_drum(PART_909,voice,1);tick_ms(8);
  CHECK(engine_drum909_amp_view(voice,&v));CHECK(v.active);CHECK(v.target_hold_ms==(voice==DR_BD?40.0f:0.0f));CHECK(v.hold_active||v.ramp_valid);CHECK(graph_hash(ep)!=idle);
  uint32_t first=graph_hash(ep);tick_ms(8);CHECK(graph_hash(ep)!=first);
  tick_ms(74);
  CHECK(engine_drum909_amp_view(voice,&v));CHECK(v.ramp_valid);float latched=v.ramp_samples;
  pref_set(r,0);tick_ms(8);CHECK(engine_drum909_amp_view(voice,&v));CHECK(v.ramp_valid);CHECK(v.ramp_samples==latched);CHECK(v.target_decay_ms<v.ramp_samples/44.1f);
  /* A target edit must not jump the sounding ramp cursor onto the next-note
   * preview. Check the actual last-returned sample's native counter position. */
  float maximum=0;CHECK(drum909_param_si(voice,r.c,127,&maximum)==X0X_U_MS);
  float elapsed=(voice==DR_BD?40.0f:0.0f)+(v.ramp_ticks-v.ramp_left-1)/44.1f;
  int px=13+(int)(214*elapsed/(maximum+v.target_hold_ms)+.5f);
  int py=68-(int)(v.raw_level*42+.5f);
  (void)graph_hash(ep);CHECK(cv_px[(py-1)*240+px-1]==swap16(SOUND_INK));

  pref_set(r,0);tick_ms(8);uint32_t screen_lo=screen_curve(ep);
  pref_set(r,127);tick_ms(8);CHECK(screen_curve(ep)!=screen_lo);
  pref_set(r,64);tick_ms(8);engine_drum(PART_909,voice,1);tick_ms(8);
  *page_of(V_PART)=ep;draw_header();draw_main();draw_knobs();draw_footer();shot(voice==DR_BD?"drum-bd-envelope":"drum-sd-envelope");
 }
 printf("BD/SD true amp graph and partition: %u checks PASS; other roles NOT QUALIFIED\n",checks);
}
