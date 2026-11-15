#include "SamplerVoice.h"
#include <cstdio>
using namespace DunneCore;
static unsigned checks=0;
#define CK(x) do{if(!(x)){std::fprintf(stderr,"FAIL line%d %s\n",__LINE__,#x);return 1;}++checks;}while(0)
static bool history_zero(const OriginalAnalyticOscillator&o){for(float x:o.down)if(x!=0)return false;return true;}
static void dirty(OriginalAnalyticOscillator&o){o.phase=.3125f;o.increment=.125f;for(float &x:o.down)x=.2f;}
int main(){
 AHDSHREnvelopeParameters amp;ADSREnvelopeParameters filter,pitch;
 amp.init(3000,.01,0,.05,.7,0,.1);filter.init(3000,.02,.1,.7,.2);pitch.init(3000,.03,.2,.5,.3);
 SamplerVoice v;float glide=0;v.ampEnvelope.pParameters=&amp;v.filterEnvelope.pParameters=&filter;v.pitchEnvelope.pParameters=&pitch;v.glideSecPerOctave=&glide;v.noteFrequency=0;v.init(48000);
 SampleBuffer b;b.sampleRate=44100;b.noteFrequency=261.62558f;b.startPoint=0;b.endPoint=273014;b.isLooping=true;b.loopStartPoint=175902;b.loopEndPoint=272998;
 dirty(v.synthOsc);v.start(60,48000,440,.7f,&b,false);CK(v.synthOsc.phase==0&&v.synthOsc.increment==0&&history_zero(v.synthOsc));
 dirty(v.synthOsc);v.stop();CK(v.synthOsc.phase==0&&v.synthOsc.increment==0&&history_zero(v.synthOsc));
 for(int kind=0;kind<2;++kind){
  v.start(60,48000,440,.7f,&b,false);
  for(int n=0;n<100;++n)CK(!v.prepToGetSamples(16,1,0,-1,1,0,0,.7,0,0,5));
  dirty(v.synthOsc);
  if(kind==0)v.restartSameNote(.8f,&b,false);else v.restartNewNote(61,48000,466.16376f,.8f,&b,false);
  CK(v.ampEnvelope.isPreStarting());CK(v.synthOsc.phase==.3125f&&!history_zero(v.synthOsc));
  bool done=false;
  for(int n=0;n<100;++n){
   CK(!v.prepToGetSamples(16,1,0,-1,1,0,0,.7,0,0,5));
   if(v.ampEnvelope.isPreStarting()){CK(v.synthOsc.phase==.3125f);CK(!history_zero(v.synthOsc));}
   else{CK(v.synthOsc.phase==0);CK(history_zero(v.synthOsc));done=true;break;}
  }
  CK(done);
 }
 dirty(v.synthOsc);v.restartNewNoteLegato(62,48000,493.8833f);CK(v.synthOsc.phase==.3125f&&!history_zero(v.synthOsc));
 std::printf("PASS %u source-derived original start/stop/deferred-completion resets and legato/entry retention checks; not original execution\n",checks);
}
