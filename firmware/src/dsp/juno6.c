/* SPDX-License-Identifier: GPL-3.0-only */
#include "juno6.h"
#include <math.h>
#include "fastmath.h"
#include <string.h>
#define PI 3.14159265358979323846f
#define TWO_PI 6.28318530717958647692f
static float clampf(float x,float a,float b){return x<a?a:x>b?b:x;}
/* Build the shipping functions directly by default. The factory adapter calls
 * these same implementations for a legacy owner. */
#if X0X_SUPERJ6_FACTORY
#define JUNO6_LEGACY(result, name) static result stock_##name
#else
#define JUNO6_LEGACY(result, name) result juno6_##name
#endif
static float midi_hz(unsigned n){return 440.0f*fm_exp2f(((float)n-69.0f)*(1.0f/12.0f));}
static inline __attribute__((always_inline)) float blep(float t,float dt){if(t<dt){t/=dt;return t+t-t*t-1.0f;}if(t>1.0f-dt){t=(t-1.0f)/dt;return t*t+t+t+1.0f;}return 0.0f;}
JUNO6_LEGACY(void, init)(juno6_t*s,float sr){memset(s,0,sizeof *s);s->sample_rate=sr>8000?sr:44100;s->attack=.008f;s->decay=.28f;s->sustain=.72f;s->release=.24f;s->cutoff=.68f;s->resonance=.12f;s->pulse_width=.5f;s->sub_level=.22f;s->macro[0]=.35f;s->macro[4]=.35f;s->macro[10]=.12f;s->macro[15]=1.0f;s->macro[16]=.5f;}
/* Re-key the sounding pitch even after key-up, while pedal or release keeps it alive. */
JUNO6_LEGACY(int, note_on)(juno6_t*s,uint8_t note,uint8_t vel){unsigned i,chosen=0;uint32_t oldest;if(!vel){juno6_note_off(s,note);return 1;}for(i=0;i<6;i++)if(s->voice[i].stage&&s->voice[i].note==note){chosen=i;goto assign;}for(i=0;i<6;i++)if(!s->voice[i].gate&&s->voice[i].envelope<.0001f){chosen=i;goto assign;}for(i=0;i<6;i++)if(!s->voice[i].gate){chosen=i;goto assign;}oldest=s->voice[0].age;for(i=1;i<6;i++)if(s->voice[i].age<oldest){oldest=s->voice[i].age;chosen=i;}assign:{juno6_voice_t*v=&s->voice[chosen];v->note=note;v->gate=1;v->stage=1;v->age=++s->age;v->increment=midi_hz(note)/s->sample_rate*fm_exp2f(s->bend*(1.0f/12.0f));v->velocity=(float)vel/127.0f;if(v->envelope<.0001f)v->phase=0;}return 1;}
JUNO6_LEGACY(void, note_off)(juno6_t*s,uint8_t note){unsigned i;for(i=0;i<6;i++)if(s->voice[i].gate&&s->voice[i].note==note){s->voice[i].gate=0;if(!s->damper)s->voice[i].stage=4;}}
JUNO6_LEGACY(void, all_off)(juno6_t *s)
{
 memset(s->voice,0,sizeof s->voice);
 s->damper=0;s->macro[17]=0;s->hp_state=s->hp_input=0;
}
JUNO6_LEGACY(void, render)(juno6_t *s, float *out, unsigned n)
{
 const float sr=s->sample_rate;
 const float attack=1.0f/fm_maxf(1,s->attack*sr);
 const float decay=(1-s->sustain)/fm_maxf(1,s->decay*sr);
 const float release=1.0f/fm_maxf(1,s->release*sr);
 const float base=35+s->cutoff*s->cutoff*12000;
 const float env=7000*s->macro[4];
 const float hp=1.0f/(1.0f+TWO_PI*(15+2000*s->hpf*s->hpf)/sr);
 const float rate=(.1f+8*s->macro[10]*s->macro[10])/sr;
 /* Each voice is independent up to its addition to the mono mix. Render in
  * slot order so rounding is unchanged, and skip idle/released voices once
  * per block instead of testing six slots at each output sample. */
 const float oscillator_mix = s->macro[0], volume = s->macro[15] / 6;
 const float resonance = s->resonance * .8f, pwm = s->macro[11];
 float widths[32];
 for (unsigned start = 0; start < n; start += 32) {
  unsigned frames = n - start < 32 ? n - start : 32;
  float *mix = out + start;
  for (unsigned k = 0; k < frames; ++k) {
   float lfo = 1 - 4 * fm_fabsf(s->lfo_phase - .5f);
   s->lfo_phase += rate;
   if (s->lfo_phase >= 1) s->lfo_phase -= 1;
   widths[k] = clampf(s->pulse_width + lfo * pwm * .2f, .05f, .95f);
   mix[k] = 0;
  }
  for (unsigned i = 0; i < JUNO6_VOICES; ++i) {
   juno6_voice_t *v = &s->voice[i];
   if (!v->stage) continue;
   const float dt = clampf(v->increment, .000001f, .45f);
   for (unsigned k = 0; k < frames; ++k) {
    float t = v->phase, width = widths[k];
    float correction = blep(t, dt);
    float saw = 2 * t - 1 - correction;
    float pulse = (t < width ? 1.0f : -1.0f) + correction, q = t - width;
    if (q < 0) q += 1;
    pulse -= blep(q, dt);
    float sub = v->subphase < .5f ? 1.0f : -1.0f;
    float x = (saw * (1 - oscillator_mix) + pulse * oscillator_mix) * .7f + sub * s->sub_level;
    if (v->stage == 1) {
     v->envelope += attack;
     if (v->envelope >= 1) { v->envelope = 1; v->stage = 2; }
    } else if (v->stage == 2) {
     v->envelope -= decay;
     if (v->envelope <= s->sustain) { v->envelope = s->sustain; v->stage = 3; }
    } else if (v->stage == 4) {
     v->envelope -= fm_maxf(.000001f, v->envelope) * release;
     if (v->envelope < .00001f) { v->envelope = 0; v->stage = 0; }
    }
    float a = clampf(TWO_PI * (base + v->envelope * env) / sr, .001f, .75f);
    v->filter_state += a * (x - v->filter_state - resonance * v->filter);
    v->filter += a * (v->filter_state - v->filter);
    mix[k] += v->velocity * v->envelope * v->filter;
    v->phase += dt;
    if (v->phase >= 1) v->phase -= 1;
    v->subphase += dt * .5f;
    if (v->subphase >= 1) v->subphase -= 1;
    if (!v->stage) break;
   }
  }
  for (unsigned k = 0; k < frames; ++k) {
   float value = mix[k] * volume;
   if (s->hpf > 0) {
    float y = hp * (s->hp_state + value - s->hp_input);
    s->hp_input = value;
    s->hp_state = y;
    value = y;
   }
   mix[k] = value;
  }
 }
}
void juno6_chorus_init(juno6_chorus_t*c,float sr){memset(c,0,sizeof *c);c->sample_rate=sr>8000?sr:44100;c->rate_hz=.8f;c->depth_ms=3.4f;c->mix=.22f;}
void juno6_chorus_set(juno6_chorus_t*c,float r,float d,float m){c->rate_hz=clampf(r,.1f,5);c->depth_ms=clampf(d,1.7f,5.4f);c->mix=clampf(m,0,.5f);}
#if X0X_SUPERJ6_FACTORY
#include "juno6_chorus_factory.h"
#endif
void juno6_chorus_process(juno6_chorus_t *c, float *l, float *r, unsigned n)
{
#if X0X_SUPERJ6_FACTORY
 if (c->factory.mode) {
  juno6_chorus_render_factory(c, l, r, n);
  return;
 }
#endif
 const float sr = c->sample_rate;
 const float min_delay = sr * .0017f, max_delay = fm_minf(255, sr * .0054f);
 const float depth = c->depth_ms * .001f - .0017f;
 const float mix = c->mix, dry = 1 - mix;
 const uint32_t increment = (uint32_t)(c->rate_hz * 4294967296.0f / sr);
 uint32_t phase = c->phase;
 unsigned write = c->write;
 for (unsigned i = 0; i < n; ++i) {
  float in = (l[i] + r[i]) * .5f;
  float angle = (float)phase / 4294967296.0f * TWO_PI;
  float d = clampf(sr * (.0017f + depth * (.5f + .5f * fm_sinf(angle))), min_delay, max_delay);
  unsigned whole = (unsigned)d, a = (write + 256 - whole) & 255, b = (a + 255) & 255;
  float frac = d - whole;
  float delayed = ((float)c->delay[a] * (1 - frac) + (float)c->delay[b] * frac) / 32768.0f;
  float wet = delayed * mix;
  c->delay[write] = (int16_t)clampf(in * 32767, -32768, 32767);
  write = (write + 1) & 255;
  l[i] = l[i] * dry + wet;
  r[i] = r[i] * dry - wet;
  phase += increment;
 }
 c->write = write;
 c->phase = phase;
}

