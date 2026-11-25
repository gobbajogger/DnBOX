#define X0X_ARP_KEYBOARD_TEST 1
#define MIXED_ONLY 1
#include "s1_pb_mod_ui.c"
#include "dsp/juno6.h"
#include "app/original_runtime.h"
static juno6_t *last_juno;
void __real_juno6_init(juno6_t *, float);
void __wrap_juno6_init(juno6_t *p,float hz) { __real_juno6_init(p,hz);last_juno=p; }
static void select_owner(unsigned owner,const char *page)
{
 ui.overlay=O_NONE;ui.part=PART_303A+owner;set_view(V_PART);build_pages();
 visit(page);smc_peek_cancel();smc_page_set(cur_page());
}
static unsigned count_page(const char *name)
{
 unsigned n=0;for(int i=0;i<pg.n;i++)n+=!strcmp(page_title(&pg,i),name);return n;
}
static void arrange(unsigned s)
{
 project_reset_stage();project_stage.set.snd_rev=REV137;
 for(unsigned i=0;i<NBASS;i++) {
  CHECK(original_sound_init(project_stage.sound.v[T_303][i],i==s?ORIGINAL_SOUND_SYNTHONE:ORIGINAL_SOUND_JUNO,0));
  engine_bass_sound_refresh(&project_stage.sound,i);
  slot137(&project_stage.sound,i)[0]=137;
  slot137(&project_stage.sound,i)[1]=(uint8_t)i;
 }
 CHECK(!project_validate_stage());project_commit_stage();stabilize();undo_start();
}
static dnbx_voice *held(juno6_t *j)
{
  for(unsigned i=0;i<DNBX_VOICES;++i)if(j->core.voice[i].gate && j->core.voice[i].note==48)
     return &j->core.voice[i];
  return NULL;
}
static void pitch_window(unsigned own,juno6_t *j,const float begin[2],float bend)
{
  dnbx_voice *v=held(j);
  CHECK(v && j->core.superj6_bend_semitones==bend);
  for(unsigned l=0;l<DNBX_LAYERS;++l) {
   CHECK(fabsf(v->layer[l].increment/begin[l]-exp2f(bend/12.f))<.00003f);
  }
  CHECK(j->core.layer[1].params.tune_semitones-j->core.layer[0].params.tune_semitones==3.f);
}
static void edit_juno(unsigned j,unsigned s,juno6_t *juno)
{
 select_owner(j,"WHEELS");
 CHECK(count_page("WHEELS")==1 && pg.r[cur_page()][0].kind==R_SJ6PB && pg.r[cur_page()][1].kind==R_SJ6PB);
 char num[16],unit[16];
 pref_value(pg.r[cur_page()][0],num,unit);CHECK(!strcmp(num,"-2")&&!strcmp(unit,"st"));
 CHECK(juno->pb_typed && juno->pb_down==2 && juno->pb_up==2);
 undo_start();
 detent(0,-127); detent(1,127); render(4);
 CHECK(juno->pb_down==48 && juno->pb_up==48 && read137(&proj.sound,j)[10]==48 && read137(&proj.sound,j)[11]==48);
 CHECK(undo_commit_stable());
 for(unsigned n=0;undo.cur && n<8;++n)physical_history(0);
 CHECK(undo.cur==0 && juno->pb_down==2 && juno->pb_up==2);
 for(unsigned n=0;undo.cur<undo.n && n<8;++n)physical_history(1);
 CHECK(undo.cur==undo.n && juno->pb_down==48 && juno->pb_up==48);
 juno6_set_layer_semi(juno,1,SJ6_SEMI_CENTER+3);
 engine_bass_on(j,48,0,0); render(2);
 dnbx_voice *v=held(juno);
 CHECK(v);
 float begin[]={v->layer[0].increment,v->layer[1].increment};
 float sp=current->e.t[31];
 pb_adc(500);pb_adc(250);pitch_window(j,juno,begin,-48.f);
 ui.part=PART_303A+s;pb_adc(750);pitch_window(j,juno,begin,48.f);
 CHECK(current->e.t[31]==sp);
 pb_adc(500);pitch_window(j,juno,begin,0.f);
 pb_midi(j,16383);pitch_window(j,juno,begin,48.f*(8191.f/8192.f));
 select_owner(j,"WHEELS");detent(1,-1);render(3);
 CHECK(juno->pb_down==48 && juno->pb_up==47);
 pitch_window(j,juno,begin,47.f*(8191.f/8192.f));
 /* Actual newly-triggered voice inherits a nonzero transient common bend. */
 engine_bass_off(j);render(8);engine_bass_on(j,48,0,0);render(8);
 pitch_window(j,juno,begin,47.f*(8191.f/8192.f));
 pb_midi(j,8192);engine_bass_off(j);render(8);
}
static void j6_cc1(unsigned owner,unsigned value)
{
 unsigned ch=seq.ch[TRK_BASS0+owner];
 host_midi_put(0xbu|((0xb0u|ch)<<8)|(1u<<16)|(value<<24));render(1);
}
static void joined_selector(unsigned owner,juno6_t *s)
{
 select_owner(owner,"WHEELS"); const pref_t r=pg.r[cur_page()][2];
 CHECK(r.kind==R_SJ6MOD && pref_desc(r)->max==5 && pref_get(r)==0);
 CHECK(s->lfo1_pwm_route==3 && s->mod_destination==0);
 CHECK(juno6_arm_alt3_running(s,10000,1));
 undo_start(); unsigned writes=store_writes;
 for(unsigned i=1;i<=5;i++){
   float cutoff=s->mod_wheel_cutoff_hz,vib=s->vibrato_depth,depth=s->alt3.to_depth;
   unsigned position=engine_bass_perf(owner,2,1);
   detent(2,1);render(4);
   CHECK(s->mod_destination==i && pref_get(r)==(int)i && read137(&proj.sound,owner)[14]==i);
   CHECK(cutoff==s->mod_wheel_cutoff_hz&&vib==s->vibrato_depth&&depth==s->alt3.to_depth&&(unsigned)engine_bass_perf(owner,2,1)==position);
 }
 CHECK(store_writes==writes && undo_commit_stable());
 for(unsigned n=0;undo.cur&&n<8;++n)physical_history(0);
 CHECK(undo.cur==0&&s->mod_destination==0&&read137(&proj.sound,owner)[14]==0);
 for(unsigned n=0;undo.cur<undo.n&&n<8;++n)physical_history(1);
 CHECK(undo.cur==undo.n&&s->mod_destination==5&&read137(&proj.sound,owner)[14]==5);
 select_owner(owner,"WHEELS");
 for(unsigned i=5;i>0;i--){detent(2,-1);render(1);}
 CHECK(s->mod_destination==0);
 for(unsigned i=0;i<6;i++){
   if(i)detent(2,1);
   if(i==5u)for(unsigned align=0;s->lfo1_cut_until<30000&&align<16;align++)render(1);
   unsigned velocity=90u+i;j6_cc1(owner,velocity);
   CHECK(s->mod_destination==i);
   switch(i){
   case 0:CHECK(s->mod_wheel_lfo1==velocity+1);break;
   case 1:CHECK(s->vibrato_depth>0.f);break;
   case 2:CHECK(s->wheel_lfo1_amount==velocity+1);break;
   case 3:CHECK(s->alt3.last_wheel_code==velocity+1&&s->alt3.tempo_sync);break;
   case 4:CHECK(s->mod_wheel_lfo2==velocity+1);break;
   case 5:CHECK(s->mod_wheel_cutoff_hz==juno6_mod_cutoff_hz(velocity));break;
   }
 }
 select_owner(owner,"WHEELS");adc(615);adc(615);
 engine_bass_on(owner,43,0,0);render(1);
 adc(485);adc(485);CHECK(engine_bass_perf(owner,2,1)==127);
 ui.part=PART_303A+(1-owner);adc(550);adc(550);
 int live=engine_bass_perf(owner,2,1);CHECK(live>0 && live<127);
 adc(615);adc(615);engine_bass_off(owner);render(12);
 CHECK(read137(&proj.sound,owner)[14]==5);
 project_capture_stage();slot137(&project_stage.sound,owner)[14]=6;CHECK(project_validate_stage()<0);
}
static unsigned wet_samples,feedback_samples;
static float wet_high,measured_fb;
static s1_dry_part *wet_owner;
void host_reverb_lfo_tap(const s1c *e,float ungated,float actual)
{
 (void)ungated;
 if(wet_owner && e==&wet_owner->e){++wet_samples;if(actual>wet_high)wet_high=actual;}
}
void __real_s1u_rvb_run(s1u_rvb*,float,float,float,float*,float*,float);
void __wrap_s1u_rvb_run(s1u_rvb*r,float fb,float l,float rgt,float*ol,float*or_,float sr)
{
 if(wet_owner && r==&wet_owner->e.rv){++feedback_samples;measured_fb=fb;}
 __real_s1u_rvb_run(r,fb,l,rgt,ol,or_,sr);
}
static void edit_a4_size(unsigned owner, unsigned a4, unsigned size, int render_wet)
{
 float old_tune=current->e.t[143],old_room=current->e.t[38];
 if(render_wet)undo_start();
 select_owner(owner,"TUNING");pref_t a=pg.r[cur_page()][0];CHECK(a.kind==R_S1A4&&a.b==owner);
 for(unsigned n=0;n<61&&pref_get(a)+410!=(int)a4;n++)detent(0,pref_get(a)+410<(int)a4?1:-1);
 CHECK(pref_get(a)+410==(int)a4&&current->e.t[143]==(float)a4);
 select_owner(owner,"FX SWITCHES");pref_t r=pg.r[cur_page()][3];
 CHECK(r.kind==R_S1RVFB&&r.b==owner);
 for(unsigned n=0;n<129&&pref_get(r)!=(int)size;n++)detent(3,pref_get(r)<(int)size?1:-1);
 CHECK(pref_get(r)==(int)size&&current->e.t[38]==(float)size/128.f);
 if(!render_wet)return;
 CHECK(undo_commit_stable());
 for(unsigned n=0;undo.cur&&n<16;n++)physical_history(0);
 CHECK(undo.cur==0&&current->e.t[143]==old_tune&&current->e.t[38]==old_room);
 for(unsigned n=0;undo.cur<undo.n&&n<16;n++)physical_history(1);
 CHECK(undo.cur==undo.n&&current->e.t[143]==(float)a4&&current->e.t[38]==(float)size/128.f);
 wet_owner=current;
 s1c_front_set(&wet_owner->e,40,.75f);
 s1c_front_set(&wet_owner->e,37,1.f);
 engine_bass_on(owner,69,0,0);render(1);
 unsigned onset=0;
 for(unsigned i=0;i<S1C_VOICES;i++){const s1c_voice*v=wet_owner->e.v+i;if(v->gate&&v->root==69){CHECK(v->freq==(float)a4);++onset;}}
 CHECK(onset);
 wet_samples=feedback_samples=0;wet_high=0;
 render(640);CHECK(wet_samples&&feedback_samples&&wet_high>.1f&&measured_fb==(float)size/128.f);
 printf("gate=1 owner=%u A4=%u feedback=%.6f applied peak=%.6f\n",owner,a4,measured_fb,wet_high);
 s1c_front_set(&wet_owner->e,37,0.f);render(640);
 wet_samples=0;wet_high=0;render(20);
 CHECK(wet_samples&&wet_high<1.e-8f);
 printf("gate=0 owner=%u applied peak=%g\n",owner,wet_high);
 engine_bass_off(owner);render(16);wet_owner=NULL;
}
static uint32_t physical_pads;
static uint64_t physical_keyboard;
uint32_t plat_arp_input(uint64_t *keyboard)
{
 *keyboard=physical_keyboard;
 return physical_pads;
}
static void gesture(uint32_t pads,uint64_t keyboard)
{
 physical_pads=pads;physical_keyboard=keyboard;
 held_keys=pads|(uint32_t)(keyboard>>5);
 ++now_ms;input();render(4);
}
static unsigned gates_s1(const s1_dry_part *p,unsigned note)
{
 unsigned n=0;
 for(unsigned i=0;i<S1C_VOICES;++i)n+=p->e.v[i].root==(int)note&&p->e.v[i].gate>0.f;
 return n;
}
static void physical_program(unsigned v)
{
 ui.overlay=O_NONE;ui.part=PART_303A+v;set_view(V_SEQ);
 *page_of(V_SEQ)=3;build_pages();smc_peek_cancel();ui.oct=0;
 CHECK(arp_seq_page());
 const uint32_t pad=1u<<WHITE_KEY[0];
 const uint64_t e=UINT64_C(1)<<(52-36), alias=UINT64_C(1)<<(WHITE_KEY[0]+5);
 engine_arp_steps before,after;
 CHECK(engine_arp_steps_read(v,&before)&&before.offset[0]==0 && (before.on_mask&1u));
 uint8_t peer[ARP_BLOCK_BYTES];
 memcpy(peer,proj.sound.v[T_303][ARP_BLOCK_VOICE0+1-v],sizeof peer);
 gesture(0,0);gesture(pad,0); CHECK(arp_held==0);gesture(pad,e);
 CHECK(engine_arp_steps_read(v,&after)&&after.offset[0]==4);
 gesture(pad,0);gesture(pad,alias);
 CHECK(engine_arp_steps_read(v,&after)&&after.offset[0]==-7);
 gesture(0,alias);gesture(0,0);
 CHECK(engine_arp_steps_read(v,&after)&&after.offset[0]==-7&&after.on_mask==before.on_mask&&after.oct_mask==before.oct_mask);
 CHECK(!memcmp(peer,proj.sound.v[T_303][ARP_BLOCK_VOICE0+1-v],sizeof peer));
 for(unsigned i=1;i<ARP_STEPS;++i)CHECK(after.offset[i]==before.offset[i]);
}
static unsigned active_pan_frames, pan_lr_distinct;
static float active_pan_rate,active_pan_amount;
static s1c *watch_panner;
static unsigned pan_bad_receiver;
void __real_s1u_autopan_authored(float*,float,float,float,float,float*,float*);
void __wrap_s1u_autopan_authored(float *ph,float mono,float rate,float amount,float sr,float *l,float *r)
{
 __real_s1u_autopan_authored(ph,mono,rate,amount,sr,l,r);
 if(watch_panner&&ph==&watch_panner->pan_ph&&amount>0.f&&fabsf(mono)>1e-6f) {
#if S1C_CSM
  pan_bad_receiver+=(rate!=watch_panner->c[28]||amount!=watch_panner->c[27]);
#else
  pan_bad_receiver+=(rate!=watch_panner->c[36]||amount!=watch_panner->c[35]);
#endif
  ++active_pan_frames;active_pan_rate=rate;active_pan_amount=amount;
  if(fabsf(*l-*r)>5e-8f)++pan_lr_distinct;
 }
}
static void set_pan_sync(unsigned v,int enabled)
{
 select_owner(v,"TEMPO SYNC");
 CHECK(s1_patch_at(pg.r[cur_page()][0].c)->factory_id==139);
 if(current->e.t[139]!=(float)enabled)detent(0,enabled?1:-1);
 CHECK(current->e.t[139]==(float)enabled);
}
static float pan_word(unsigned v){return s1_pan_rate_hz(s1_pan_rate_word(read137(&proj.sound,v)));}
static void configure_pan(unsigned v)
{
 set_pan_sync(v,0);
 select_owner(v,"AUTO PAN");CHECK(pg.r[cur_page()][0].b==v&&pg.r[cur_page()][1].b==v);
 for(unsigned n=0;n<3u+v;++n)detent(0,1);
 for(unsigned n=0;n<22u+v;++n)detent(1,1);
 const float free_hz=pan_word(v);
 CHECK(free_hz==current->e.t[36]&&free_hz>0.f);
 set_pan_sync(v,1);
 select_owner(v,"AUTO PAN");
 /* Choose a musically useful non-plateau synced value. */
 pref_t r=pg.r[cur_page()][1];
 for(unsigned n=0;n<30&&pref_get(r)!=9;++n)detent(1,pref_get(r)<9?1:-1);
 CHECK(pref_get(r)==9&&pan_word(v)==current->e.t[36]);
 CHECK(read137(&proj.sound,v)[21]==3+v);
}
static void configure_clock(unsigned s)
{
 ui.overlay=O_NONE;ui.part=PART_303A+s;set_view(V_PART);build_pages();visit("ARP PARAM");
 CHECK(engine_arp_source_get(s,ARP_F_SOURCE_RATE)==120&&engine_arp_source_get(s,ARP_F_SOURCE_FACTOR)==9);
 CHECK(current->e.t[64]==200.f);
 const float original_pan=pan_word(s);
 detent(1,-1);render(1); /* Factor only must not author tone BPM. */
 CHECK(engine_arp_source_get(s,ARP_F_SOURCE_RATE)==120&&engine_arp_source_get(s,ARP_F_SOURCE_FACTOR)==10);
 CHECK(read137(&proj.sound,s)[8]==0&&current->e.t[64]==200.f&&pan_word(s)==original_pan);
 undo_start();const unsigned before_writes=store_writes;
 detent(0,1);render(1);
 CHECK(engine_arp_source_get(s,ARP_F_SOURCE_RATE)==121&&engine_arp_source_get(s,ARP_F_SOURCE_FACTOR)==10);
 CHECK(read137(&proj.sound,s)[8]==0x80&&current->e.t[64]==121.f);
 const float authored_pan=pan_word(s);
 CHECK(authored_pan!=original_pan&&authored_pan==current->e.t[36]&&store_writes==before_writes);
 CHECK(undo_commit_stable());physical_history(0);
 CHECK(read137(&proj.sound,s)[8]==0&&current->e.t[64]==200.f&&current->e.t[36]==original_pan&&pan_word(s)==original_pan);
 physical_history(1);CHECK(read137(&proj.sound,s)[8]==0x80&&current->e.t[64]==121.f&&pan_word(s)==authored_pan);
 const uint8_t transport=seq.bpm;seq.bpm=96;render(3);
 CHECK(current->e.t[64]==121.f&&pan_word(s)==authored_pan&&store_writes==before_writes);
 seq.bpm=transport;render(1);
}
static engine_arp_settings joined_arp(unsigned owner)
{ engine_arp_settings v; CHECK(engine_arp_settings_read(owner,&v));return v; }
static void cfg_arp(unsigned owner,unsigned field,int value)
{ CHECK(engine_arp_set(owner,joined_arp(owner).revision,field,value)==S1_EDIT_QUEUED);render(1); }
static void next_frame(void){int32_t lr[2]={0};engine_render(lr,1);}
static void joined_key(unsigned owner,unsigned note,unsigned vel)
{unsigned ch=seq.ch[TRK_BASS0+owner];host_midi_put((vel?9u:8u)|((vel?0x90u:0x80u)|ch)<<8|note<<16|vel<<24);}
static unsigned gates_juno(const juno6_t*p,unsigned note)
{ unsigned n=0;for(unsigned i=0;i<DNBX_VOICES;i++)n+=p->core.voice[i].gate && p->core.voice[i].note==note;return n; }
extern const classic_mode_i *sj6_classic_i_probe(unsigned);
static void configure_classic_mode(unsigned v,unsigned mode)
{
 select_owner(v,"CHORUS");
 CHECK(pg.r[cur_page()][0].c==SJ6_CHORUS_I&&pg.r[cur_page()][1].c==SJ6_CHORUS_II);
 const char *page=mode==2?"CHORUS II":"CHORUS BOTH";
 if(!pref_get(pg.r[cur_page()][1])){detent(1,1);render(2);}
 select_owner(v,"CHORUS");
 if((pref_get(pg.r[cur_page()][0])!=0)!=(mode==3)){detent(0,mode==3?1:-1);render(2);}
 select_owner(v,"CHORUS");
 CHECK(pref_get(pg.r[cur_page()][0])==(mode==3)&&pref_get(pg.r[cur_page()][1])==1);
 CHECK(pg.r[cur_page()][2].kind==R_NONE&&pg.r[cur_page()][3].kind==R_NONE);
 select_owner(v,page);
 CHECK(pg.r[cur_page()][3].kind==R_NONE&&count_page(page)==1&&count_page("CHORUS I")==0);
 const unsigned writes=store_writes;
 for(unsigned k=0;k<3;++k) {
    const pref_t x=pg.r[cur_page()][k];
    CHECK(x.kind==R_SJ6CLASSIC_I&&x.b==v&&x.c==k);
    const int raw=pref_get(x);
    CHECK(read137(&proj.sound,v)[SJ6_CLASSIC_I_BYTE+k]==SJ6_CLASSIC_I_DEFAULT);
    detent(k,1);CHECK(read137(&proj.sound,v)[SJ6_CLASSIC_I_BYTE+k]==raw+2);
 }
 const classic_mode_i *p=sj6_classic_i_probe(v);
 CHECK(p&&p->started&&p->mode_ii==mode&&store_writes==writes);
 CHECK(p->p[0].pending>0.f&&p->p[1].pending>0.f&&p->p[3].pending>0.f);
}
static void census_owner(unsigned owner,int is_s1,unsigned mode)
{
 select_owner(owner,"SOUND");
 CHECK(pg.n>0&&pg.n<=MAXPAGES&&4u*pg.n+pg.nmore<=PAGE_REFS);
 const unsigned count=pg.n;
 unsigned link=0,lfo2_rate=0;
 static const uint8_t common[]={PT_SOUND,PT_ARPEGGIATOR,PT_FILTER,PT_FILTER_ADSR,PT_AMP_ADSR,PT_WHEELS};
 static const uint8_t s1[]={PT_TUNING,PT_FX_BYPASS,PT_CRUSH_MOD,PT_AUTO_PAN,PT_TEMPO_SYNC,PT_ARP_PARAM,PT_DELAY};
 static const uint8_t sj6[]={PT_TUNING,PT_FILTER_MOD,PT_SJ6_DELAY,PT_SJ6_DELAY_CLOCK,PT_CHORUS,PT_VIBRATO,PT_LFO2_PWM};
 const uint8_t *expected=is_s1?s1:sj6;
 const size_t len=is_s1?sizeof s1:sizeof sj6;
 for(unsigned i=0;i<sizeof common+len;i++) {
   const uint8_t target=i<sizeof common?common[i]:expected[i-sizeof common];
   unsigned seen=0;for(unsigned row=0;row<count;++row)seen+=pg.title[row]==target;
   CHECK(seen==1);
 }
 if(!is_s1) {
    CHECK(count_page(mode==2?"CHORUS II":"CHORUS BOTH")==1);
    CHECK(count_page(mode==2?"CHORUS BOTH":"CHORUS II")==0&&count_page("CHORUS I")==0);
 }
 for(unsigned row=0;row<count;++row)for(unsigned k=0;k<4;++k){
   const pref_t x=pg.r[row][k];
   if(x.kind==R_S1LINK || (x.kind==R_SJ6KNOB&&x.c==SJ6_FILTER_LINK)){
      CHECK(x.b==owner&&pg.title[row]==PT_FILTER&&k==3);
      CHECK(is_s1?(x.kind==R_S1LINK):(x.kind==R_SJ6KNOB));
      ++link;
   }
   if(x.kind==R_SJ6KNOB&&x.c==SJ6_LFO2_RATE)++lfo2_rate;
 }
 CHECK(link==1&&lfo2_rate==(is_s1?0u:2u));
 ui.overlay=O_NONE;ui.part=PART_303A+owner;set_view(V_SEQ);smc.view=V_SEQ;build_pages();visit("ARP SEQ");
 CHECK(arp_seq_page()&&pg.r[cur_page()][2].kind==R_ARP&&pg.r[cur_page()][2].b==owner&&pg.r[cur_page()][2].a==ARP_F_GATE);
 printf("pages owner=%u engine=%s part=%u/%u seq=%u LINK=%u existing-LFO2-rate-shorthand=%u\n",
     owner,is_s1?"S1":"SJ6",count,MAXPAGES,pg.n,link,lfo2_rate);
}
static void arp_edits_and_reload(unsigned s,unsigned j)
{
 CHECK(engine_bass_bank(s)==ORIGINAL_SOUND_SYNTHONE && engine_bass_bank(j)==ORIGINAL_SOUND_JUNO);
 select_owner(s,"TUNING");
 const pref_t tr=pg.r[cur_page()][1];
 CHECK(tr.kind==R_S1TRANS);
 detent(1,1);detent(1,1);CHECK(pref_get(tr)==26&&current->e.t[148]==2.f);
 CHECK(s1_trans_st(read137(&proj.sound,s)[15])==2);
 cfg_arp(j,ARP_F_ON,1);cfg_arp(j,ARP_F_MODE,ARP_SEQ);cfg_arp(j,ARP_F_DIVISION,1);cfg_arp(j,ARP_F_GATE,1);
 cfg_arp(s,ARP_F_ON,1);cfg_arp(s,ARP_F_MODE,ARP_SEQ);
 const unsigned classic_mode=2+s;
 configure_classic_mode(j,classic_mode);census_owner(s,1,0);census_owner(j,0,classic_mode);
 physical_program(j);physical_program(s);
 configure_pan(s);
 select_owner(s,"DELAY");CHECK(count_page("DELAY")==1);
 const pref_t feedback=pg.r[cur_page()][0];
 CHECK(feedback.kind==R_S1DLFB&&feedback.b==s&&pref_get(feedback)==14);
 for(unsigned i=1;i<4;i++)CHECK(pg.r[cur_page()][i].kind==R_NONE);
 CHECK(current->e.delay_feedback_direct&&current->e.t[42]==.1f);
 undo_start();const unsigned fb_writes=store_writes;
 const int hold_len=current->e.dl[0].len,hold_dec=current->e.ddec;
 const void *hold_buf=current->e.dl[0].buf;
 detent(0,1);render(1);
 CHECK(read137(&proj.sound,s)[S1_FB_BYTE]==16&&read137(&proj.sound,j)[S1_FB_BYTE]==0);
 CHECK(current->e.t[42]==.9f*15.f/127.f&&current->e.dl[0].len==hold_len&&current->e.ddec==hold_dec&&current->e.dl[0].buf==hold_buf);
 CHECK(undo_commit_stable());
 physical_history(0);CHECK(current->e.t[42]==.1f&&current->e.delay_feedback_direct);
 physical_history(1);CHECK(current->e.t[42]==.9f*15.f/127.f&&store_writes==fb_writes);
 project_capture_stage();slot137(&project_stage.sound,s)[S1_FB_BYTE]=129;CHECK(project_validate_stage()<0);
 configure_clock(s);
 CHECK(pan_word(s)==current->e.t[36]);
 const float saved_pan=pan_word(s);
 ui.overlay=O_NONE;ui.part=PART_303A+j;set_view(V_SEQ);smc.view=V_SEQ;build_pages();visit("ARP SEQ");
 CHECK(arp_seq_page()&&pg.r[cur_page()][2].a==ARP_F_GATE);
 smc_peek_cancel();smc_page_set(cur_page());detent(2,-1);render(1);
 CHECK(joined_arp(j).gate==0&&read137(&proj.sound,j)[13]==1);
 for(unsigned v=0;v<NBASS;v++){
   project_capture_stage();slot137(&project_stage.sound,v)[13]=2;CHECK(project_validate_stage()<0);
   project_capture_stage();slot137(&project_stage.sound,v)[15]=49;CHECK(project_validate_stage()<0);
 }
 witness=proj;
 CHECK(!project_save()&&!project_load_stage());project_commit_stage();stabilize();
 CHECK(!memcmp(&witness.sound,&proj.sound,sizeof proj.sound));
 CHECK(read137(&proj.sound,j)[14]==5&&read137(&proj.sound,j)[13]==1);
 CHECK(read137(&proj.sound,s)[3]==2&&s1_a4_hz(read137(&proj.sound,s)[6])==449u+4u*s&&read137(&proj.sound,s)[15]==s1_trans_code(2));
 CHECK(last_juno->pb_down==48&&last_juno->pb_up==47&&last_juno->mod_destination==5);
 CHECK(current->e.t[148]==2.f&&current->e.t[143]==449u+4u*s);
 CHECK(pan_word(s)==saved_pan&&current->e.t[36]==saved_pan&&current->e.t[35]==(3.f+s)/127.f);
 CHECK(read137(&proj.sound,s)[S1_FB_BYTE]==16&&current->e.delay_feedback_direct&&current->e.t[42]==.9f*15.f/127.f);
 CHECK(engine_arp_source_get(s,ARP_F_SOURCE_RATE)==121&&engine_arp_source_get(s,ARP_F_SOURCE_FACTOR)==10);
 CHECK(current->e.t[64]==121.f && read137(&proj.sound,s)[8]==0x80);
 CHECK(!sj6_classic_i_probe(s)&&sj6_classic_i_probe(j)&&sj6_classic_i_probe(j)->mode_ii==classic_mode);
 for(unsigned k=0;k<3;++k)CHECK(read137(&proj.sound,j)[SJ6_CLASSIC_I_BYTE+k]!=SJ6_CLASSIC_I_DEFAULT&&read137(&proj.sound,s)[SJ6_CLASSIC_I_BYTE+k]==0);

 joined_key(j,60,100);next_frame();
 CHECK(gates_juno(last_juno,53)==1&&!gates_s1(current,53));
 next_frame();CHECK(!gates_juno(last_juno,53));
 joined_key(j,60,0);next_frame();
 joined_key(s,60,100);next_frame();CHECK(gates_s1(current,53)==1);
 float hz=(449.f+4.f*s)*exp2f((55.f-69.f)/12.f);
 for(unsigned i=0;i<S1C_VOICES;i++)if(current->e.v[i].root==53&&current->e.v[i].gate)CHECK(fabsf(current->e.v[i].freq-hz)<.001f);
 next_frame();CHECK(gates_s1(current,53)==1);
 active_pan_frames=pan_lr_distinct=pan_bad_receiver=0;
 watch_panner=&current->e;
 for(unsigned i=0;i<1536;++i)next_frame();
 CHECK(active_pan_frames>0&&pan_lr_distinct>0);
 CHECK(!pan_bad_receiver&&active_pan_rate>0.f&&active_pan_amount>0.f);
 watch_panner=NULL;
 printf("pan owner=%u rate=%g amount=%g rendered=%u differing LR=%u\n",s,active_pan_rate,active_pan_amount,active_pan_frames,pan_lr_distinct);
 joined_key(s,60,0);next_frame();CHECK(!gates_s1(current,53));
 CHECK(!memcmp(&witness.sound,&proj.sound,sizeof proj.sound));
}

