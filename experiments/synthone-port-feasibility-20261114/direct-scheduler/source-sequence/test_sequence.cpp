#include "source_sequence.hpp"
#include "original_sequence.hpp"
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <cstdint>
#include <new>
#include <limits>
using namespace s1source_sequence;
static uint64_t allocations=0, samples=0, events=0,cases=0,rejects=0;
void *operator new(std::size_t n){++allocations;if(void*p=std::malloc(n?n:1))return p;throw std::bad_alloc();}
void operator delete(void*p)noexcept{std::free(p);}
void operator delete(void*p,std::size_t)noexcept{std::free(p);}
struct Event {int kind,note,velocity;};
struct Sink{
 Event events[2048]{};unsigned count=0;
 void on(int n,int v){if(count>=2048)std::abort();events[count++]={1,n,v};}
 void off(int n){if(count>=2048)std::abort();events[count++]={0,n,0};}
 void beat(){if(count>=2048)std::abort();events[count++]={2,0,0};}
};
[[noreturn]] static void fail(const char*why){std::fprintf(stderr,"FAIL %s case=%llu sample=%llu\n",why,(unsigned long long)cases,(unsigned long long)samples);std::exit(1);}
struct Pair{
 float p[150]{};HeldNote held[128]{};unsigned count=1;Sink a,b;SerializedSequence runtime;
 s1source_oracle::OriginalSequence oracle;
 Pair():oracle([&](int n,int v){a.on(n,v);},[&](int n){a.off(n);},[&](){a.beat();}){
  p[arpIsOn]=1;p[arpRate]=120;p[arpSeqTempoMultiplier]=.25f;p[arpInterval]=12;p[arpTotalSteps]=16;
  for(int i=0;i<16;i++){p[sequencerNoteOn00+i]=1;p[sequencerPattern00+i]=(i%5)-2;p[sequencerOctBoost00+i]=i%2;}
  for(int i=0;i<128;i++)held[i]={i,120-i,(i*31)%128};
  rate(48000);init();
 }
 void rate(double x){oracle.setSampleRate(x);runtime.set_sample_rate(x);}
 void init(){oracle.init();runtime.init();}
 void reset(bool x){oracle.reset(x);runtime.reset(x);}
 void npo(int x){oracle.setNotesPerOctave(x);runtime.set_notes_per_octave(x);}
 void clock(double x){oracle.mBeatTime=x;runtime.beat_time=x;}
 void compare(){
  if(a.count!=b.count||std::memcmp(a.events,b.events,a.count*sizeof(Event)))fail("callback stream");
  if(std::memcmp(&oracle.mBeatTime,&runtime.beat_time,sizeof(double)))fail("binary64 clock");
  if(oracle.previousHeldNoteNumbersAECount!=runtime.previous_held||oracle.getArpBeatCount()!=runtime.get_beat_count()||
     oracle.mNotesPerOctave.load()!=runtime.notes_per_octave())fail("sequence state");
  if(oracle.sequencerLastNotes.size()!=runtime.last_count+runtime.initial_zero_count)fail("pending count");
  unsigned i=0;for(int n:oracle.sequencerLastNotes){int v=i<runtime.last_count?runtime.last_notes[i]:0;if(v!=n)fail("pending notes");i++;}
 }
 void one(){
  a.count=b.count=0;HeldView v{held,count};s1source_oracle::DSPParameters values{};std::memcpy(values.data(),p,sizeof p);
  oracle.process(values,v);const auto before=allocations;
  if(!runtime.process(p,150,v,b))fail("legal source domain rejected");
  if(allocations!=before)fail("runtime heap allocation");
  compare();samples++;events+=a.count;
 }
 void beat(unsigned k){double dt=(p[arpRate]/60.f)/runtime.sample_rate;clock(static_cast<double>(k)*p[arpSeqTempoMultiplier]-dt*.5);one();}
 void bad(){
  const SerializedSequence old=runtime;const auto before=allocations;b.count=0;
  if(runtime.process(p,150,HeldView{held,count},b))fail("invalid input accepted");
  if(std::memcmp(&old,&runtime,sizeof old)||b.count||allocations!=before)fail("reject mutated state/events/heap");rejects++;
 }
};
int main(){
 {
  Pair t;t.held[0]={60,0,100};t.one();if(t.a.count!=1027)fail("source initial 1024 zero offs");cases++;
 }
 // Every arp direction, octave count, interval, representative tuning and all
 // generated positions, including UpDown omission of both tail duplicates.
 for(int mode=0;mode<3;mode++)for(int oct=0;oct<4;oct++)for(int n:{1,2,3,8,128})
 for(int interval:{0,1,7,12})for(int np:{1,5,12,19,31,53}){
  Pair t;t.count=n;t.p[arpDirection]=mode;t.p[arpOctave]=oct;t.p[arpInterval]=interval;t.npo(np);
  t.one();unsigned m=n*(oct+1),length=mode==1&&m>1?2*m-2:m;
  for(unsigned i=1;i<=length*2+2;i++)t.beat(i);cases++;
 }
 // All 16 step lengths, all per-step enable/boost patterns, noninteger control
 // truncation and microtonal transpositions; all held velocities/order preserved.
 for(int len=1;len<=16;len++)for(int n:{1,2,7,128})for(int np:{1,12,19,53}){
  Pair t;t.count=n;t.p[arpIsSequencer]=1;t.p[arpTotalSteps]=len;t.npo(np);
  for(int i=0;i<16;i++){t.p[sequencerPattern00+i]=(i%2?-1.f:1.f)*(i%13+.25f);if(t.p[sequencerPattern00+i]>12)t.p[sequencerPattern00+i]=12;if(t.p[sequencerPattern00+i]<-12)t.p[sequencerPattern00+i]=-12;t.p[sequencerNoteOn00+i]=(i%3)/2.f;t.p[sequencerOctBoost00+i]=(i%4)/3.f;}
  t.one();for(unsigned i=1;i<=unsigned(len*4+2);i++)t.beat(i);cases++;
 }
 // Contiguous real sample clocks with serialized UI/preset/note transitions.
 for(double sr:{44100.,48000.,96000.}){
  Pair t;t.rate(sr);t.count=0;uint32_t rng=0x12345678;
  for(unsigned i=0;i<220000;i++){
   rng=rng*1664525u+1013904223u;
   if(i%997==0){t.count=(rng>>8)%129;t.p[arpIsOn]=(rng&1)?1.f:.5f;t.p[arpIsSequencer]=(rng&2)?1.f:.5f;t.p[arpRate]=1.f+(rng%200);t.p[arpDirection]=(rng>>4)%3;t.p[arpOctave]=(rng>>7)%4;t.p[arpInterval]=(rng>>11)%13;t.p[arpTotalSteps]=1+(rng>>15)%16;t.p[arpSeqTempoMultiplier]=(i%2)?1.f/64.f/1.5f:8.f;t.npo((i%5)?12:19);}
   if(i%10111==0)t.reset((i&1)!=0);
   if(i%17027==0)t.init();
   t.one();
  }cases++;
 }
 // Source exact ==1 gate and integer conversion behavior, not truthiness.
 for(float flag:{0.f,.5f,.99999994f,1.f})for(float mode:{0.f,.5f,1.f}){
  Pair t;t.p[arpIsOn]=flag;t.p[arpIsSequencer]=mode;t.p[arpDirection]=1.9f;t.p[arpOctave]=2.9f;t.p[arpInterval]=11.9f;t.p[arpTotalSteps]=3.9f;t.count=3;t.one();for(unsigned i=1;i<30;i++)t.beat(i);cases++;
 }
 // Reinitialization resizes the source list to1024 without resetting clock/step.
 {Pair t;t.count=128;t.p[arpIsSequencer]=1;t.one();t.init();t.one();t.reset(false);t.one();t.reset(true);t.one();t.npo(0);t.beat(5);t.npo(-99);t.beat(6);cases++;}
 // Invalid controls / ranges / coordinates fail before callbacks or state writes.
 const int ids[]={arpIsOn,arpIsSequencer,arpDirection,arpOctave,arpInterval,arpTotalSteps,arpRate,arpSeqTempoMultiplier,sequencerPattern00,sequencerOctBoost00,sequencerNoteOn00};
 for(int id:ids)for(float bad:{std::numeric_limits<float>::quiet_NaN(),std::numeric_limits<float>::infinity(),-std::numeric_limits<float>::infinity()}){Pair t;t.p[id]=bad;t.bad();}
 {Pair t;t.count=129;t.bad();}{Pair t;t.held[0].noteNumber=-1;t.bad();}{Pair t;t.held[0].velocity=128;t.bad();}
 {Pair t;t.rate(0);t.bad();}{Pair t;t.rate(std::numeric_limits<double>::infinity());t.bad();}
 std::printf("{\"cases\":%llu,\"source_sample_calls\":%llu,\"callback_events\":%llu,\"transactional_rejects\":%llu,\"runtime_heap_allocations\":0,\"runtime_state_bytes\":%zu,\"failures\":0}\n",(unsigned long long)cases,(unsigned long long)samples,(unsigned long long)events,(unsigned long long)rejects,sizeof(SerializedSequence));
 return 0;
}
