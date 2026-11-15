// Host-only mechanical full-sequence oracle binding. Never linked into target.
#pragma once
#include "oracle/state.hpp"
#include "source-sequence/original_sequence.hpp"
namespace s1scheduler {
struct OriginalSequenceAdapter{
 std::function<void(int,int)> on;std::function<void(int)> off;std::function<void()> beat;
 s1source_oracle::OriginalSequence original;
 OriginalSequenceAdapter():original([&](int n,int v){on(n,v);},[&](int n){off(n);},[&](){beat();}){}
 void init(){original.init();}void reset(bool x){original.reset(x);}
 void setSampleRate(double x){original.setSampleRate(x);}void setNotesPerOctave(int x){original.setNotesPerOctave(x);}
 int getArpBeatCount(){return original.getArpBeatCount();}
 void process(float*p,const Held&h){
  s1source_oracle::DSPParameters values;std::memcpy(values.data(),p,150*sizeof(float));
  s1source_sequence::HeldNote notes[128];
  for(int i=0;i<h.count;i++)notes[i]={h[i].noteNumber,h[i].transpose,h[i].velocity};
  original.process(values,{notes,static_cast<size_t>(h.count)});
 }
};
class FullOracleState:public OracleState{
public:
 OriginalSequenceAdapter sequencer;
protected:
 bool init_oracle(const synthone_shared::Tables&t,int rate,const float*v,size_t n,const float*tuning,size_t tn,const bool(&flags)[150],InstanceSpan self){
  if(!valid_init(t,rate,v,n,tuning,tn,self,true))return false;
  init_graph(t,rate,tuning);sequencer.setSampleRate(rate);sequencer.init();
  for(unsigned i=0;i<150;i++){reference_ports_[i]={};s1p[i]={};s1p[i].usePortamento=flags[i];s1p[i].portamento=&reference_ports_[i];}
  return restore_oracle(v,n,self);
 }
 bool restore_oracle(const float*v,size_t n,InstanceSpan self){
  if(!valid_restore(v,n,self,true))return false;
  float saved[150];std::memcpy(saved,v,sizeof saved);std::memcpy(parameters,saved,sizeof saved);
  for(unsigned i=0;i<150;i++)if(s1p[i].usePortamento){s1p[i].portamentoTarget=saved[i];sp_port_init(sp,s1p[i].portamento,saved[i]);s1p[i].portamento->htime=saved[portamentoHalfTime];}
  restore_graph();sequencer.init();return true;
 }
};
}
