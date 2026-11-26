/* Bench carrier for actual DnBOX arp source. Does not read original emitted events. */
#include "original_runtime.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "stimulus.h"
#include "S1Sequencer.c"
#include "arp-runtime.inc"
#if X0X_S1_FACTORY_ARP_INGRESS && !S1_ARP_REAL_PACK
unsigned s1_part_programs(void) { return 906u; } /* pinned accepted host bank cardinality; app supplies actual function */
#endif
static unsigned frame;
static unsigned valid;
static FILE *output;
static void on(void*x,int note,int velocity){(void)x;assert(note>=0&&note<128&&velocity>0&&velocity<=127);fprintf(output,"%u %d %d\n",frame,note,velocity);++valid;}
static void off(void*x,int note){(void)x;assert(note>=0&&note<128);fprintf(output,"%u %d 0\n",frame,note);++valid;}
int main(int argc,char **argv)
{
 assert(argc==5);
 FILE *input=fopen(argv[1],"rt");assert(input);
 output=fopen(argv[3],"wt"); assert(output);
 const unsigned index=(unsigned)strtoul(argv[2],0,10);
 const char *mode=argv[4];
 const int interval_probe=!strcmp(mode,"panel-source-interval");
 const int factory=!strcmp(mode,"factory");
 const int panel=interval_probe||!strcmp(mode,"panel"),stock=!strcmp(mode,"stock");
 assert(panel||stock||factory||!strcmp(mode,"project"));
 static original_runtime_part p;
 p.kind=ORIGINAL_SOUND_SYNTHONE;p.status=ORIGINAL_RUNTIME_OK;p.arp.bpm=120.f;
 engine_arp_cfg_default(&p.arp.cfg);S1Sequencer_init(&p.arp.seq);
 float v[150]={0};unsigned n,read=0;
 float x; while(fscanf(input,"%u %a",&n,&x)==2) {assert(n<150&&isfinite(x));v[n]=x;++read;}
 assert(read==56&&feof(input));fclose(input);
 if(factory){
#if X0X_S1_FACTORY_ARP_INGRESS
#if S1_ARP_REAL_PACK
   const char *pack=getenv("S1_ARP_PACK");
   assert(pack);
   FILE *f=fopen(pack,"rb");assert(f);
   static uint8_t bytes[1048576];
   const size_t n=fread(bytes,1,sizeof bytes,f);
   assert(n<sizeof bytes&&feof(f)&&fclose(f)==0);
   assert(s1_part_set_pack(bytes,(uint32_t)n)==906);
   s1_part_init(&p.dsp.synthone);
   assert(s1_part_program(&p.dsp.synthone,index));
#else
   p.dsp.synthone.program=(uint16_t)index;
   memcpy(p.dsp.synthone.e.t,v,sizeof v);
#endif
   const engine_arp_cfg untouched=p.arp.cfg;
   assert(!original_runtime_s1_factory_cfg(&p,&p.arp.cfg,1,0));  /* saved user arp (including off) */
   assert(!original_runtime_s1_factory_cfg(&p,&p.arp.cfg,0,1));  /* pending user edit */
   assert(memcmp(&untouched,&p.arp.cfg,sizeof untouched)==0);
   const uint16_t program=p.dsp.synthone.program;
   p.dsp.synthone.program=S1_PART_INIT;
   assert(!original_runtime_s1_factory_cfg(&p,&p.arp.cfg,0,0));
   p.dsp.synthone.program=program;
   const int8_t status=p.status;p.status=ORIGINAL_RUNTIME_INVALID;
   assert(!original_runtime_s1_factory_cfg(&p,&p.arp.cfg,0,0));
   p.status=status;
   assert(memcmp(&untouched,&p.arp.cfg,sizeof untouched)==0);
   if(original_runtime_s1_factory_cfg(&p,&p.arp.cfg,0,0))
       p.arp.prm.s1FactoryPitch=p.dsp.synthone.e.t;
#else
   abort();
#endif
 }else if(!stock)for(n=60;n<=114;++n)assert(engine_arp_from_s1(&p.arp.cfg,n,v[n]));
 arp_params(&p);
 const char *sc=getenv("S1_AUTHORED_TUNING_NPO");
 if(sc)p.arp.prm.npo=(uint8_t)strtoul(sc,0,10);
 if(interval_probe && (!p.arp.prm.npo||p.arp.prm.npo==12))p.arp.prm.arpInterval=(uint8_t)fmaxf(0.f,fminf(12.f,v[61]));
 unsigned factor=0;float min=INFINITY;
 for(n=0;n<19;++n){const union {uint32_t i;float f;} f={.i=source_factor_bits(n)};
   const float d=fabsf(f.f-v[147]);if(d<min){factor=n;min=d;}}
 const float bpm=roundf(v[64]); 
 int accepted=panel&&original_runtime_s1_clock(&p,bpm,factor);
#if X0X_S1_FACTORY_ARP_INGRESS
 if(factory)accepted=original_runtime_s1_factory_clock(&p);
#endif
 fprintf(stderr,"bpm=%a factor=%a interval=%a mapped_interval=%u mapped_steps=%u receiver_bpm=%a factor_stop=%u factor_abs_error=%a panel_admitted=%d fixed_project_frames=%u\n",
  (double)v[64],(double)v[147],(double)v[61],p.arp.prm.arpInterval,p.arp.prm.arpTotalSteps,(double)bpm,factor,(double)min,accepted,engine_arp_step_frames(p.arp.bpm,p.arp.cfg.speed));
 const unsigned active=p.arp.prm.arpIsOn;
 fprintf(output,"S1T1 %u %u %u %u\n",index,S1AB_LEAD+S1AB_FRAMES,active,p.arp.prm.arpIsSequencer);
 for(frame=0;frame<S1AB_LEAD+S1AB_FRAMES;++frame){
   const unsigned cue=frame>=S1AB_LEAD?frame-S1AB_LEAD:UINT32_MAX;
   if(cue<=S1AB_NOTES*S1AB_NOTE_LEN && cue%S1AB_NOTE_LEN==0u){
     n=cue/S1AB_NOTE_LEN;
     if(n)arp_hold(&p,s1ab_note[n-1],0);
     if(n<S1AB_NOTES)arp_hold(&p,s1ab_note[n],s1ab_vel(n));
   }
   if(!active&&!p.arp.seq.sequencerLastNotesCount)continue;
   if(accepted){
       const S1ClockPulse pulse=source_clock_sample(&p.arp);
       S1Sequencer_processOriginal(&p.arp.seq,&p.arp.prm,p.arp.held,p.arp.nheld,&p,on,off,&pulse);
   }else S1Sequencer_process(&p.arp.seq,&p.arp.prm,p.arp.held,p.arp.nheld,&p,on,off);
 }
 fprintf(stderr,"events=%u\n",valid);
 return fclose(output);
}