static void both_order_random2(unsigned s,unsigned j)
{
 const unsigned n=store_writes;
 const sound_t before=proj.sound;
 select_owner(j,"ARPEGGIATOR");const pref_t mode=pg.r[cur_page()][2];
 CHECK(mode.kind==R_ARP&&mode.a==ARP_F_MODE&&mode.b==j);
 CHECK(pref_get(mode)==ARP_SEQ&&pref_desc(mode)->max==ARP_SAGA_RANDOM2);
 detent(2,1);render(1);CHECK(pref_get(mode)==ARP_UPDN_REPEAT);
 detent(2,1);render(1);CHECK(pref_get(mode)==ARP_SAGA_RANDOM2);
 char value[24],unit[8];pref_value(mode,value,unit);CHECK(!strcmp(value,"RANDOM2"));
 const uint8_t a=read137(&proj.sound,s)[26];CHECK(read137(&proj.sound,j)[26]==2&&a==0);
 CHECK(read137(&proj.sound,j)[13]==1&&store_writes==n);
 for(unsigned v=0;v<2;v++) {
  const uint8_t *old=read137(&before,v),*now=read137(&proj.sound,v);
  for(unsigned k=20;k<32;k++)if(k!=26)CHECK(old[k]==now[k]);
 }
 select_owner(s,"ARPEGGIATOR");CHECK(pref_desc(pg.r[cur_page()][2])->max==ARP_SEQ);
 CHECK(engine_arp_set(s,joined_arp(s).revision,ARP_F_MODE,ARP_SAGA_RANDOM2)==S1_EDIT_REJECTED);
 project_capture_stage();slot137(&project_stage.sound,j)[26]=1;CHECK(project_validate_stage()<0);
 project_capture_stage();slot137(&project_stage.sound,j)[26]=3;CHECK(project_validate_stage()<0);
 witness=proj;CHECK(!project_save()&&!project_load_stage());project_commit_stage();stabilize();
 CHECK(!memcmp(&witness.sound,&proj.sound,sizeof proj.sound)&&joined_arp(j).mode==ARP_SAGA_RANDOM2&&joined_arp(j).gate==0);
}
static void owner_roundtrip(unsigned s)
{
 unsigned j=1-s;
 arrange(s);
 s1_dry_part *synth=current;parts[s]=synth;
 juno6_t *juno=last_juno;
 CHECK(synth&&juno);
 knob_mod(s,2);
 edit_a4_size(s,449u+4u*s,97u+11u*s,1);
 select_owner(s,"CRUSH MOD");
 CHECK(count_page("CRUSH MOD")==1&&pg.r[cur_page()][0].b==s);
 CHECK(pref_get(pg.r[cur_page()][0])==0);
 undo_start();
 for(unsigned i=0;i<2+s;++i){detent(0,1);render(3);CHECK(undo_commit_stable());}
 CHECK(read137(&proj.sound,s)[20]==2+s&&current->e.t[58]==2.f+s);
 CHECK(read137(&proj.sound,j)[20]==0);
 physical_history(0);render(4);CHECK(read137(&proj.sound,s)[20]==1+s&&current->e.t[58]==1.f+s);
 physical_history(1);render(4);CHECK(read137(&proj.sound,s)[20]==2+s&&current->e.t[58]==2.f+s);
 project_capture_stage();slot137(&project_stage.sound,s)[20]=4;CHECK(project_validate_stage()<0);
 select_owner(s,"WHEELS");
 CHECK(count_page("FILTER MOD")==0);
 detent(0,-1);detent(1,1);render(4);
 CHECK(s1_pb_decode(read137(&proj.sound,s)[4])==11);
 CHECK(s1_pb_decode(read137(&proj.sound,s)[5])==13);
 select_owner(s,"FX SWITCHES");
 CHECK(s1_patch_at(pg.r[cur_page()][2].c)->factory_id==51);
 detent(2,1);render(4);CHECK(read_slot(s,51).position==1);
 select_owner(j,"FILTER MOD");
 CHECK(count_page("WHEELS")==1 && count_page("PITCH MOD")==0);
 CHECK(pg.r[cur_page()][0].c==SJ6_FILTER_VELOCITY && pg.r[cur_page()][0].b==j);
 for(unsigned n=0;n<7;n++){detent(0,1);render(4);}
 CHECK(juno->filter_velocity_position==7 && synth->mod_wheel_route==2);
 edit_juno(j,s,juno);
 joined_selector(j,juno);
#if X0X_SJ6_DELAY_WITNESS
 select_owner(j,"DELAY");CHECK(count_page("DELAY")==1);
 CHECK(pg.r[cur_page()][0].c==SJ6_DELAY24&&pg.r[cur_page()][1].c==SJ6_DELAY25);
 CHECK(pg.r[cur_page()][2].c==SJ6_DELAY26&&pg.r[cur_page()][3].c==SJ6_DELAY27);
 detent(0,1);render(3);
 for(unsigned i=0;i<4;++i){detent(1,1);detent(2,1);detent(3,1);render(3);}
 select_owner(j,"DELAY CLOCK");CHECK(pref_get(pg.r[cur_page()][0])==0);
 detent(0,1);render(3);
 const uint8_t *delay=read137(&proj.sound,j)+16;
 CHECK(delay[0]==3&&delay[1]==4&&delay[2]==4&&delay[3]==4);
 CHECK(juno->delay24.sync&&juno->delay24.time_raw==4&&!juno->delay24.clock_refused);
 CHECK(read137(&proj.sound,s)[16]==0&&read137(&proj.sound,s)[20]==2+s);
#endif
 const uint8_t *a=read137(&proj.sound,s),*b=read137(&proj.sound,j);
 CHECK(a[2]==0&&a[3]==2&&b[2]==7&&b[3]==0&&b[4]==0&&b[5]==0&&b[10]==48&&b[11]==47);
 project_capture_stage();slot137(&project_stage.sound,j)[10]=49;CHECK(project_validate_stage()<0);
 project_capture_stage();slot137(&project_stage.sound,j)[11]=255;CHECK(project_validate_stage()<0);
 witness=proj;
 CHECK(!project_save()&&!project_load_stage());project_commit_stage();stabilize();
 CHECK(!memcmp(&witness.sound,&proj.sound,sizeof proj.sound));
 CHECK(last_juno->pb_down==48 && last_juno->pb_up==47);
 CHECK(last_juno->mod_destination==5 && !last_juno->mod_wheel_lfo1 && !last_juno->mod_wheel_cutoff_hz && !last_juno->alt3.last_wheel_code);
#if X0X_SJ6_DELAY_WITNESS
 CHECK(delay[0]==3&&delay[1]==4&&delay[2]==4&&delay[3]==4);
 CHECK(last_juno->delay24.sync&&last_juno->delay24.time_raw==4&&!last_juno->delay24.clock_refused);
#endif
 CHECK(read137(&proj.sound,s)[20]==2+s&&current->e.t[58]==2.f+s);
 CHECK(current->mod_wheel_route==2 && last_juno->filter_velocity_position==7 && read_slot(s,51).position==1);
 CHECK(current->e.t[143]==449u+4u*s && current->e.t[38]==(97u+11u*s)/128.f);
 CHECK(s1_pb_decode(read137(&proj.sound,s)[4])==11 && s1_pb_decode(read137(&proj.sound,s)[5])==13);
 /* No aliasing on an actual bank swap of the exact same owner. */
 select_patch(j); pb_read(j,-12,12);
 goto_pb(j);detent(0,-1);detent(1,1);render(4);pb_read(j,-13,13);
 CHECK(read137(&proj.sound,j)[2]==7 && read137(&proj.sound,j)[10]==48 && read137(&proj.sound,j)[11]==47 && read137(&proj.sound,j)[14]==5);
 knob_mod(j,1); CHECK(read137(&proj.sound,j)[3]==1 && read137(&proj.sound,j)[14]==5);
 edit_a4_size(j,432,41,0);
 CHECK(!engine_bass_select_preset(j,ORIGINAL_SOUND_JUNO,0,&proj.sound));
 for(unsigned n=0;n<1024 && engine_bass_preset_pending(j);++n)render(1);
 CHECK(engine_bass_bank(j)==ORIGINAL_SOUND_JUNO && last_juno->pb_down==48 && last_juno->pb_up==47);
 CHECK(read137(&proj.sound,j)[3]==1 && read137(&proj.sound,j)[14]==5 && last_juno->mod_destination==5);
 CHECK(s1_a4_hz(read137(&proj.sound,j)[6])==432&&s1_rvfb_pos(read137(&proj.sound,j)[12])==41);
 CHECK(read137(&proj.sound,j)[4]==12 && read137(&proj.sound,j)[5]==13);
 CHECK(!project_save()&&!project_load_stage());project_commit_stage();stabilize();
 CHECK(last_juno->pb_down==48 && last_juno->pb_up==47);
 CHECK(last_juno->mod_destination==5 && !last_juno->mod_wheel_lfo1 && !last_juno->mod_wheel_cutoff_hz && !last_juno->alt3.last_wheel_code);
 select_patch(j);
 CHECK(current->e.t[143]==432.f&&current->e.t[38]==41.f/128.f&&current->mod_wheel_route==1);
 CHECK(!engine_bass_select_preset(j,ORIGINAL_SOUND_JUNO,0,&proj.sound));
 for(unsigned n=0;n<1024 && engine_bass_preset_pending(j);++n)render(1);
 CHECK(engine_bass_bank(j)==ORIGINAL_SOUND_JUNO&&last_juno->mod_destination==5&&last_juno->pb_down==48&&last_juno->pb_up==47);
 current=parts[s];
 arp_edits_and_reload(s,j);
 both_order_random2(s,j);
}

