#define main external_fixture_main
#include "controls_external.cpp"
#undef main
#include "SamplerVoice.h"
#include <cstring>
#include <cstdint>
using namespace DunneCore;
static unsigned checks=0;
#define CK(x) do{if(!(x)){std::fprintf(stderr,"FAIL line%d %s\n",__LINE__,#x);return 1;}++checks;}while(0)
static unsigned bits(float f){unsigned u;std::memcpy(&u,&f,4);return u;}
int main(){
 for(unsigned id:{30u,32u,41u,42u,43u,44u,49u})for(float f:{-.5f,0.f,.49999997f,.5f,.50000006f,1.f}){
  CoreSampler s;CK(apply(s,id,f));bool actual=id==30?s.isOSC:id==32?s.vintageFilterADSR:id==41?s.oscSaw:id==42?s.oscTriangle:id==43?s.oscSine:id==44?s.oscPWM:s.whiteNoise;CK(actual==(f>.5f));
 }
 for(unsigned initial:{0u,1u,0x6d2b79f5u,0xffffffffu}){
  OriginalWhiteNoise n;n.seed=initial;n.reset();unsigned x=initial+0x9e3779b9u;
  if(x){x^=x>>16;x*=0x7feb352du;x^=x>>15;x*=0x846ca68bu;x^=x>>16;}else x=0x6d2b79f5u;
  CK(n.seed==x);
  for(int i=0;i<50000;++i){x^=x<<13;x^=x>>17;x^=x<<5;float expected=std::fma(float(x>>8),1.1920930376163597e-7f,-1.f);CK(bits(n.sample())==bits(expected));CK(n.seed==x);}
 }
 AHDSHREnvelopeParameters amp;ADSREnvelopeParameters filter,pitch;
 amp.init(3000,.01,0,.05,.7,0,.1);filter.init(3000,.02,.1,.7,.2);pitch.init(3000,.03,.2,.5,.3);
 for(int syrup=0;syrup<2;++syrup)for(int bypass=0;bypass<2;++bypass)for(float hz:{130.f,261.62558f,523.25116f})for(float key:{-1.f,0.f,1.f})for(float base:{3.f,1000.f,30000.f}){
  SamplerVoice v;v.ampEnvelope.pParameters=&amp;v.filterEnvelope.pParameters=&filter;v.pitchEnvelope.pParameters=&pitch;v.noteFrequency=0;float glide=0;v.glideSecPerOctave=&glide;v.init(48000);
  SampleBuffer b;b.sampleRate=44100;b.noteFrequency=261.62558f;v.start(60,48000,hz,.7f,&b,true);
  v.useSyrupFilter=syrup;v.useJunoFilter=!syrup;v.vintageFilterADSR=true;v.userCutoff=base;
  const float phase=.3125f;v.synthOsc.phase=phase;v.synthOsc.increment=.125f;for(float &h:v.synthOsc.down)h=.2f;v.start(60,48000,hz,.7f,&b,true);CK(v.synthOsc.phase==0&&v.synthOsc.increment==0);for(float h:v.synthOsc.down)CK(h==0);
  auto envelope=v.filterEnvelope;const float e=envelope.getSample(),vel=std::fma(.4f,.7f,1.f-.4f);
  double expected;
  if(syrup){double lo=std::max(double(base),1.),hi=std::max(lo,22050.),ec=std::min(std::max(double(e),0.),1.),amount=std::min(std::max(double(.3f*vel),0.),1.),t=1.-ec;expected=std::exp2(std::fma(amount*(1.-t*t),std::log2(hi)-std::log2(lo),std::log2(lo)));if(key!=0.f&&hz>1.f&&expected>1.)expected*=std::pow(double(hz)/double(261.626f),double(key));expected=std::min(std::max(expected,1.),22050.);expected=std::min(std::max(expected,12.),48000.*.45351473923);}
  else{const float span=22050.f-base;expected=double(base)+double(span*((.3f*vel)*e));if(hz>130.80999755859375f){float ratio=hz/130.80999755859375f;float power=std::max(1.f/(12.f-std::log2(ratio)),.0416666679084301f);expected+=double((((std::pow(ratio,power)-1.f)*key)*span)/3.3299999237060547f);}expected=std::max(12.,std::min(expected,22050.));}
  CK(!v.prepToGetSamples(16,1,0,bypass?-1.f:3.f,key,.3f,.4f,.7f,0,0,5));
  CK(v.isFilterEnabled==!bypass);
  CK((syrup?v.leftSyrupFilter.lastCutoff:v.leftSmoothFilter.cutoff)==expected);
  auto seed=v.whiteNoiseOsc.seed;v.stop();CK(!v.whiteNoise&&!v.pendingWhiteNoise&&v.whiteNoiseOsc.seed!=seed);CK(v.synthOsc.phase==0&&v.synthOsc.increment==0);for(float h:v.synthOsc.down)CK(h==0);
 }
 std::printf("PASS %u source-derived checks: controls/noise/lifecycle/vintage law; not original execution or active-vintage PCM proof\n",checks);
}
