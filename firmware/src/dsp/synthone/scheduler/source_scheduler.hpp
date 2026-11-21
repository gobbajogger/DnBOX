// Full original source sequence/arp + compact pre-FX engine composition.
// Isolated experiment, not firmware admission. See SOURCE-SEQUENCE-CONTRACT.md.
#pragma once
#include "scheduler.hpp"
#include "source-sequence/source_sequence.hpp"
namespace s1scheduler {
class SourceScheduler final:public RuntimeState {
public:
 SourceScheduler()=default;
 SourceScheduler(const SourceScheduler&)=delete;SourceScheduler&operator=(const SourceScheduler&)=delete;
 SourceScheduler(SourceScheduler&&)=delete;SourceScheduler&operator=(SourceScheduler&&)=delete;
 s1source_sequence::SerializedSequence sequence{};
 bool init_compact_metadata(const synthone_shared::Tables&t,const void*p,size_t bytes,int rate,const float*v,size_t n,const float*tuning,size_t tn){
  if(!init_compact_runtime(t,p,bytes,rate,v,n,tuning,tn,{this,sizeof *this},true))return false;
  sequence={};sequence.set_sample_rate(rate);sequence.init();return true;
 }
#if defined(S1_HARMONIC_ONLY)
 bool init_harmonic_metadata(const synthone_shared::Tables&t,int rate,const float*v,size_t n,const float*tuning,size_t tn){
  if(!init_harmonic_runtime(t,rate,v,n,tuning,tn,{this,sizeof *this},true))return false;
  sequence={};sequence.set_sample_rate(rate);sequence.init();return true;
 }
#endif
#if defined(S1_HARMONIC_ONLY) && defined(S1_CANONICAL_SINE)
 bool init_harmonic_canonical_metadata(const synthone_shared::Tables&t,int rate,const float*v,size_t n,const float*tuning,size_t tn){
  if(!init_harmonic_runtime(t,rate,v,n,tuning,tn,{this,sizeof *this},true,true))return false;
  sequence={};sequence.set_sample_rate(rate);sequence.init();return true;
 }
#endif
 bool restore(const float*v,size_t n){
  if(!initialized_ || (reinterpret_cast<uintptr_t>(v)%alignof(float)) ||
     !valid_restore(v,n,{this,sizeof *this},true) ||
     !s1source_sequence::SerializedSequence::tuning_valid(v,n,sequence.notes_per_octave()))return false;
  if(!restore_runtime(v,n,{this,sizeof *this},true))return false;sequence.init();return true;
 }
 bool valid_output(float*l,float*r,unsigned n,unsigned offset)const{
  return valid_runtime_output(l,r,n,offset,{this,sizeof *this},true);
 }
 bool set_target(unsigned,float);
 bool start(int,int,float=1.f);
 bool on(int,int,float);bool on(int,int);
 bool stop(int);bool off(int);bool stop_all();bool reset();bool reset_dsp();bool reset_sequencer();
 bool process(unsigned,unsigned,float*,float*);
 // Checked public tuning boundary: false preserves the entire live owner.
 bool set_notes_per_octave(int n);
 int beat_count()const{return sequence.get_beat_count();}
private:
 bool valid_source_event(int,int,float)const;
 void on_valid(int,int,float);void off_valid(int);
 struct Sink{
  SourceScheduler&s;
  void on(int n,int v){s.on_valid(n,v,s.tuningTableNoteToHz(n+static_cast<int>(s.parameters[transpose])));}
  void off(int n){s.off_valid(n);}
  void beat(){s.beatCounterDidChange();}
 };
};
}