int main(void)
{
 setup(V_PART,PART_303A);proj.set.bt|=PROJ_BT_CLK_IN;
 for(unsigned mag=0;mag<=48;++mag)CHECK(sj6_pb_decode(sj6_pb_encode(mag))==mag);
 CHECK(!engine_bass_select_preset(0,ORIGINAL_SOUND_JUNO,0,&proj.sound));
 for(unsigned n=0;n<1024 && engine_bass_preset_pending(0);++n)render(1);
 CHECK(last_juno && !last_juno->pb_typed);
 ui.part=PART_303A;set_view(V_PART);build_pages();
 CHECK(!count_page("WHEELS") && !sj6_typed_pb_edit(0,0,0));
 owner_roundtrip(0);
 owner_roundtrip(1);
 project_capture_stage();uint8_t h[32];project_file_header(h);CHECK(!project_file_validate(h));
 FILE *f=fopen("out/mixed-media.bin","wb");CHECK(f);
 CHECK(fwrite(h,sizeof h,1,f)==1&&fwrite(&project_stage,sizeof project_stage,1,f)==1&&
       fwrite(store,sizeof store,1,f)==1&&fwrite(store_len,sizeof store_len,1,f)==1);
 CHECK(!fclose(f));
 /* Same-revision split generations: range bytes join the full Sound digest. */
 uint8_t old_sound_obj[PLAT_STORE_MAX];
 memcpy(old_sound_obj,store[OBJ_SOUND],sizeof old_sound_obj);
 select_owner(0,"WHEELS");detent(1,-1);render(3);
 CHECK(last_juno->pb_up==46 && !project_save());
 memcpy(store[OBJ_SOUND],old_sound_obj,sizeof old_sound_obj);
 unsigned writes=store_writes;
 CHECK(!project_load());stabilize();
 select_owner(0,"WHEELS");
 CHECK(pref_read_only(pg.r[cur_page()][1]) && last_juno->pb_up==47);
 detent(1,-1);render(3);
 CHECK(last_juno->pb_up==47 && project_save()==-9 && store_writes==writes);
 printf("PASS full typed wheel/A4/RV/clock/gate/transpose/CRUSH/Delay/AutoPan/ID64/S1 Feedback/Classic II/Both/RANDOM2 mixed project with S1 MOD/PB/ID51 + SJ6 ID137, real held pitch, Undo/Redo and staged recall: %u checks\n",checks);
}
