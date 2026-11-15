#define main accepted_compact_suite_main
#include "compact_scheduler_test.cpp"
#undef main
#include "source_scheduler.hpp"
#include "full_oracle.hpp"
static Snapshot full_capture(const SourceScheduler&s){
 Snapshot z=capture(static_cast<const RuntimeState&>(s));const auto&q=s.sequence;
 z.put(q.beat_time);z.put(q.previous_held);z.put(q.get_beat_count());z.put(q.notes_per_octave());
 size_t n=q.last_count+q.initial_zero_count;z.put(n);
 for(unsigned i=0;i<q.last_count;i++)z.put(q.last_notes[i]);
 for(unsigned i=0;i<q.initial_zero_count;i++){int zero=0;z.put(zero);}
 return z;
}
static Snapshot full_capture(const FullOracle&s){
 Snapshot z=capture(static_cast<const OracleState&>(s));const auto&q=s.sequencer.original;
 z.put(q.mBeatTime);z.put(q.previousHeldNoteNumbersAECount);z.put(q.mStepCounter.load());z.put(q.mNotesPerOctave.load());
 size_t n=q.sequencerLastNotes.size();z.put(n);for(int note:q.sequencerLastNotes)z.put(note);
 return z;
}
struct FullPair{
 SourceScheduler s;FullOracle o;Fixture&f;float values[150];
 struct Context{FullPair*p;Trace*t;};Context sc{this,nullptr},oc{this,nullptr};
 FullPair(Fixture&fixture,int rate,bool mono,bool seq,int direction):f(fixture){
  std::memcpy(values,defaults,sizeof values);values[isMono]=mono?1:0;values[arpIsOn]=1;values[arpIsSequencer]=seq?1:0;values[arpDirection]=direction;
  values[arpRate]=200;values[arpSeqTempoMultiplier]=1.f/64.f/1.5f;values[arpOctave]=3;values[arpTotalSteps]=16;
  for(int i=0;i<16;i++){values[sequencerPattern00+i]=(i%13)*(i%2?-1:1);values[sequencerNoteOn00+i]=i%3?1:0;values[sequencerOctBoost00+i]=i%2;}
  Tables meta=f.tables;for(auto&r:meta.morph)r.samples=nullptr;
  CHECK(s.init_compact_metadata(meta,compact_data,compact_size,rate,values,150,f.tuning,128));CHECK(o.init(f.tables,rate,values,150,f.tuning,128));
  CHECK(s.using_compact());compare();
 }
 void compare(){equal(full_capture(s),full_capture(o));}
 void parameter(unsigned p,float v){{NoHeap guard;CHECK(s.set_target(p,v));}o._setSynthParameter(p,v);events++;compare();}
 void start(int n,int v){{NoHeap guard;CHECK(s.start(n,v));}o.startNote(n,v);events++;compare();}
 void stop(int n){{NoHeap guard;CHECK(s.stop(n));}o.stopNote(n);events++;compare();}
 void stop_all(){{NoHeap guard;CHECK(s.stop_all());}o.stopAllNotes();events++;compare();}
 void reset_dsp(){{NoHeap guard;CHECK(s.reset_dsp());}o.resetDSP();events++;compare();}
 void reset_seq(){{NoHeap guard;CHECK(s.reset_sequencer());}o.resetSequencer();events++;compare();}
 void restore(){{NoHeap guard;CHECK(s.restore(values,150));}CHECK(o.restore(values,150));events++;compare();}
 void npo(int n){s.set_notes_per_octave(n);o.sequencer.setNotesPerOctave(n);events++;compare();}
 void beat(unsigned k){double dt=(s.parameters[arpRate]/60.f)/s.sp->sr;double clock=k*s.parameters[arpSeqTempoMultiplier]-dt*.5;s.sequence.beat_time=o.sequencer.original.mBeatTime=clock;}
 static void st(const RuntimeState&,unsigned phase,unsigned frame,void*c){auto&x=*static_cast<Context*>(c);auto z=full_capture(x.p->s);z.put(phase);z.put(frame);x.t->values.push_back(z);}
 static void ot(const OracleState&,unsigned phase,unsigned frame,void*c){auto&x=*static_cast<Context*>(c);auto z=full_capture(x.p->o);z.put(phase);z.put(frame);x.t->values.push_back(z);}
 void block(unsigned n,unsigned offset=0){
  CHECK(n<=256&&offset<=7);float sl[264],sr[264],ol[264],orr[264];for(unsigned i=0;i<264;i++)sl[i]=sr[i]=ol[i]=orr[i]=123.25f;
  Trace a,b;a.values.reserve(n+1);b.values.reserve(n+1);sc.t=&a;oc.t=&b;
  s.trace_fn=st;s.trace_context=&sc;o.trace_fn=ot;o.trace_context=&oc;
  compact_oracle_window=true;o.process(n,offset,ol,orr);compact_oracle_window=false;
  {NoHeap guard;CHECK(s.process(n,offset,sl,sr));}
  s.trace_fn=nullptr;o.trace_fn=nullptr;s.trace_context=o.trace_context=nullptr;
  CHECK(a.values.size()==n+1&&b.values.size()==n+1);
  for(size_t i=0;i<a.values.size();i++)equal(a.values[i],b.values[i]);
  CHECK(!std::memcmp(sl,ol,sizeof sl)&&!std::memcmp(sr,orr,sizeof sr));
  for(unsigned i=0;i<n;i++){CHECK(std::isfinite(sl[offset+i]));CHECK(std::isfinite(sr[offset+i]));}
  frames+=n;compare();
 }
};
int main(int argc,char**argv){
 CHECK(argc==3);Fixture f(argv[1]);FILE*file=std::fopen(argv[2],"rb");CHECK(file);
 std::vector<unsigned char>pack(165292);CHECK(std::fread(pack.data(),1,pack.size(),file)==pack.size());CHECK(!std::fclose(file));compact_data=pack.data();compact_size=pack.size();

 // F1: a rejected PUBLIC tuning/control domain must not poison later render.
 // This catches accepting INT_MAX then rejecting after output/owner mutation.
 {
  case_name="public tuning domain rejection is transactional";++cases;
  FullPair p(f,48000,false,false,0);p.start(60,90);p.parameter(arpInterval,12);
  unsigned char before[sizeof p.s];std::memcpy(before,&p.s,sizeof p.s);
  CHECK(!p.s.set_notes_per_octave(std::numeric_limits<int>::max()));
  CHECK(!std::memcmp(before,&p.s,sizeof p.s));p.block(1);
 }
 {
  case_name="prospective control and restore checked against tuning domain";++cases;
  FullPair p(f,48000,false,true,1);
  for(unsigned i=0;i<16;i++){p.parameter(sequencerPattern00+i,0);p.parameter(sequencerOctBoost00+i,0);}
  p.parameter(arpInterval,0);p.npo(std::numeric_limits<int>::max());
  p.start(60,100);p.block(1);
  unsigned char before[sizeof p.s];std::memcpy(before,&p.s,sizeof p.s);
  CHECK(!p.s.set_target(arpInterval,12));
  CHECK(!std::memcmp(before,&p.s,sizeof p.s));
  CHECK(!p.s.set_target(sequencerPattern00,12));
  CHECK(!std::memcmp(before,&p.s,sizeof p.s));
  CHECK(!p.s.set_target(sequencerOctBoost00,1));
  CHECK(!std::memcmp(before,&p.s,sizeof p.s));
  CHECK(!p.s.restore(p.values,150));
  CHECK(!std::memcmp(before,&p.s,sizeof p.s));p.block(2);
 }

 for(int rate:{44100,48000,96000})for(bool mono:{false,true})for(bool seq:{false,true})for(int mode:{0,1,2}){
  case_name="full actual source seq/arp compact PCM-state composition";cases++;FullPair p(f,rate,mono,seq,mode);
  p.block(1);p.start(60,90);p.start(67,127);p.start(72,31);p.start(0,0);p.start(127,100);
  for(unsigned k=1;k<=32;k++){
   p.beat(k);p.block(1,3);p.block(64+(k%3)*32,k%7);
   if(k==4)p.npo(19);
   if(k==8){p.stop(67);p.start(60,111);p.parameter(transpose,-12);p.parameter(arpOctave,1);}
   if(k==12){p.parameter(arpIsOn,0);p.stop(127);p.block(13);p.parameter(arpIsOn,1);}
   if(k==16){p.parameter(isMono,mono?0:1);p.parameter(arpIsSequencer,seq?0:1);p.parameter(arpTotalSteps,7);p.parameter(arpDirection,(mode+1)%3);}
   if(k==20){p.reset_seq();p.npo(0);p.parameter(arpSeqTempoMultiplier,.25f);p.parameter(arpRate,1);}
   if(k==24){p.restore();p.start(48,45);p.start(55,88);}
   if(k==28){p.stop_all();p.block(2);p.start(36,127);}
  }
  p.reset_dsp();p.block(3);p.restore();p.start(61,90);p.block(17);
  // Whole-final-owner aliases and invalid cold controls preserve the entire live object.
  unsigned char before[sizeof p.s];std::memcpy(before,&p.s,sizeof p.s);Tables meta=f.tables;for(auto&r:meta.morph)r.samples=nullptr;
  float left=42,right=43,bad[150];std::memcpy(bad,p.values,sizeof bad);bad[arpRate]=std::numeric_limits<float>::quiet_NaN();
  {NoHeap guard;CHECK(!p.s.init_compact_metadata(meta,compact_data,compact_size,rate,bad,150,f.tuning,128));CHECK(!p.s.restore(bad,150));CHECK(!p.s.process(1,0,reinterpret_cast<float*>(&p.s.sequence),&right));CHECK(!p.s.process(1,0,&left,&left));}
  CHECK(!std::memcmp(before,&p.s,sizeof p.s));CHECK(left==42&&right==43);
 }
 std::printf("{\"scope\":\"full-source-sequence-arp-pre-FX-compact-composition\",\"cases\":%u,\"checks\":%llu,\"frames\":%llu,\"state_snapshots\":%llu,\"control_note_events\":%llu,\"heap_calls_during_runtime_actions\":0,\"source_scheduler_host_bytes\":%zu,\"failures\":0}\n",cases,checks,frames,snapshots,events,sizeof(SourceScheduler));return 0;
}
