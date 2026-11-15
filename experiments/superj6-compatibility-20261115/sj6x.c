/* SPDX-License-Identifier: GPL-3.0-only */
/* ORIGINAL bounded feature proxy. No AudioKit or extracted proprietary code. */
#include "sj6x.h"
#include <stddef.h>
#define TAG 0x534a3658u
#define PI2 6.28318530717958647692f
static int between(float x,float lo,float hi){return x>=lo&&x<=hi;}
static float clamp(float x,float lo,float hi){return x<lo?lo:x>hi?hi:x;}
static void zero(void*p,size_t n){volatile unsigned char*b=(volatile unsigned char*)p;while(n--)*b++=0;}
static int live(const sj6x_engine*s){return s&&s->tag==TAG&&between(s->sample_rate,8000,96000);}
static int patch_ok(const sj6x_patch*p){
 if(!p)return 0;
 for(unsigned i=0;i<3;i++)if(!between(p->envelope[i].attack,.001f,8)||!between(p->envelope[i].decay,.001f,8)||
  !between(p->envelope[i].sustain,0,1)||!between(p->envelope[i].release,.001f,8)||
  !between(p->lfo[i].hz,0,30)||!between(p->lfo[i].depth,0,1)||(p->lfo[i].routes&~15u))return 0;
 for(unsigned i=0;i<2;i++)if(!between(p->pulse_width[i],.05f,.95f)||!between(p->detune_cents[i],-200,200)||!between(p->layer_mix[i],0,1))return 0;
 return between(p->saw,0,1.001f)&&between(p->pulse,0,1.001f)&&between(p->sub,0,1)&&between(p->noise,0,1)&&
  between(p->level,0,1.32f)&&between(p->cutoff_hz,20,20000)&&between(p->resonance,0,.6f)&&
  between(p->filter_env_hz,0,12000)&&between(p->highpass_hz,0,6000);
}
void sj6x_default_patch(sj6x_patch*p){
 if(!p)return;zero(p,sizeof *p);
 for(unsigned i=0;i<3;i++){p->envelope[i].attack=.008f;p->envelope[i].decay=.28f;p->envelope[i].sustain=.72f;p->envelope[i].release=.24f;}
 p->saw=.7f;p->pulse=.3f;p->sub=.2f;p->level=.8f;p->cutoff_hz=5000;p->resonance=.12f;p->filter_env_hz=2000;
 for(unsigned i=0;i<2;i++){p->pulse_width[i]=.5f;p->layer_mix[i]=.5f;}
}
int sj6x_init(sj6x_engine*s,float sr){
 if(!s||!between(sr,8000,96000))return 0;zero(s,sizeof *s);s->sample_rate=sr;s->noise_state=0x12345678u;s->tag=TAG;sj6x_default_patch(&s->patch);return 1;
}
int sj6x_apply(sj6x_engine*s,const sj6x_patch*p){if(!live(s)||!patch_ok(p))return 0;sj6x_patch next=*p;s->patch=next;return 1;}
static float hz(unsigned n){
 static const float semitone[12]={8.175798916f,8.661957218f,9.177023997f,9.722718241f,10.30086115f,10.91338223f,11.56232571f,12.24985737f,12.9782718f,13.75f,14.56761755f,15.43385316f};
 float f=semitone[n%12];for(unsigned i=0;i<n/12;i++)f*=2;return f;
}
/* Original exp2 polynomial on [-.667,.667], used only for chosen manual pitch.
 * Not an inferred app tuning/detune law. */
