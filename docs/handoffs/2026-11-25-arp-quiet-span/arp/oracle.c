/* Validate quiet-prefix reduction against repeated original operations. */
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "S1Sequencer.h"
typedef struct { unsigned f,n; unsigned event[8192]; } Events;
static void on(void *c,int k,int v){Events *e=c; assert(e->n<8192); e->event[e->n++]=(e->f<<16)|((unsigned)k<<8)|(unsigned)v;}
static void off(void *c,int k){on(c,k,0);}
static uint32_t seed=0x431ab23u;
static uint32_t rng(void){seed=seed*1664525u+1013904223u;return seed^seed>>16;}
int main(void) {
 unsigned long skipped=0,frames=0;
 for(unsigned j=0;j<30000;++j){
  S1Sequencer a,b;
  memset(&a,0,sizeof a);S1Sequencer_init(&a);
  S1SequencerParams p={0};
  p.arpIsOn=(uint8_t)(j%5u!=0);
  p.arpIsSequencer=(uint8_t)(j&1u);
  p.arpDirection=(uint8_t)(j%4u);
  p.arpOctave=(uint8_t)(j%4u);
  p.arpInterval=(uint8_t)(j%12u);
  p.arpTotalSteps=(uint8_t)(1u+j%16u);
  p.sequencerNoteOn=0xbdb7u;
  p.sequencerOctBoost=0x7789u;
  p.patternRange=24;
  p.seqGate=(uint8_t)(j%101u);
  p.arpBeatsPerSample=j%19u==0?0u:j%313u==0?UINT32_MAX:256u;
  S1Sequencer_setStep(&p,j%43u==0?0u:32000u+(rng()%1000000u));
  p.clocked=(j&8u)!=0;
#if X0X_SUPERJ6_BUILTIN_INIT
  p.saga_notes=(j&32u)!=0;p.saga_timing=(j&64u)!=0;
#endif
#if X0X_SJ6_SEQ_SETTINGS_NATIVE
  p.saga_snap_native=(j&128u)!=0;
#endif
  a.mBeatTime=j%37u==0?UINT32_MAX-rng()%50u:rng()%((p.arpSeqTempoMultiplier|1u)+2048u);
  a.previousHeldNoteNumbersAECount=(int)(j%4u);
  b=a;
  NoteNumber held[3]={{48,85},{60,98},{64,118}};
  Events one={0}, many={0};
  for (unsigned f=0;f<5;++f) {
   int n=(int)(rng()%4u); unsigned span=1u+rng()%256u; frames+=span;
   if(j%17u==0 && f==2) S1Sequencer_setStep(&p,rng()%91000u);
   for(unsigned i=0;i<span;i++){one.f++; S1Sequencer_process(&a,&p,held,n,&one,on,off);}
   for(unsigned i=0;i<span;){
    int k=i?0:S1Sequencer_skipQuiet(&b,&p,n,span);
    if(k>0){i+=k;many.f+=k;skipped+=k;continue;}
    i++;many.f++;S1Sequencer_process(&b,&p,held,n,&many,on,off);
   }
   assert(!memcmp(&a,&b,sizeof a));
   assert(one.n==many.n && !memcmp(one.event,many.event,one.n*sizeof one.event[0]));
  }
 }
 printf("%lu / %lu original samples skipped without changing sequencer state or event timestamps\n",skipped,frames);
}
