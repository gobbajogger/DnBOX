/* SPDX-License-Identifier: GPL-3.0-only */
/* RESOURCE FIXTURE ONLY. Existing stock18 controls drive both candidate layers.
 * Exact stock chorus code is appended unchanged. No original engine code.
 * This does not claim sound equivalence or the original218-control semantics. */
#include "juno6.h"
#include "fastmath.h"
#include <string.h>
#define TWO_PI 6.28318530717958647692f
static float clampf(float x,float a,float b){return x<a?a:x>b?b:x;}

void juno6_init(juno6_t *s,float sr)
{
 memset(s,0,sizeof *s);
 (void)dnbx_init(&s->core,sr>8000?sr:44100);
 s->macro[0]=.35f;s->macro[4]=.35f;s->macro[10]=.12f;
 s->macro[15]=1;s->macro[16]=.5f;
}
int juno6_note_on(juno6_t *s,uint8_t note,uint8_t velocity)
{ return dnbx_note_on(&s->core,note,velocity)>=0; }
void juno6_note_off(juno6_t *s,uint8_t note)
{ (void)dnbx_note_off(&s->core,note); }
void juno6_all_off(juno6_t *s)
{
 (void)dnbx_all_off(&s->core);s->macro[17]=0;
 memset(s->hp_state,0,sizeof s->hp_state);memset(s->hp_input,0,sizeof s->hp_input);
}
int juno6_active(const juno6_t *s)
{ return dnbx_active_voices(&s->core)>0; }

int juno6_set(juno6_t *s,unsigned id,float x)
{
 if(!s||id>=18||!(x>=0&&x<=1))return 0;
 s->macro[id]=x;
 if(id==17)return dnbx_set_sustain(&s->core,x>=.5f)==DNBX_OK;
 if(id==9){s->hpf=x;return 1;}
 if(id==13||id==14)return 1; /* Existing runtime owns stock chorus parameters. */
 for(unsigned l=0;l<DNBX_LAYERS;l++){
  dnbx_layer_params p;
  if(dnbx_get_layer(&s->core,l,&p)!=DNBX_OK)return 0;
  switch(id){
  case 0:p.waveform=(2+x)/3;break;
  case 1:p.sub_gain=x*.5f;break;
  case 2:p.cutoff_hz=35+x*x*12000;break;
  case 3:p.resonance=x;break;
  case 4:p.filter_env_hz=7000*x;break;
  case 5:p.amp.attack_s=p.filter.attack_s=.002f+x*x*3;break;
  case 6:p.amp.decay_s=p.filter.decay_s=.01f+x*x*3;break;
  case 7:p.amp.sustain=p.filter.sustain=x;break;
  case 8:p.amp.release_s=p.filter.release_s=.01f+x*x*4;break;
  case 10:p.lfo_hz=.1f+8*x*x;break;
  case 11:p.lfo_pwm_depth=x*.2f;break;
  case 12:p.pulse_width=clampf(.1f+x*.8f,.03f,.5f);break;
  case 15:p.gain=x;break;
  case 16:p.tune_semitones=(x-.5f)*4;break;
  default:break;
  }
  if(dnbx_set_layer(&s->core,l,&p)!=DNBX_OK)return 0;
 }
 return 1;
}

int juno6_render_stereo(juno6_t *s,float *left,float *right,unsigned frames)
{
 if(dnbx_render(&s->core,left,right,frames,0)!=DNBX_OK)return 0;
 if(s->hpf>0){
  const float hp=1/(1+TWO_PI*(15+2000*s->hpf*s->hpf)/s->core.sample_rate);
  for(unsigned i=0;i<frames;i++){
   float y=hp*(s->hp_state[0]+left[i]-s->hp_input[0]);
   s->hp_input[0]=left[i];s->hp_state[0]=y;left[i]=y;
   y=hp*(s->hp_state[1]+right[i]-s->hp_input[1]);
   s->hp_input[1]=right[i];s->hp_state[1]=y;right[i]=y;
  }
 }
 return 1;
}

static unsigned legacy_stage(unsigned stage)
{
 if(stage==DNBX_ENV_IDLE)return 0;
 if(stage==DNBX_ENV_ATTACK)return 1;
 if(stage==DNBX_ENV_RELEASE_HOLD||stage==DNBX_ENV_RELEASE)return 4;
 return stage==DNBX_ENV_SUSTAIN?3u:2u;
}
int juno6_read_envelope(const juno6_t *s,juno6_envelope_snapshot *out)
{
 if(!s||!out)return 0;
 juno6_envelope_snapshot value={0};
 const dnbx_envelope_params *p=&s->core.layer[0].params.amp;
 value.adsr[0]=p->attack_s;value.adsr[1]=p->decay_s;
 value.adsr[2]=p->sustain;value.adsr[3]=p->release_s;
 value.selected_voice=-1;value.shared_amp_filter=0;
 for(unsigned i=0;i<DNBX_VOICES;i++)for(unsigned l=0;l<DNBX_LAYERS;l++){
  const dnbx_voice *v=&s->core.voice[i];
  const dnbx_envelope_state *e=&v->layer[l].amp;
  if(e->stage&&(value.selected_voice<0||e->level>value.level)){
   value.selected_voice=(int)i;value.level=e->level;
   value.stage=legacy_stage(e->stage);value.gate=v->gate;value.active=1;
  }
 }
 *out=value;return 1;
}

void juno6_chorus_init(juno6_chorus_t*c,float sr){memset(c,0,sizeof *c);c->sample_rate=sr>8000?sr:44100;c->rate_hz=.8f;c->depth_ms=3.4f;c->mix=.22f;}
void juno6_chorus_set(juno6_chorus_t*c,float r,float d,float m){c->rate_hz=clampf(r,.1f,5);c->depth_ms=clampf(d,1.7f,5.4f);c->mix=clampf(m,0,.5f);}
void juno6_chorus_process(juno6_chorus_t*c,float*l,float*r,unsigned n){unsigned i;float maxd=fm_minf(255,c->sample_rate*.0054f);for(i=0;i<n;i++){float in=(l[i]+r[i])*.5f,ph=(float)c->phase/4294967296.0f*TWO_PI,d=clampf(c->sample_rate*(.0017f+(c->depth_ms*.001f-.0017f)*(.5f+.5f*fm_sinf(ph))),c->sample_rate*.0017f,maxd);unsigned whole=(unsigned)d,a=(c->write+256-whole)&255,b=(a+255)&255;float frac=d-whole,del=((float)c->delay[a]*(1-frac)+(float)c->delay[b]*frac)/32768.0f,wet=del*c->mix;c->delay[c->write]=(int16_t)clampf(in*32767,-32768,32767);c->write=(c->write+1)&255;l[i]=l[i]*(1-c->mix)+wet;r[i]=r[i]*(1-c->mix)-wet;c->phase+=(uint32_t)(c->rate_hz*4294967296.0f/c->sample_rate);}}
