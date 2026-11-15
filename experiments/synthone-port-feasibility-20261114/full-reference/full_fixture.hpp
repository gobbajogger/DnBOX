#define main accepted_compact_suite_main
#include "tests/compact_scheduler_test.cpp"
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
