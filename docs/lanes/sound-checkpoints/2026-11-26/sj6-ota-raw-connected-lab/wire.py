from pathlib import Path
import sys
core,drv,dh,jh,out=map(Path,sys.argv[1:]); my=Path(sys.argv[0]).parent.parent/'pitchenv-lease'
mod=(my/'lab.inc').read_text()
a='/* Return 1 eligible'; b='typedef struct {const ColdProfile *profile; unsigned enabled,failed;} ColdRun;'
assert mod.count(a)==mod.count(b)==1
types,mod=mod.split(a,1)
(out/'lease-types.h').write_text('#ifndef LAB_LEASE_TYPES\n#define LAB_LEASE_TYPES\n'+types+b+'''
typedef struct sj6_cold_ctx { ColdLease lease; ColdProfile profile; ColdRun run; float p[4],c[5]; } sj6_cold_ctx;
#endif
''')
(out/'lease-logic.h').write_text(a+mod.replace(b,''))
s=dh.read_text(); k='    dnbx_voice voice[DNBX_VOICES];';assert s.count(k)==1
dh.write_text(s.replace(k,k+'\n    struct sj6_cold_ctx *cold_lab;'))
s=jh.read_text(); p='typedef struct {\n float phase,increment,envelope,filter,filter_state,velocity;';assert s.count(p)==1
s=s.replace(p,'#include "lease-types.h"\n'+p)
for slot,n in (('a',55),('b',8),('c',3)):
 p=f'float typed_filter_gap_{slot}[{n}];'; assert s.count(p)==1
 s=s.replace(p,f'Gap{slot.upper()} typed_filter_gap_{slot};')
jh.write_text(s)
s=core.read_text()
assert s.count('    assert(arm<=2);')==1
s=s.replace('    assert(arm<=2);','    assert(arm<=3);')
p='static inline float sj6_ota_stage';assert s.count(p)==1
s=s.replace(p,'''#include "lease-types.h"
#define OTA LeaseOTA
#define clock_2x lease_clock
#define physical_tick lease_tick
#define physical_replay lease_replay
#define physical_tile_tick lease_tile
#define prepare_tile lease_prepare
#define ap_pair lease_ap
#define fmaf(a,b,c) (((a)*(b))+(c))
#include "transposed.inc"
#undef fmaf
#undef clock_2x
#undef physical_replay
#undef prepare_tile
#undef physical_tile_tick
#undef ap_pair
#include "lease-logic.h"
#undef OTA
#undef physical_tick
static uint32_t lab_sub_pcm[131072]; static unsigned lab_sub_n,lab_resets;
static void lab_sub_trace(float x){assert(lab_sub_n<131072);memcpy(lab_sub_pcm+lab_sub_n++, &x,4);}
__attribute__((visibility("default"))) const uint32_t *lab_sub_words(void){return lab_sub_pcm;}
__attribute__((visibility("default"))) unsigned lab_sub_count(void){return lab_sub_n;}
__attribute__((visibility("default"))) unsigned lab_reset_count(void){return lab_resets;}
void bind_lab(dnbx_part *s,sj6_cold_ctx *d,GapA *a,GapB *b,GapC *c,float gain){
 if(ota_arm!=3 || gain==0.f)return;
 assert(gain==.808f && s->sj6_ota && !s->sj6_pitch_depth && !s->sj6_pitch_routed && !dnbx_active_voices(s));
 assert(s->sample_rate==44100.f && params_ota[0]==1.8208369016647339f);
 d->lease=(ColdLease){a,b,c};
 memcpy(d->p,params_ota,sizeof d->p);memcpy(d->c,coeff_ota,sizeof d->c);
 d->profile=(ColdProfile){30000.00390625f,44100.f,1.191f,gain,d->p,d->c};
 d->run=(ColdRun){&d->profile,1,0};
 s->cold_lab=d;
}
'''+p)
# Catch actual fresh allocation at both receiver paths. Do not reset a retrigger.
p='    if(ota_arm){memset(main_ota+chosen,0,';assert s.count(p)==1
s=s.replace(p,'    if(s->cold_lab){reset_cold(s->cold_lab->lease,v,chosen);++lab_resets;}\n'+p)
p='    if(!voice_active(v))memset(v,0,sizeof *v);';assert s.count(p)==1
s=s.replace(p,'    if(!voice_active(v)){memset(v,0,sizeof *v);if(s->cold_lab){reset_cold(s->cold_lab->lease,v,0);++lab_resets;}}')
# Avoid touching actual Main+Sub stock recurrence for arm3, while default is untouched.
p='                if(s->sj6_ota){\n                    float y=sj6_ota_filter(lv,x,a,p->resonance);';assert s.count(p)==1
s=s.replace(p,'''                if(s->cold_lab && !l){
                    sj6_cold_ctx *ctx=s->cold_lab;
                    const FrameFingerprint f={base+env*amount,s->sample_rate,p->resonance,main_in,osc_gain[0],p->waveform,
                     p->oscillator_mode,s->sj6_ota,1,s->sj6_pitch_depth,s->sj6_pitch_routed,!s->sj6_noise_gain,!p->sub_gain};
                    if(match(&ctx->profile,&f)!=1 || ctx->run.failed || osc_gain[1]!=0.f || p->pan!=0.f ||
                        amount!=0.f || !s->sj6_sub_gain) {ctx->run.failed=1;abort();}
                    const float sub_y=sj6_ota_filter(lv,sub_in,a,p->resonance); /* ACTUAL stock Sub recurrence */
                    lab_sub_trace(sub_y);
                    float y;
                    if(guarded_main_sample(&ctx->run,ctx->lease,v,i,&f,sub_y,&y)!=1)abort();
                    mix[l]+=velocity*amp*y;
                    ++ota_count;
                } else if(s->sj6_ota){
                    float y=sj6_ota_filter(lv,x,a,p->resonance);''')
k='y=orig+sj6_ota_filter(isolated_sub+i,sub_in,a,p->resonance);';assert s.count(k)==1
s=s.replace(k,"const float old_sub_y=sj6_ota_filter(isolated_sub+i,sub_in,a,p->resonance); lab_sub_trace(old_sub_y); y=orig+old_sub_y;")
core.write_text('#include <stdlib.h>\n'+s)
s=drv.read_text();p='  prepare(bpm);';assert s.count(p)==1
s=s.replace(p,p+'''
  sj6_cold_ctx lab={0};
  juno6_t *donor=&part.dsp.juno.synth;
  extern void bind_lab(dnbx_part *,sj6_cold_ctx *,GapA *,GapB *,GapC *,float);
  bind_lab(&donor->core,&lab,&donor->typed_filter_gap_a,&donor->typed_filter_gap_b,
     &donor->typed_filter_gap_c,donor->osc1_level_code?((float)(donor->osc1_level_code-1)/127.f)*.808f:1.f);
''')
# Bind run-owned caller context only within this render call.
p='assert(total==148684);';assert s.count(p)==1
s=s.replace(p,'donor->core.cold_lab=0; '+p)
drv.write_text(s)