JUNO6_LEGACY(int, active)(const juno6_t *s) {
 for(unsigned i=0;i<JUNO6_VOICES;i++) if(s->voice[i].stage) return 1;
 return 0;
}
JUNO6_LEGACY(int, set)(juno6_t *s,unsigned id,float x) {
 if(!s || id>=18 || !(x>=0 && x<=1)) return 0;
 s->macro[id]=x;
 switch(id) {
 case 0:s->waveform=(uint8_t)(x*2.99f);break;
 case 1:s->sub_level=x*.5f;break;
 case 2:s->cutoff=x;break;
 case 3:s->resonance=x;break;
 case 5:s->attack=.002f+x*x*3;break;
 case 6:s->decay=.01f+x*x*3;break;
 case 7:s->sustain=x;break;
 case 8:s->release=.01f+x*x*4;break;
 case 9:s->hpf=x;break;
 case 12:s->pulse_width=.1f+x*.8f;break;
 case 16:s->bend=(x-.5f)*4;for(unsigned i=0;i<6;i++) if(s->voice[i].stage) s->voice[i].increment=midi_hz(s->voice[i].note)/s->sample_rate*fm_exp2f(s->bend*(1.0f/12.0f));break;
 case 17:s->damper=x>=.5f;if(!s->damper)for(unsigned i=0;i<6;i++)if(!s->voice[i].gate&&s->voice[i].stage)s->voice[i].stage=4;break;
 default:break;
 }
 return 1;
}

JUNO6_LEGACY(int, read_envelope)(const juno6_t *s,juno6_envelope_snapshot *out)
{
 if(!s||!out)return 0;
 juno6_envelope_snapshot v={0};
 v.adsr[0]=s->attack;v.adsr[1]=s->decay;v.adsr[2]=s->sustain;v.adsr[3]=s->release;
 v.selected_voice=-1;v.shared_amp_filter=1;
 for(unsigned i=0;i<JUNO6_VOICES;i++){
  const juno6_voice_t *n=&s->voice[i];
  if(n->stage && (v.selected_voice<0 || n->envelope>v.level)){
   v.selected_voice=(int)i;v.level=n->envelope;v.stage=n->stage;v.gate=n->gate;v.active=1;
  }
 }
 *out=v;return 1;
}

#if X0X_SUPERJ6_FACTORY
#include "juno6_factory.inc"
#endif
