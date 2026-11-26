from pathlib import Path
import subprocess,sys,tempfile
d=Path(sys.argv[1]);s=(d/'original_presets.inc').read_text();p=Path(__file__).resolve().parent
ui=(d/'ui.c').read_text();rp=(d/'remote_sj6_panel.inc').read_text()
rr=(d/'remote_synth_panel.inc').read_text(); rdn=(d/'remote_dnr3.c').read_text()
assert all(x in rp for x in ('{107, R_SJ6CRUSH, 0}', '{108, R_SJ6CRUSH, 1}'))
assert '{109, R_SJ6CRUSH,' not in rp
assert 'pref_set_impl(r, value, 0);' in rr and 'return pref_get(r) == value ? value : -1;' in rr
assert 'value=(int)rd_g(q+6,2);' in rdn and 'value>(int)d->max' in rdn
assert '[TYPED_R_SJ6CRUSH-TYPED_1_BASE-1]=sj6_crush_choose' in ui
assert 'crush[0]>1u || crush[1]>127u' in ui
pre=r'''
#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "converter.h"
#define NBASS 2
#define X0X_COLD
typedef struct {uint8_t crush[2][2];} sound_t;
static sound_t project;
static sound_t *mot_base_snd=&project;
typedef struct {struct {struct {struct {float step,phase,hold[2]; uint8_t active;} crush;} juno;} dsp;} original_runtime_part;
static original_runtime_part original_bass[2];
static int held,allowed=1,receipt[2],accepted,revoke_seen;
static sj6_crush_applied applied[2];
static int original_typed_sj6_live(const sound_t*s,unsigned o){return s&&o<NBASS;}
static int original_typed_edit_open(unsigned o){assert(held==1);return allowed&&o<NBASS;}
static void engine_project_hold(int x){held+=x?1:-1;assert(held>=0&&held<=1);}
static const uint8_t *sj6_crush_code(const sound_t*s,unsigned o){return s->crush[o];}
static uint8_t *sj6_crush_write(sound_t*s,unsigned o){return s->crush[o];}
static void original_typed_undo_apply(unsigned m,void(*fn)(const sound_t*,unsigned)){
 assert(held==1); for(unsigned o=0;o<NBASS;o++)if((m>>o)&1u)fn(mot_base_snd,o);
}
static void sj6_native_room_apply(const sound_t*,unsigned);
'''
hooks=r'''
void sj6_crush_ingress_accepted(unsigned o,unsigned id,unsigned q,float raw) {
 assert(held==1); ++accepted;
 if(receipt[o])assert(sj6_crush_commit(&applied[o],id,q,raw));
}
void sj6_crush_ingress_revoke(unsigned o) {assert(held==1);receipt[o]=0;revoke_seen++;}
static void sj6_native_room_apply(const sound_t *s,unsigned o){sj6_crush_apply(s,o);}
'''
body=r'''
int main(void){
 original_bass[0].dsp.juno.crush.phase=.375f;
 original_bass[0].dsp.juno.crush.hold[0]=.25f;
 assert(sj6_crush_choose(0,1,47)&&project.crush[0][1]==47);
 assert(original_bass[0].dsp.juno.crush.step<1.f&&!original_bass[0].dsp.juno.crush.active);
 assert(original_bass[0].dsp.juno.crush.phase==.375f&&original_bass[0].dsp.juno.crush.hold[0]==.25f);
 assert(accepted==1 && !applied[0].wet_seen);
 assert(!sj6_crush_choose(0,1,47)&&!sj6_crush_choose(0,1,128)&&!sj6_crush_choose(0,2,1));
 allowed=0; assert(!sj6_crush_choose(0,0,1)&&accepted==1);
 allowed=1; receipt[0]=receipt[1]=1;
 assert(sj6_crush_choose(0,0,1)&&applied[0].wet_pct==30.f&&applied[0].booster==1.3f);
 assert(sj6_crush_choose(0,1,64)&&applied[0].booster==1.3f&&!applied[0].cadence_seen);
 assert(!applied[1].wet_seen && !original_bass[1].dsp.juno.crush.active);
 unsigned before=accepted;
 engine_project_hold(1); project.crush[0][0]=0;project.crush[0][1]=47;sj6_native_room_undo(1u);engine_project_hold(0);
 assert(accepted==before&&revoke_seen==1&&!receipt[0]&&receipt[1]);
 assert(project.crush[0][0]==0&&project.crush[0][1]==47&&!original_bass[0].dsp.juno.crush.active);
 assert(sj6_crush_choose(0,1,48)&&accepted==before+1&&!receipt[0]&&!applied[0].cadence_seen);
 printf("real indexed choose/storage/Undo extracted; 7-bit vs hold/stale proofs, no graph changes\n");
}
'''
def section(s,a,b):return s[s.index(a):s.index(b,s.index(a))]
with tempfile.TemporaryDirectory() as t:
 t=Path(t)
 subprocess.run([sys.executable,str(p/'compose.py'),str(d/'original_presets.inc'),str(t/'mut.inc')],check=True)
 c=(t/'mut.inc').read_text()
 pick=section(c,'static __attribute__((noinline)) int original_typed_indexed_choose','static __attribute__((noinline)) int original_typed_live_undo_open')
 apply=section(c,'static void sj6_crush_apply','static void sj6_native_room_apply(const sound_t *sound,unsigned owner)')
 undo=section(c,'X0X_COLD void sj6_native_room_undo','#endif\n#if X0X_SJ6_OTA_NATIVE')
 src=t/'test.c';src.write_text(pre+pick+apply+undo+hooks+body)
 exe=t/'exe'
 subprocess.run(['cc','-std=c11','-Wall','-Werror','-O2','-ffp-contract=off','-DX0X_HOST=1','-DSJ6_CRUSH_EDGE_WITNESS','-I'+str(p.parent/'sj6-crusher-converter'),'-I'+str(d),'-I'+str(d.parent/'dsp'),str(src),'-o',str(exe)],check=True)
 subprocess.run([str(exe)],check=True)
