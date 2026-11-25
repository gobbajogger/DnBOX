#define MIXED_ONLY 1
#include "s1_pb_mod_ui.c"
#include "dsp/juno6.h"
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
static void owner_roundtrip(unsigned s)
{
 unsigned j=1-s;
 arrange(s);
 s1_dry_part *synth=current;
 juno6_t *juno=last_juno;
 CHECK(synth&&juno);
 knob_mod(s,2);
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
 const uint8_t *a=read137(&proj.sound,s),*b=read137(&proj.sound,j);
 CHECK(a[2]==0&&a[3]==2&&b[2]==7&&b[3]==0&&b[4]==0&&b[5]==0&&b[10]==48&&b[11]==47);
 project_capture_stage();slot137(&project_stage.sound,j)[10]=49;CHECK(project_validate_stage()<0);
 project_capture_stage();slot137(&project_stage.sound,j)[11]=255;CHECK(project_validate_stage()<0);
 witness=proj;
 CHECK(!project_save()&&!project_load_stage());project_commit_stage();stabilize();
 CHECK(!memcmp(&witness.sound,&proj.sound,sizeof proj.sound));
 CHECK(last_juno->pb_down==48 && last_juno->pb_up==47);
 CHECK(last_juno->mod_destination==5 && !last_juno->mod_wheel_lfo1 && !last_juno->mod_wheel_cutoff_hz && !last_juno->alt3.last_wheel_code);
 CHECK(current->mod_wheel_route==2 && last_juno->filter_velocity_position==7 && read_slot(s,51).position==1);
 CHECK(s1_pb_decode(read137(&proj.sound,s)[4])==11 && s1_pb_decode(read137(&proj.sound,s)[5])==13);
 /* No aliasing on an actual bank swap of the exact same owner. */
 select_patch(j); pb_read(j,-12,12);
 goto_pb(j);detent(0,-1);detent(1,1);render(4);pb_read(j,-13,13);
 CHECK(read137(&proj.sound,j)[2]==7 && read137(&proj.sound,j)[10]==48 && read137(&proj.sound,j)[11]==47 && read137(&proj.sound,j)[14]==5);
 knob_mod(j,1); CHECK(read137(&proj.sound,j)[3]==1 && read137(&proj.sound,j)[14]==5);
 CHECK(!engine_bass_select_preset(j,ORIGINAL_SOUND_JUNO,0,&proj.sound));
 for(unsigned n=0;n<1024 && engine_bass_preset_pending(j);++n)render(1);
 CHECK(engine_bass_bank(j)==ORIGINAL_SOUND_JUNO && last_juno->pb_down==48 && last_juno->pb_up==47);
 CHECK(read137(&proj.sound,j)[3]==1 && read137(&proj.sound,j)[14]==5 && last_juno->mod_destination==5);
 CHECK(read137(&proj.sound,j)[4]==12 && read137(&proj.sound,j)[5]==13);
 CHECK(!project_save()&&!project_load_stage());project_commit_stage();stabilize();
 CHECK(last_juno->pb_down==48 && last_juno->pb_up==47);
 CHECK(last_juno->mod_destination==5 && !last_juno->mod_wheel_lfo1 && !last_juno->mod_wheel_cutoff_hz && !last_juno->alt3.last_wheel_code);
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
 printf("PASS mixed SJ6 MOD six + PB WHEELS ±48 with S1 MOD/PB/ID51 + SJ6 ID137, real held pitch, Undo/Redo and staged recall: %u checks\n",checks);
}
