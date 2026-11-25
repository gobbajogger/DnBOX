/* SPDX-License-Identifier: GPL-3.0-only */
#include "synth_init_test_helpers.h"
#include "app/s1c_part.h"
#include "dsp/s1c/s1c_front.h"
#include <math.h>
static struct { unsigned frames; int wheel[2]; uint16_t wheel_mod_seq; } fm1_in;
static int clamp(int v,int lo,int hi) { return v<lo?lo:v>hi?hi:v; }
#include "mod-wheels.inc"
static s1_dry_part *current,*parts[2];
void __real_s1_part_init(s1_dry_part *);
void __wrap_s1_part_init(s1_dry_part *p) { __real_s1_part_init(p); current=p; }
enum { CUT=16, BEND=31, LFO1=15,LFO2=47 };
static void adc(int raw)
{
    fm1_in.wheel[0]=500;
    fm1_in.wheel[1]=raw;
    ++fm1_in.wheel_mod_seq; fm1_in.frames=301;
    smk_wheels(); render(1);
}
static void midi(unsigned owner,unsigned cc)
{
    unsigned ch=(unsigned)seq.ch[TRK_BASS0+owner];
    host_midi_put(0xbu | ((0xb0u|ch)<<8) | 1u<<16 | cc<<24);
    render(1);
    /* Consume the genuine external-MIDI owner/recordable guard. */
    smk_wheels();
}
static float target(unsigned cc,unsigned bpm)
{
    const double lo=1./1920., hi=80.;
    if (!bpm) return (float)(lo + pow((double)cc/127.,2.5)*(hi-lo));
    const double beats[]={32,24,16,12,8,4,4/1.5,2,2/1.5,1,1/1.5,.5,.5/1.5,.25,.25/1.5,.125,.125/1.5,.0625,.0625/1.5};
    double hz=bpm/60./beats[cc*18/127];
    if (hz>80. || (bpm==200 && cc==127)) hz=bpm/60./beats[17];
    return (float)hz;
}
#include "app/sj6_137_host_witness.h"
extern int s1_typed_mod_choose(unsigned,unsigned);
int sj6_137_fixture_lens;

