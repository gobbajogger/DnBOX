#include "original_runtime.c"
#include "master.h"
#include <assert.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#ifndef RING_ENC
#define RING_ENC 32767.f
#endif
static original_runtime_part p;
static master_t m[2];
static float l[128],r[128], fl[128], fr[128];
int original_chicago_backend_retire(original_chicago_backend *p){(void)p;abort();}
static void go(int n,int coh,int chorus,int waveform)
{
 memset(&p,0,sizeof p);
 juno6_t *j=&p.dsp.juno.synth;
 juno6_init(j,44100);
 p.kind=ORIGINAL_SOUND_JUNO;
 p.status=ORIGINAL_RUNTIME_OK;
 juno6_chorus_init(&p.dsp.juno.chorus,44100);
 p.dsp.juno.chorus.factory_mode=chorus?JUNO6_CHORUS_I:JUNO6_CHORUS_OFF;
 if(chorus)p.dsp.juno.chorus.mix=.5f;
 j->osc1_level_code=128;j->osc2_level_code=128;
 for(unsigned k=0;k<2;k++){
   dnbx_layer_params v; assert(!dnbx_get_layer(&j->core,k,&v));
   v.waveform=waveform/3.f;v.pan=0;v.gain=1;v.cutoff_hz=20000;v.resonance=.8f;
   v.sub_gain=0;v.filter_env_hz=0;v.oscillator_mode=DNBX_OSC_SUPERJ6;
   v.amp.attack_s=.002f;v.amp.decay_s=.002f;v.amp.sustain=1.f;v.amp.release_s=.008f;
   assert(!dnbx_set_layer(&j->core,k,&v));
 }
 const uint8_t gain_codes[2]={127,127}; dnbx_set_sj6_sources(&j->core,0,gain_codes);
#if DNBX_HOST_SJ6_FIXED_SOURCE_ROUTE
 j->core.sj6_host_fixed_source=1;
#endif
 const unsigned note[]={57,52,55,60,64,67};
 for(int k=0;k<n;k++)assert(dnbx_note_on(&j->core,note[k],n==1?87:127)==k);
 if(coh)for(int k=1;k<n;k++)j->core.voice[k].layer[0].increment=j->core.voice[0].layer[0].increment;
 double srcpeak=0,fxpeak=0,post[2]={0},pcm[2][2]={{0}};
 unsigned ring_over=0; double e2=0, y2=0;
 unsigned changed[2]={0},clipped[2][2]={{0}};
 for(unsigned q=0;q<2;q++){master_init(m+q);master_set(m+q,MST_LIMIT,q);}
 for(unsigned k=0;k<80;k++){
   assert(juno6_render_stereo(j,l,r,128));
   /* Undo the exact legacy common factor on a centered, unity-gain two-layer diagnostic; no adoption or authored gain claim. */
   for(unsigned i=0;i<128;i++){l[i]*=12.f;r[i]*=12.f;}
   for(unsigned i=0;i<128;i++)srcpeak=fmax(srcpeak,fmax(fabs(l[i]),fabs(r[i])));
   if(chorus)for(unsigned i=0;i<128;i++){
       const double y=(l[i]+r[i])*.5f;
       ring_over+= fabs(y)*RING_ENC>32767.f;
       const double recovered=(int16_t)fmax(-32768.,fmin(32767.,y*RING_ENC))/
               (RING_ENC==32767.f?32768.:RING_ENC);
       const double e=y-recovered;
       e2+=e*e;y2+=y*y;
   }
   sj6_run_effects(&p,l,r,128);
   for(unsigned i=0;i<128;i++)fxpeak=fmax(fxpeak,fmax(fabs(l[i]),fabs(r[i])));
   for(unsigned q=0;q<2;q++){
      memcpy(fl,l,sizeof(l));memcpy(fr,r,sizeof(r));
      master_process_st(m+q,fl,fr,128,1.f);
      for(unsigned i=0;i<128;i++){
         post[q]=fmax(post[q],fabs(fl[i]));
         if(q==0)changed[q]+=fl[i]!=l[i];
         for(unsigned loud=0;loud<2;loud++){
            const float lim=loud?1.f:2.f,scale=loud?8388607.f:4194303.f;
            const float out=fl[i]>lim?lim:fl[i]<-lim?-lim:fl[i];
            clipped[q][loud]+=out!=fl[i];
            pcm[q][loud]=fmax(pcm[q][loud],abs((int32_t)(out*scale)));
         }
      }
   }
 }
 printf("fixed=%d n=%d coherent=%d classic=%d waveform=%d ringOver=%u pre=%.9f postfx=%.9f masterOff=%.9f masterOn=%.9f hardChanged=%u PCM24 normal/loud off %.0f/%.0f on %.0f/%.0f writerCuts %u,%u,%u,%u historyErr %.3f dBr\n",DNBX_HOST_SJ6_FIXED_SOURCE_ROUTE,n,coh,chorus,waveform,ring_over,srcpeak,fxpeak,post[0],post[1],changed[0],pcm[0][0],pcm[0][1],pcm[1][0],pcm[1][1],clipped[0][0],clipped[0][1],clipped[1][0],clipped[1][1],10*log10(e2/y2));
}
int main(void){for(int w=0;w<4;w++){go(6,0,1,w);go(6,1,1,w);}}