static float ratio(float x){
 const float y=x*.69314718056f;
 return 1+y*(1+y*(.5f+y*(.16666666667f+y*(.04166666667f+y*(.00833333333f+y*.00138888889f)))));
}
static void release(sj6x_voice*v,const sj6x_patch*p,float sr){
 v->gate=0;for(unsigned i=0;i<3;i++){v->envelope[i].stage=4;v->envelope[i].release_step=v->envelope[i].value/(p->envelope[i].release*sr);}
}
int sj6x_note_on(sj6x_engine*s,unsigned note,unsigned velocity){
 if(!live(s)||note>127||velocity>127)return 0;if(!velocity)return sj6x_note_off(s,note);
 unsigned chosen=6;
 for(unsigned i=0;i<6;i++)if(s->voice[i].gate&&s->voice[i].note==note){chosen=i;break;}
 if(chosen==6)for(unsigned i=0;i<6;i++)if(!s->voice[i].envelope[0].stage&&!s->voice[i].envelope[1].stage){chosen=i;break;}
 if(chosen==6){chosen=s->next_voice;s->next_voice=(s->next_voice+1)%6;}
 sj6x_voice*v=&s->voice[chosen];zero(v,sizeof *v);v->note=note;v->gate=1;v->hz=hz(note);v->velocity=(float)velocity/127;
 for(unsigned i=0;i<3;i++)v->envelope[i].stage=1;return 1;
}
int sj6x_note_off(sj6x_engine*s,unsigned note){
 if(!live(s)||note>127)return 0;for(unsigned i=0;i<6;i++)if(s->voice[i].gate&&s->voice[i].note==note)release(&s->voice[i],&s->patch,s->sample_rate);return 1;
}
int sj6x_all_off(sj6x_engine*s){
 if(!live(s))return 0;zero(s->voice,sizeof s->voice);s->hp_state=s->hp_input=0;return 1;
}
static float env(sj6x_envelope*e,const sj6x_adsr*p,float sr){
 if(e->stage==1){e->value+=1/(p->attack*sr);if(e->value>=1){e->value=1;e->stage=2;}}
 else if(e->stage==2){e->value-=(1-p->sustain)/(p->decay*sr);if(e->value<=p->sustain){e->value=p->sustain;e->stage=3;}}
 else if(e->stage==3)e->value=p->sustain;
 else if(e->stage==4){e->value-=e->release_step;if(e->value<=.000001f){e->value=0;e->stage=0;}}
 return e->value;
}
static float blep(float t,float dt){
 if(t<dt){t/=dt;return t+t-t*t-1;}
 if(t>1-dt){t=(t-1)/dt;return t*t+t+t+1;}return 0;
}
static float triangle(float p){float x=p-.5f;if(x<0)x=-x;return 1-4*x;}
int sj6x_render(sj6x_engine*s,float*out,unsigned count){
 if(!live(s)||count>SJ6X_BLOCK_MAX)return 0;if(!count)return 1;
 const uintptr_t o=(uintptr_t)out,b=(uintptr_t)s;const size_t bytes=(size_t)count*sizeof(float);
 if(!out||o%__alignof__(float)||o>UINTPTR_MAX-bytes||b>UINTPTR_MAX-sizeof *s||(o<b+sizeof *s&&b<o+bytes))return 0;
 const sj6x_patch*p=&s->patch;const float sr=s->sample_rate;
 const float hp=sr/(sr+PI2*p->highpass_hz);
 const float detune[2]={ratio(p->detune_cents[0]/1200),ratio(p->detune_cents[1]/1200)};
 for(unsigned k=0;k<count;k++){
  float pwm=0,pitch=0,cut=0,amp=0,mix=0;
  for(unsigned l=0;l<3;l++){
   const float x=triangle(s->lfo_phase[l])*p->lfo[l].depth;const unsigned mask=p->lfo[l].routes;
   if(mask&SJ6X_PWM)pwm+=x;if(mask&SJ6X_PITCH)pitch+=x;if(mask&SJ6X_CUTOFF)cut+=x;if(mask&SJ6X_AMP)amp+=x;
   s->lfo_phase[l]+=p->lfo[l].hz/sr;if(s->lfo_phase[l]>=1)s->lfo_phase[l]-=1;
  }
  const float vibrato=ratio(pitch*(1.f/6.f)),ampgain=clamp(1+amp*.5f,0,2);
  for(unsigned i=0;i<6;i++){
   sj6x_voice*v=&s->voice[i];if(!v->envelope[0].stage&&!v->envelope[1].stage)continue;
   float e[3];for(unsigned a=0;a<3;a++)e[a]=env(&v->envelope[a],&p->envelope[a],sr);
   float x=0;
   for(unsigned l=0;l<2;l++){
    const float dt=clamp(v->hz*detune[l]*vibrato/sr,.000001f,.45f),t=v->phase[l],pw=clamp(p->pulse_width[l]+pwm*.2f,.05f,.95f);
    const float saw=2*t-1-blep(t,dt);float q=t-pw;if(q<0)q+=1;
    const float pulse=(t<pw?1.f:-1.f)+blep(t,dt)-blep(q,dt);
    x+=(p->saw*saw+p->pulse*pulse)*p->layer_mix[l]*e[l];
    v->phase[l]+=dt;if(v->phase[l]>=1)v->phase[l]-=1;
   }
   v->subphase+=clamp(v->hz*vibrato/sr*.5f,.0000005f,.225f);if(v->subphase>=1)v->subphase-=1;
   s->noise_state^=s->noise_state<<13;s->noise_state^=s->noise_state>>17;s->noise_state^=s->noise_state<<5;
   const float noise=(float)(s->noise_state>>8)*(1.f/8388608.f)-1;
   x+=((v->subphase<.5f?1.f:-1.f)*p->sub+noise*p->noise)*.5f*(e[0]+e[1]);
   const float coefficient=clamp(PI2*(p->cutoff_hz+e[2]*p->filter_env_hz+cut*3000)/sr,.001f,.7f);
   v->filter_a+=coefficient*(x-v->filter_a-p->resonance*v->filter_b);
   v->filter_b+=coefficient*(v->filter_a-v->filter_b);
   mix+=v->velocity*v->filter_b*ampgain;
  }
  mix*=p->level*(1.f/6.f);
  if(p->highpass_hz>0){const float y=hp*(s->hp_state+mix-s->hp_input);s->hp_input=mix;s->hp_state=y;mix=y;}
  out[k]=mix;
 }
 return 1;
}
