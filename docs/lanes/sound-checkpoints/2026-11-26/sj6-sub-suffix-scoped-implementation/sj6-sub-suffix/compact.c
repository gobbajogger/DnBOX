/* Diagnostic typed projection. No reference samples enter this renderer. */
#include <assert.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#ifdef SJ6_RUNTIME_OVERRIDE
#include SJ6_RUNTIME_OVERRIDE
#else
#include "original_runtime.c"
#endif
static float peak[2][2];
static unsigned above_unity[2],above_ring[2];
void witness(unsigned stage,const float *l,const float *r,unsigned n){
    for(unsigned i=0;i<n;i++){
        const float a=fabsf(l[i]),b=fabsf(r[i]);
        if(a>peak[stage][0])peak[stage][0]=a;
        if(b>peak[stage][1])peak[stage][1]=b;
        above_unity[stage]+=(a>1.f)+(b>1.f);
        above_ring[stage]+=(a>7.999f)+(b>7.999f);
    }
}

static original_runtime_part part;
static original_runtime_workspace work;
static int16_t delay_hist[2u*SJ6_NATIVE_DELAY_CAP],unison_hist[2][SJ6_UNISON_HISTORY];
static float word[256];
static unsigned present[256];
#include "suffix.h"
static unsigned suffix_scope;
static sj6_scoped_dry_suffix routed_suffix;
int sj6_validated_dry_pre(float*l,float*r,unsigned n) {
 if(!suffix_scope)return 0;
 sj6_pre_dry(&routed_suffix,l,r,n);return 1;
}
void sj6_validated_dry_post(float*l,float*r,unsigned n) {
 if(suffix_scope)sj6_post_dry(&routed_suffix,l,r,n);
}
static void select_suffix(void) {
 /* All fitted/bypassed stages outside this exact connected historical
  * input are refused. #0 retains the ordinary original runtime multiplier. */
 const unsigned reject[]={11,16,17,22,23,24,28,40,41,59,63,103,107,108,109,142};
 suffix_scope=1;
 for(unsigned k=0;k<sizeof reject/sizeof *reject;k++)
    if(!present[reject[k]]||word[reject[k]]!=0.f)suffix_scope=0;
 fprintf(stderr,"exact_HP_zeroCrusher_finalDC=%u\n",suffix_scope);
}

static unsigned q(unsigned id,unsigned max){ return (unsigned)(word[id]*max+.5f); }
static unsigned opt(unsigned id){ return present[id]?q(id,127)+1:0; }
static unsigned bid(unsigned id){ return word[id]>.5f; }
static void prepare(float bpm){
  part.kind=ORIGINAL_SOUND_JUNO;part.status=ORIGINAL_RUNTIME_OK;
  juno6_t *s=&part.dsp.juno.synth; juno6_chorus_t *c=&part.dsp.juno.chorus;
  juno6_init(s,44100.f); juno6_chorus_init(c,44100.f);
  assert(s->typed && !bid(63) && !bid(142) && !bid(212)); /* do not silently disable active arp/seq/scale */
  assert(juno6_set(s,JUNO6_APP_PARAM_BASE,word[0]));
  const juno6_oscillator_edit a[2]={{word[82],word[78],word[210]},{word[83],word[79],word[211]}};
  juno6_apply_factory_waves(s,a);juno6_apply_pre_note_range(s,word[86]);
  s->osc1_level_code=q(1,127)+1;s->osc2_level_code=q(3,127)+1;
  uint8_t sources[2]={q(201,127),q(202,127)};dnbx_set_sj6_sources(&s->core,0,sources);
  dnbx_set_sj6_vintage(&s->core,0,q(123,127));
  if(present[206])juno6_set_a_reference(s,word[206],1);
  juno6_set_layer_semi(s,0,q(72,24));juno6_set_layer_semi(s,1,q(73,24));
  juno6_set_layer_fine(s,0,q(74,100));juno6_set_layer_fine(s,1,q(75,100));
  uint8_t env[SJ6_EXTRA_COUNT]={q(36,127),q(37,127),q(38,127),q(39,127),
    q(132,127),q(133,127),q(134,127),q(135,127),bid(116)};
  juno6_apply_panel_envelopes(s,511u,env,0);
  if(!bid(115))for(unsigned i=119;i<=122;i++)assert(juno6_set(s,JUNO6_APP_PARAM_BASE+i,word[i]));
  juno6_sj6_set_ota(s,q(209,1));
  assert(juno6_set(s,2,word[7]) && juno6_set(s,3,word[8]) && juno6_set(s,4,word[136]));
  juno6_set_sj6_hpf(s,word[28]); assert(juno6_set_filter_velocity(s,q(137,127)));
  assert(juno6_mono32_set(s,bid(32)) && juno6_legato33_set(s,bid(33)) && juno6_glide35_set(s,q(35,127)));
  assert(dnbx_set_velocity_mode(&s->core,DNBX_VELOCITY_LINEAR)==DNBX_OK);
  const juno6_lfo_words lfo={{word[88],word[96],word[100],word[195],word[196],word[197]},63};
  juno6_set_lfo_tempo(s,bpm);juno6_set_lfo_words(s,&lfo,1);
  juno6_set_vibrato(s,word[105],word[106],1);
  juno6_lfo1_controls(s,q(94,4),!bid(87),bid(198));juno6_lfo1_delay_set(s,q(91,127));
  if(bid(87)&&word[181]==1.f && word[89])assert(juno6_configure_lfo1_cutoff(s,word[180],word[7],word[89]));
  assert(dnbx_store_raw(&s->core,131,word[131])==DNBX_RAW_STORED_ONLY);
  assert(dnbx_store_raw(&s->core,136,word[136])==DNBX_RAW_STORED_ONLY);
  const uint8_t penv[5]={q(138,127),q(139,127),q(140,127),q(141,127),q(57,60)};
  assert(dnbx_set_sj6_pitch_env(&s->core,penv));
  if (bid(95) || bid(99)) {
    unsigned rt[4]={word[185],word[187],word[189],word[191]};
    uint8_t aux[18]={1u+bid(95),1u+bid(99),q(101,127),q(102,4),q(93,127),
      rt[0]?rt[0]+7:0,rt[1]?rt[1]+7:0,rt[2]?rt[2]+7:0,rt[3]?rt[3]+7:0,
      q(184,127),q(186,127),q(188,127),q(190,127),bid(192),bid(193),bid(194),bid(199),bid(200)};
    juno6_set_lfo2_shape(s,q(98,4));
    s->lfo2_pwm=(uint16_t)((s->lfo2_pwm&~SJ6_LFO2_PWM_DEPTH_MASK)|q(97,127));
    juno6_set_aux_lfo(s,aux);
  }
  if (word[103]) assert(juno6_alt3_set_panel(s,q(103,127),q(104,127),bid(118)));
  c->factory_mode=JUNO6_CHORUS_OFF+bid(40)+2u*bid(41);
  
  const unsigned jrate=(word[215]==.5f)?0u:opt(215),jdepth=(word[216]==.5f)?0u:opt(216);
  uint8_t j60[4]={bid(208),jrate,jdepth,opt(217)};
  juno6_j60_config(c,j60);
  uint8_t classic[3]={opt(42),opt(43),opt(44)};
  if(!c->j60.selected)juno6_classic_native_config(c,classic);
  uint8_t dl[4]={bid(24)+2u*bid(118),q(25,127),q(26,127),q(27,127)};
  juno6_delay_native_arm(s,delay_hist);juno6_delay_native_config(s,dl,bpm);
  assert(!s->delay24.clock_refused || !bid(24));
  uint8_t unison[4]={bid(59),opt(60),q(61,127),q(62,5)};
  juno6_unison_config(s,unison,unison_hist);
  uint8_t room[SJ6_ROOM_COUNT]={bid(11),bid(12),q(13,127),q(14,127),q(15,127),bid(16),q(22,127),q(23,127)};
  original_runtime_sj6_room(&part,room);original_runtime_sj6_room_clock(&part,bpm,bid(118));
  original_runtime_sj6_rage(&part,q(17,127));
  part.dsp.juno.crush.step=1.f/(1.f+31.f*fm_powf((float)q(108,127)/127.f,1.23f));
  part.dsp.juno.crush.active=bid(107);part.dsp.juno.crush.phase=1.f-part.dsp.juno.crush.step;
}

