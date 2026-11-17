/* Full-row ADSR contract for currently admitted control rows.
 * Does not claim missing Synth One or independent SuperJ6 filter rows exist. */
#define SMC_UI_MAIN unused_smc_ui_main
#include "smc_ui_test.c"
static void audio_ms(unsigned ms) {int32_t a[512];unsigned left=ms*44100/1000;while(left){unsigned n=left>256?256:left;engine_render(a,n);left-=n;}}
static int envelope_page(int first) {
 for(int p=0;p<pg.n;p++) {
  int match=1;for(int k=0;k<4;k++)match&=pg.r[p][k].kind==R_ENG&&pg.r[p][k].c==first+k;
  if(match)return p;
 }
 return -1;
}
static uint32_t graph(int page) {
 cv_begin(240,82,C_BLACK);
 if(!getenv("ADSR_MISSING_NEGATIVE"))draw_adsr_inline(page,1,4,69,0);
 /* Detector mutation: remove timed curve ink, retaining baseline and text.
  * This is a negative fixture, never a production rendering option. */
 if(getenv("ADSR_CURVE_NEGATIVE"))
  for(int yy=26;yy<68;yy++)for(int xx=13;xx<=227;xx++)
   if(cv_px[yy*240+xx]==swap16(matrix_col()))cv_px[yy*240+xx]=swap16(C_BLACK);
 /* Only curve/marker area: changing percentage text cannot satisfy this. */
 uint32_t hash=2166136261u;
 for(int y=26;y<=68;y++)for(int x=13;x<=227;x++)hash=(hash^cv_px[y*240+x])*16777619u;
 return hash;
}
static void partial_rows_need_graph(int p) {
 pref_t saved[4];memcpy(saved,pg.r[p],sizeof saved);
 for(int k=0;k<4;k++)pref_set(saved[k],64);audio_ms(12);
 for(unsigned mask=1;mask<=15;mask++) {
  for(int k=0;k<4;k++)pg.r[p][k]=(mask&(1u<<k))?saved[k]:NONE;
  CHECK(adsr_row(p));
  ui_adsr_t e;int xs[5];float dur[3];adsr_values(&e,p);CHECK(e.present==mask);
  adsr_layout(&e,13,214,xs,dur);CHECK(xs[0]==13&&xs[4]==227);
  for(int k=0;k<4;k++)CHECK((xs[k+1]>xs[k])==!!(mask&(1u<<k)));
  (void)graph(p);int curve_lit=0;
  for(int y=26;y<=68;y++)for(int x=13;x<=227;x++)curve_lit+=cv_px[y*240+x]!=swap16(C_BLACK);
  CHECK(curve_lit>0);
  for(int k=0;k<4;k++) {
   if(mask&(1u<<k)) {
    int segment_ink=0;
    for(int x=xs[k]+1;x<xs[k+1];x++)for(int y=26;y<68;y++)segment_ink+=cv_px[y*240+x]==swap16(matrix_col());
    CHECK(segment_ink>0); /* baseline/text cannot satisfy a missing stage curve */
   } else {
    for(int x=4+k*59;x<4+(k+1)*59;x++)for(int y=12;y<26;y++)CHECK(cv_px[y*240+x]==swap16(C_BLACK));
   }
  }
 }
 for(int k=0;k<4;k++)pg.r[p][k]=NONE;CHECK(!adsr_row(p));
 pg.r[p][3]=saved[0];ui_adsr_t e;adsr_values(&e,p);CHECK(e.present==1&&e.slot[0]==3);CHECK(adsr_row(p));
 memcpy(pg.r[p],saved,sizeof saved);
 /* Reordering knobs must preserve stage identity, curve highlight and the
  * exact top-right stage/value text; only the knob readout positions move. */
 uint16_t expected[240*82];
 for(int stage=0;stage<4;stage++) {
  memcpy(pg.r[p],saved,sizeof saved);ui.touched=stage;ui.touch_until=now_ms+1000;
  (void)graph(p);memcpy(expected,cv_px,sizeof expected);
  for(int k=0;k<4;k++)pg.r[p][(k+1)%4]=saved[k];
  ui.touched=(stage+1)%4;(void)graph(p);
  for(int yy=4;yy<12;yy++)for(int xx=0;xx<240;xx++)CHECK(cv_px[yy*240+xx]==expected[yy*240+xx]);
  for(int yy=26;yy<=71;yy++)for(int xx=13;xx<=227;xx++)CHECK(cv_px[yy*240+xx]==expected[yy*240+xx]);
 }
 ui.touched=-1;
 memcpy(pg.r[p],saved,sizeof saved);
 pg.r[p][1]=saved[0];CHECK(!adsr_row(p)); /* duplicate stage is not a valid row */
 memcpy(pg.r[p],saved,sizeof saved);pg.r[p][0].b=(uint8_t)(1-bidx());CHECK(!adsr_row(p));
 memcpy(pg.r[p],saved,sizeof saved);pg.r[p][0].a=T_909;CHECK(!adsr_row(p)); /* requires partition/mapping */
 memcpy(pg.r[p],saved,sizeof saved);
}
int main(void) {
 const int banks[]={ORIGINAL_SOUND_CHICAGO,ORIGINAL_SOUND_JUNO};
 for(unsigned b=0;b<2;b++) {
  setup(V_PART,PART_303A);CHECK(!engine_bass_select_preset(0,banks[b],0,&proj.sound));
  for(int i=0;i<1024&&engine_bass_preset_pending(0);i++)audio_ms(6);
  CHECK(!engine_bass_preset_pending(0));build_pages();
  int p=envelope_page(b?5:7);CHECK(p>=0);CHECK(adsr_row(p));
  *page_of(V_PART)=p;build_pages();partial_rows_need_graph(p);
  for(int k=0;k<4;k++) {
   for(int j=0;j<4;j++)pref_set(pg.r[p][j],64);
   pref_set(pg.r[p][k],0);audio_ms(12);uint32_t lo=graph(p);
   if(!getenv("ADSR_STATIC_NEGATIVE"))pref_set(pg.r[p][k],127);audio_ms(12);CHECK(graph(p)!=lo);
  }
  for(int j=0;j<4;j++)pref_set(pg.r[p][j],40);pref_set(pg.r[p][2],80);pref_set(pg.r[p][3],100);audio_ms(12);
  uint32_t idle=graph(p);engine_bass_on_velocity(0,60,0,0,100);audio_ms(40);CHECK(graph(p)!=idle);
  engine_bass_off(0);audio_ms(1);uint32_t released=graph(p);CHECK(released!=idle);
  draw_header();draw_main();draw_knobs();draw_footer();shot(b?"adsr-juno":"adsr-chicago");
  if(b) {
   /* Raw Juno DECAY must not be relabelled sustain just because level is near S. */
   pref_set(pg.r[p][0],0);pref_set(pg.r[p][1],32);pref_set(pg.r[p][2],64);pref_set(pg.r[p][3],0);audio_ms(12);
   engine_bass_on_velocity(0,72,0,0,100);juno6_envelope_snapshot snap;int found=0;
   for(int i=0;i<500;i++) {audio_ms(1);CHECK(engine_juno_envelope_view(0,&snap));
    if(snap.stage==2 && snap.level>snap.adsr[2] && snap.level-snap.adsr[2]<.009f){found=1;break;}}
   CHECK(found);ui.touched=-1;(void)graph(p);
   ui_adsr_t e;int xs[5];float dur[3];adsr_values(&e,p);adsr_layout(&e,13,214,xs,dur);
   int x=(xs[1]+xs[2])/2,y=68-(int)(snap.level*42+.5f);
   CHECK(cv_px[y*240+x-2]==swap16(SOUND_INK));
   pref_t full[4];memcpy(full,pg.r[p],sizeof full);for(int k=0;k<4;k++)if(k!=2)pg.r[p][k]=NONE;
   (void)graph(p);CHECK(cv_px[y*240+7]==swap16(SOUND_INK));
   int fake_head=0;for(int yy=26;yy<=68;yy++)for(int xx=13;xx<=227;xx++)fake_head+=cv_px[yy*240+xx]==swap16(SOUND_INK);
   CHECK(fake_head==0); /* real DECAY omitted: separate level marker, no endpoint fiction */
   memcpy(pg.r[p],full,sizeof full);
   found=0;for(int i=0;i<500;i++){audio_ms(1);CHECK(engine_juno_envelope_view(0,&snap));if(snap.stage==3){found=1;break;}}
   CHECK(found);(void)graph(p);x=(xs[2]+xs[3])/2;y=68-(int)(snap.level*42+.5f);
   CHECK(cv_px[y*240+x-2]==swap16(SOUND_INK));engine_bass_off(0);
  }
  ui.touched=0;ui.touch_until=now_ms+1000;draw_main();draw_knobs();
  shot(b?"adsr-juno-attack-edited":"adsr-chicago-attack-edited");
 }
 printf("Operational full-row ADSR graphs: %u checks PASS; required unavailable engine roles NOT QUALIFIED\n",checks);
}
