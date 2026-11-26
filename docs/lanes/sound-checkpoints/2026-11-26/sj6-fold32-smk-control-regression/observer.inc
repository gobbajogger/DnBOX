#include <stdio.h>
unsigned fold_arp_admit(unsigned o)
{
  const original_runtime_part *r=&original_bass[o];
  if(r->kind!=ORIGINAL_SOUND_JUNO||!r->dsp.juno.synth.typed)return 0;
  const dnbx_part *d=&r->dsp.juno.synth.core;
  return d->layer[0].params.oscillator_mode==DNBX_OSC_SUPERJ6 &&
         d->layer[0].params.waveform==0.f && d->layer[1].params.waveform==1.f/3.f;
}
unsigned fold_arp_state(unsigned o, unsigned sample)
{
  const original_runtime_part *r=&original_bass[o];const original_runtime_arp *a=&r->arp;
  const S1Sequencer *s=&a->seq;
  unsigned vmask=0;int key=-1;
  if(r->kind==ORIGINAL_SOUND_JUNO){
   const dnbx_part *d=&r->dsp.juno.synth.core;
   for(unsigned i=0;i<DNBX_VOICES;i++)if(d->voice[i].gate){vmask|=1u<<i;key=d->voice[i].note;}
  }
  printf("STATE %u %u %d %d %u %u %u %u %u %d %d %d\n",
         sample,o,s->mStepCounter,s->lastPosition,s->mBeatTime,s->gateOpen,
         s->sequencerLastNotesCount,a->nheld,vmask,key,
         engine_bass_perf((int)o,0,1),engine_bass_perf((int)o,2,1));
  return (s->lastPosition<0?0:(1u<<(s->lastPosition&15))) | (s->gateOpen?(1u<<16):0);
}