static void goto_pitch(unsigned owner)
{
 ui.overlay=O_NONE;ui.part=PART_303A+owner;set_view(V_PART);build_pages();
 visit("PITCH MOD");
 pref_t r=pg.r[cur_page()][3];
 CHECK(r.kind==R_S1MODWHEEL && r.b==owner && !pref_read_only(r));
 CHECK(pg.r[cur_page()][0].c==S1_DETUNE_LFO && pg.r[cur_page()][1].c==S1_PITCH_LFO &&
       pg.r[cur_page()][2].c==S1_TREMOLO_LFO);
}
static void display_route(unsigned route)
{
 static const char *const names[]={"CUTOFF","LFO1 RATE","LFO2 RATE"};
 char num[24],unit[8];
 const pref_t r=pg.r[cur_page()][3];pref_value(r,num,unit);
 CHECK(pref_get(r)==(int)route && !strcmp(pref_desc(r)->name,"MOD WHEEL"));
 CHECK(!strcmp(num,names[route]) && !*unit);
 CHECK(tw(&FONT_XS,num)<=54);
}
static void knob_mod(unsigned owner,int value)
{
 goto_pitch(owner);
 int before=pref_get(pg.r[cur_page()][3]);
 for(unsigned n=0;before!=value && n<3;++n) {
  detent(3,before<value?1:-1);
  before=pref_get(pg.r[cur_page()][3]);
 }
 display_route(value);
}
static void physical_history(int redo)
{
 held_btn=1u<<B_HOME;now_ms++;input();
 held_btn|=1u<<(redo?B_PLAY:B_REC);now_ms++;input();
 held_btn&=~(1u<<(redo?B_PLAY:B_REC));now_ms++;input();
 held_btn=0;now_ms++;input();render(3);
}
static void pb_adc(int pitch)
{
 fm1_in.wheel[0]=pitch; fm1_in.wheel[1]=615; fm1_in.frames=301;
 smk_wheels();render(4);
}
static void pb_midi(unsigned owner,unsigned raw)
{
 unsigned ch=seq.ch[TRK_BASS0+owner];
 host_midi_put(0xeu | ((0xe0u|ch)<<8) | ((raw&127u)<<16) | (raw>>7)<<24);
 render(1);
}
static void pb_read(unsigned owner,int lo,int hi)
{
 CHECK(read_slot(owner,141).physical==lo && read_slot(owner,142).physical==hi);
 CHECK(read_slot(owner,141).max_position==24 && read_slot(owner,142).max_position==24);
}
static void pb_target(unsigned owner,unsigned raw,float lo,float hi)
{
 s1_dry_part *p=parts[owner];
 float st=raw<8192?lo*(8191.f-raw)/8191.f:hi*(raw-8192.f)/8192.f;
 CHECK(p->bend_raw==raw && fabsf(p->e.t[BEND]/p->detm-exp2f(st/12.f))<.000004f);
}
static void goto_pb(unsigned owner)
{
 ui.overlay=O_NONE;ui.part=PART_303A+owner;set_view(V_PART);build_pages();
 visit("WHEELS");
 CHECK(pg.r[cur_page()][0].b==owner && pg.r[cur_page()][1].b==owner);
 CHECK(pg.r[cur_page()][0].c==S1_BEND_LO && pg.r[cur_page()][1].c==S1_BEND_HI);
}
static void pb_witness(void)
{
 for(unsigned pos=0;pos<=24;++pos) CHECK(s1_pb_decode(s1_pb_encode(pos))==pos);
 undo_start();
 for(unsigned owner=0;owner<2;++owner) {
   const unsigned peer=1-owner;
   goto_pb(owner);pb_read(owner,-12,12);
   const unsigned mod=read137(&proj.sound,owner)[3], v137=read137(&proj.sound,owner)[2];
   detent(0,-127);render(1);pb_read(owner,-24,12);
   detent(1,-127);render(1);pb_read(owner,-24,0);
   detent(1,1);render(1);pb_read(owner,-24,1);
   CHECK(read137(&proj.sound,owner)[3]==mod && read137(&proj.sound,owner)[2]==v137);
   engine_bass_on(owner,48,0,0);render(4);
   float peer_target=parts[peer]->e.t[BEND];
   pb_adc(500);pb_adc(250);pb_target(owner,0,-24,1);
   ui.part=PART_303A+peer;pb_adc(750);pb_target(owner,16383,-24,1);
   CHECK(peer_target==parts[peer]->e.t[BEND]);
   pb_adc(500);pb_target(owner,8192,-24,1);
   const unsigned witness[]={0,1,8190,8191,8192,8193,16382,16383};
   for(unsigned i=0;i<sizeof witness/sizeof witness[0];++i) {
      pb_midi(owner,witness[i]);pb_target(owner,witness[i],-24,1);
      CHECK(parts[peer]->e.t[BEND]==peer_target);
   }
   goto_pb(owner);detent(1,1);render(1);pb_read(owner,-24,2);
   pb_target(owner,16383,-24,2);
   if (owner) { detent(0,1);render(1); detent(1,1);render(1); pb_read(owner,-23,3); }
   pb_midi(owner,8192);engine_bass_off(owner);render(4);
 }
 CHECK(undo_commit_stable());
 /* Multiple input events may form discrete history entries; step to the actual baseline. */
 for(unsigned n=0;(undo.cur>0) && n<16;++n) physical_history(0);
 CHECK(undo.cur==0);
 for(unsigned owner=0;owner<2;++owner)pb_read(owner,-12,12);
 for(unsigned n=0;undo.cur<undo.n && n<16;++n)physical_history(1);
 CHECK(undo.cur==undo.n);
 for(unsigned owner=0;owner<2;++owner)pb_read(owner,-24+(int)owner,2+(int)owner);
 project_capture_stage();
 slot137(&project_stage.sound,0)[S1_PB_DN_BYTE]=25;CHECK(project_validate_stage()<0);
 project_capture_stage();
 slot137(&project_stage.sound,1)[S1_PB_UP_BYTE]=255;CHECK(project_validate_stage()<0);
 project_capture_stage();
 slot137(&project_stage.sound,1)[6]=1;CHECK(project_validate_stage()<0);
 CHECK(!project_save()&&!project_load_stage());project_commit_stage();stabilize();undo_start();
 for(unsigned owner=0;owner<2;++owner) {
   pb_read(owner,-24+(int)owner,2+(int)owner);pb_target(owner,8192,-24+(int)owner,2+(int)owner);
 }
}
static unsigned all_bindings(unsigned owner)
{
 ui.overlay=O_NONE;ui.part=PART_303A+owner;set_view(V_PART);build_pages();
 unsigned links=0,mods=0;
 for(int p=0;p<pg.n;++p)for(unsigned c=0;c<4;++c) {
  pref_t r=pg.r[p][c];
  if(r.kind==R_S1LINK){CHECK(c==3 && r.b==owner && pg.title[p]==PT_FILTER);++links;}
  if(r.kind==R_S1MODWHEEL){CHECK(c==3 && r.b==owner && pg.title[p]==PT_PITCH_MOD);++mods;}
 }
 CHECK(links==1);
 return mods;
}
static project_t witness;
static void fresh(void)
{
 project_reset_stage();
 project_stage.set.snd_rev=REV137;
 for(unsigned i=0;i<2;i++) {
  CHECK(original_sound_init(project_stage.sound.v[T_303][i],ORIGINAL_SOUND_SYNTHONE,0));
  engine_bass_sound_refresh(&project_stage.sound,i);
  slot137(&project_stage.sound,i)[0]=137;
  slot137(&project_stage.sound,i)[1]=(uint8_t)i;
 }
 CHECK(!project_validate_stage());project_commit_stage();undo_start();
 stabilize();
 for(unsigned i=0;i<2;i++) {
  CHECK(engine_bass_bank(i)==ORIGINAL_SOUND_SYNTHONE);
 }
 /* COLD restore initializes owners in order: switch each explicitly and use wrapper addresses. */
}
static void play(unsigned owner,unsigned locked,unsigned base,unsigned peer)
{
 bpart_t *line=&proj.pat[0].bass[owner];
 for(unsigned k=0;k<2;++k)for(unsigned n=0;n<NSTEPS;++n)proj.pat[0].bass[k].step[n].flags=G_REST;
 line->len=16;line->rate=RATE_16;line->dir=DIR_FWD;line->transpose=24;
 line->step[3].flags=G_NOTE;line->step[3].note=45;line->step[3].velocity=64;
 line->step[4].flags=G_NOTE;line->step[4].note=45;line->step[4].velocity=127;
 seq.ppat[TRK_BASS0+owner]=0;engine_note_lock_refresh();engine_bass_edit_hold(1);seq_start(&seq);
 for(unsigned n=0;n<3000 && parts[owner]->mod_wheel_route!=locked;++n)render(1);
 CHECK(parts[owner]->mod_wheel_route==locked && parts[1-owner]->mod_wheel_route==peer);
 CHECK(!s1_typed_mod_choose(owner,1));
 pb_read(owner,-24+(int)owner,2+(int)owner);
 CHECK(engine_s1_patch_set(owner,read_slot(owner,141).revision,141,2)==S1_EDIT_BUSY);
 for(unsigned n=0;n<3000 && parts[owner]->mod_wheel_route!=base;++n)render(1);
 CHECK(parts[owner]->mod_wheel_route==base && parts[1-owner]->mod_wheel_route==peer);
 seq_stop(&seq);engine_bass_edit_hold(0);render(4);
 CHECK(parts[owner]->mod_wheel_route==base);
}
int main(void)
{
 setup(V_PART,PART_303A);proj.set.bt|=PROJ_BT_CLK_IN;
 CHECK(!s1_typed_mod_choose(0,1));
 select_patch(0);CHECK(all_bindings(0)==0);
 CHECK(!s1_typed_mod_choose(0,1));
 s1_patch_value old; CHECK(!engine_s1_patch_read(0,141,&old));
 CHECK(!current->bend_typed);
 pb_midi(0,0); CHECK(fabsf(current->e.t[BEND]/current->detm-exp2f(-2.f/12.f))<.000004f);
 pb_midi(0,8192);
 fresh();
 for(unsigned i=0;i<2;i++) {current=NULL;select_patch(i);CHECK(current);parts[i]=current;}
 CHECK(parts[0]!=parts[1]);stabilize();undo_start();
 CHECK(parts[0]->mod_wheel_route==0&&parts[1]->mod_wheel_route==0);
 float untouched_l1=parts[0]->e.t[LFO1],untouched_l2=parts[0]->e.t[LFO2],untouched_peer=parts[1]->e.t[CUT];
 midi(0,127);CHECK(fabsf(parts[0]->e.t[CUT]-360.f)<.0001f);
 CHECK(parts[0]->e.t[LFO1]==untouched_l1 && parts[0]->e.t[LFO2]==untouched_l2 && parts[1]->e.t[CUT]==untouched_peer);
 CHECK(all_bindings(0)==1 && all_bindings(1)==1);
 knob_mod(0,1);knob_mod(1,2);detent(3,5);display_route(2);
 CHECK(!s1_typed_mod_choose(0,3));
 CHECK(parts[0]->mod_wheel_route==1 && parts[1]->mod_wheel_route==2);
 CHECK(undo_commit_stable());physical_history(0);CHECK(parts[0]->mod_wheel_route==0&&parts[1]->mod_wheel_route==0);
 physical_history(1);CHECK(parts[0]->mod_wheel_route==1&&parts[1]->mod_wheel_route==2);
 project_capture_stage();uint8_t h[32];project_file_header(h);CHECK(!project_file_validate(h));
 witness=proj;CHECK(!project_save()&&!project_load_stage());project_commit_stage();stabilize();undo_start();
 CHECK(!memcmp(&witness.sound,&proj.sound,sizeof(sound_t)));
 CHECK(parts[0]->mod_wheel_route==1&&parts[1]->mod_wheel_route==2);
 pb_witness();witness=proj;
 for(unsigned i=0;i<2;i++){s1c_front_set(&parts[i]->e,64,120);s1c_front_set(&parts[i]->e,139,0);}
 unsigned writes=store_writes;
 adc(615);ui.part=PART_303A;adc(485);
 float a=parts[0]->e.t[LFO1],b=parts[1]->e.t[LFO2];
 float held_lfo2=parts[0]->e.t[LFO2],held_cut=parts[0]->e.t[CUT];
 CHECK(fabsf(a-target(127,0))<.0001f);
 knob_mod(0,2);
 CHECK(parts[0]->e.t[LFO1]==a && parts[0]->e.t[LFO2]==held_lfo2 && parts[0]->e.t[CUT]==held_cut);
 ui.part=PART_303B;adc(485); /* same deflection, even with a new report and visible owner */
 CHECK(parts[0]->e.t[LFO1]==a && parts[0]->e.t[LFO2]==held_lfo2 && parts[1]->e.t[LFO2]==b);
 adc(545);
 float mid=target((615u-545u-12u)*127u/(615u-485u-12u),0);
 CHECK(parts[0]->e.t[LFO1]==a && fabsf(parts[0]->e.t[LFO2]-mid)<.0001f);
 CHECK(parts[1]->e.t[LFO2]==b && parts[0]->e.t[CUT]==held_cut);
 adc(615);midi(1,127);CHECK(fabsf(parts[1]->e.t[LFO2]-target(127,0))<.0001f);
 knob_mod(0,1); /* persist prior authored selection for later lock witness */
 CHECK(!memcmp(&witness.sound,&proj.sound,sizeof(sound_t)) && store_writes==writes);
 engine_note_lock_capture(1);
 CHECK(!engine_bass_note_capture(proj.arr.lane,0,0,3,&proj.sound));
 CHECK(!engine_bass_note_capture(proj.arr.lane,0,1,3,&proj.sound));
 engine_note_lock_capture(0);
 knob_mod(0,2);knob_mod(1,1);
 int k=bass_note_find(proj.arr.lane,0,0,3);
 CHECK(k>=0&&proj.arr.lane[k].i==NOTE_S1_MOD&&proj.arr.lane[k].val[BASS_NOTE_TAG+11]==1);
 project_capture_stage();
 slot137(&project_stage.sound,1)[3]=3;CHECK(project_validate_stage()<0);
 project_capture_stage();project_stage.arr.lane[k].i=BASS_NOTE_VERSION;
 project_stage.arr.lane[k].val[BASS_NOTE_TAG+11]=0;CHECK(project_validate_stage()<0);
 project_capture_stage();project_stage.arr.lane[k].val[BASS_NOTE_TAG+11]=3;
 CHECK(project_validate_stage()<0);
 CHECK(!project_save()&&!project_load());stabilize();
 play(0,1,2,1);play(1,2,1,2);
 project_capture_stage();project_file_header(h);CHECK(!project_file_validate(h));
 FILE *f=fopen("out/media.bin","wb");CHECK(f);
 CHECK(fwrite(h,sizeof h,1,f)==1&&fwrite(&project_stage,sizeof project_stage,1,f)==1&&
       fwrite(store,sizeof store,1,f)==1&&fwrite(store_len,sizeof store_len,1,f)==1);
 CHECK(!fclose(f));
 /* Two typed writer generations and a deliberately torn latest object tuple. */
 uint8_t sound_obj[PLAT_STORE_MAX];
 memcpy(sound_obj,store[OBJ_SOUND],sizeof sound_obj);
 knob_mod(0,1);
 goto_pb(0);detent(1,1);render(1);pb_read(0,-24,3);
 CHECK(!project_save());
 memcpy(store[OBJ_SOUND],sound_obj,sizeof sound_obj);
 unsigned torn_writes=store_writes;
 CHECK(!project_load());stabilize();
 CHECK(!s1_typed_tuple_editable() && read137(&proj.sound,0)[3]==2);
 ui.overlay=O_NONE;ui.part=PART_303A;set_view(V_PART);build_pages();visit("PITCH MOD");
 pref_t recovered=pg.r[cur_page()][3];
 CHECK(recovered.kind==R_S1MODWHEEL && pref_read_only(recovered));
 display_route(2);detent(3,-1);display_route(2);
 CHECK(!s1_typed_mod_choose(0,1) && project_save()==-9 && store_writes==torn_writes);
 goto_pb(0); CHECK(pref_read_only(pg.r[cur_page()][1]));
 detent(1,1);render(1);pb_read(0,-24,2);pb_read(1,-23,3);
 CHECK(engine_s1_patch_set(0,read_slot(0,142).revision,142,3)==S1_EDIT_REJECTED);
 CHECK(project_save()==-9 && store_writes==torn_writes);
 printf("PASS actual typed torn-tuple previous route is readable, K4 and ordinary save cannot write\n");
 printf("PASS real SMK WHEELS and PITCH MOD K4, typed project save/reopen and Undo/Redo, held ADC+MIDI, unchanged V3 and distinct owner recall: %u checks\n",checks);
}
