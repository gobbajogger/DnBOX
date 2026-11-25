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
 for(unsigned n=0;n<3000 && parts[owner]->mod_wheel_route!=base;++n)render(1);
 CHECK(parts[owner]->mod_wheel_route==base && parts[1-owner]->mod_wheel_route==peer);
 seq_stop(&seq);engine_bass_edit_hold(0);render(4);
 CHECK(parts[owner]->mod_wheel_route==base);
}
int main(void)
{
 setup(V_PART,PART_303A);proj.set.bt|=PROJ_BT_CLK_IN;
 CHECK(!s1_typed_mod_choose(0,1));
 fresh();
 for(unsigned i=0;i<2;i++) {current=NULL;select_patch(i);CHECK(current);parts[i]=current;}
 CHECK(parts[0]!=parts[1]);stabilize();undo_start();
 CHECK(parts[0]->mod_wheel_route==0&&parts[1]->mod_wheel_route==0);
 float untouched_l1=parts[0]->e.t[LFO1],untouched_l2=parts[0]->e.t[LFO2],untouched_peer=parts[1]->e.t[CUT];
 midi(0,127);CHECK(fabsf(parts[0]->e.t[CUT]-360.f)<.0001f);
 CHECK(parts[0]->e.t[LFO1]==untouched_l1 && parts[0]->e.t[LFO2]==untouched_l2 && parts[1]->e.t[CUT]==untouched_peer);
 CHECK(s1_typed_mod_choose(0,1) && s1_typed_mod_choose(1,2) && !s1_typed_mod_choose(0,3));
 CHECK(parts[0]->mod_wheel_route==1 && parts[1]->mod_wheel_route==2);
 CHECK(undo_commit_stable());do_undo(0);CHECK(parts[0]->mod_wheel_route==0&&parts[1]->mod_wheel_route==0);
 do_undo(1);CHECK(parts[0]->mod_wheel_route==1&&parts[1]->mod_wheel_route==2);
 project_capture_stage();uint8_t h[32];project_file_header(h);CHECK(!project_file_validate(h));
 witness=proj;CHECK(!project_save()&&!project_load_stage());project_commit_stage();stabilize();undo_start();
 CHECK(!memcmp(&witness.sound,&proj.sound,sizeof(sound_t)));
 CHECK(parts[0]->mod_wheel_route==1&&parts[1]->mod_wheel_route==2);
 for(unsigned i=0;i<2;i++){s1c_front_set(&parts[i]->e,64,120);s1c_front_set(&parts[i]->e,139,0);}
 unsigned writes=store_writes;
 adc(615);ui.part=PART_303A;adc(485);
 float a=parts[0]->e.t[LFO1],b=parts[1]->e.t[LFO2];
 float held_lfo2=parts[0]->e.t[LFO2],held_cut=parts[0]->e.t[CUT];
 CHECK(fabsf(a-target(127,0))<.0001f);
 CHECK(s1_typed_mod_choose(0,2));
 CHECK(parts[0]->e.t[LFO1]==a && parts[0]->e.t[LFO2]==held_lfo2 && parts[0]->e.t[CUT]==held_cut);
 ui.part=PART_303B;adc(485); /* same deflection, even with a new report and visible owner */
 CHECK(parts[0]->e.t[LFO1]==a && parts[0]->e.t[LFO2]==held_lfo2 && parts[1]->e.t[LFO2]==b);
 adc(545);
 float mid=target((615u-545u-12u)*127u/(615u-485u-12u),0);
 CHECK(parts[0]->e.t[LFO1]==a && fabsf(parts[0]->e.t[LFO2]-mid)<.0001f);
 CHECK(parts[1]->e.t[LFO2]==b && parts[0]->e.t[CUT]==held_cut);
 adc(615);midi(1,127);CHECK(fabsf(parts[1]->e.t[LFO2]-target(127,0))<.0001f);
 CHECK(s1_typed_mod_choose(0,1)); /* persist prior authored selection for later lock witness */
 CHECK(!memcmp(&witness.sound,&proj.sound,sizeof(sound_t)) && store_writes==writes);
 engine_note_lock_capture(1);
 CHECK(!engine_bass_note_capture(proj.arr.lane,0,0,3,&proj.sound));
 CHECK(!engine_bass_note_capture(proj.arr.lane,0,1,3,&proj.sound));
 engine_note_lock_capture(0);
 CHECK(s1_typed_mod_choose(0,2)&&s1_typed_mod_choose(1,1));
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
 printf("PASS S1 MOD typed project save/reopen, real Undo/Redo, physical held owner + MIDI, V3 frozen onset and independent both-owner restore: %u checks\n",checks);
}