int main(int argc,char**argv){
  assert(argc==3);FILE *in=fopen(argv[1],"r"),*out=fopen(argv[2],"wb");assert(in&&out);
  unsigned n;float bpm;assert(fscanf(in,"%f%u",&bpm,&n)==2&&n<256);
  for(unsigned i=0,id;i<n;i++){float x;assert(fscanf(in,"%u%f",&id,&x)==2&&id<256&&!present[id]&&isfinite(x));word[id]=x;present[id]=1;}
  prepare(bpm); select_suffix();
  assert(fscanf(in,"%u",&n)==1&&n<4096);
  unsigned total=0;
  float l[128],r[128],st[256];
  for(unsigned i=0;i<n;i++){
    unsigned frames,ev,key,v;
    assert(fscanf(in,"%u%u",&frames,&ev)==2&&frames&&frames<=128&&ev<5);
    while(ev--){assert(fscanf(in,"%u%u",&key,&v)==2);assert(original_runtime_key(&part,ORIGINAL_INPUT_USB,key,(v&&bid(205))?87u:v)==ORIGINAL_RUNTIME_OK);}
    assert(original_runtime_render(&part,&work,l,r,frames)==ORIGINAL_RUNTIME_OK);
    for(unsigned j=0;j<frames;j++){assert(isfinite(l[j])&&isfinite(r[j]));st[2*j]=l[j];st[2*j+1]=r[j];}
    assert(fwrite(st,2*sizeof(float),frames,out)==frames);total+=frames;
  }
  fprintf(stderr,"gain %a pre_fx %a %a pre_master %a %a above1 %u/%u above7.999 %u/%u\n",
    part.dsp.juno.synth.master_gain,peak[0][0],peak[0][1],peak[1][0],peak[1][1],
    above_unity[0],above_unity[1],above_ring[0],above_ring[1]);
  assert(total==148684); fclose(in);assert(!fclose(out));
}
/* The fixture has one SJ6 owner. A misroute aborts instead of returning silence. */
#include <stdlib.h>
int original_chicago_backend_render(original_chicago_backend *p,original_chicago_backend_workspace *w,const original_engine_event *e,uint32_t n,float*l,float*r,uint32_t frames){abort();}
int original_miami_backend_render_stereo(original_miami_backend *p,original_miami_backend_workspace *w,float*l,float*r,uint32_t frames){abort();}
void s1_part_render(s1_dry_part*p,float*l,float*r,unsigned frames){abort();}
