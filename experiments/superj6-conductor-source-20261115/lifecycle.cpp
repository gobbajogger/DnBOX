#include "lifecycle.hpp"
#include <cmath>
namespace original_conductor_v1 {
namespace {
LifecycleResult empty(LifecycleStatus status)noexcept{return {status,{OrderedStatus::NotLoaded,OrderedStage::None,0}};}
bool valid(BeatUpdate beat)noexcept{return !beat.supplied||(std::isfinite(beat.finite_nonnegative)&&beat.finite_nonnegative>=0);}
LifecycleResult tick_zero(Lifecycle& s,const OrderedInput& input,OrderedBackend b)noexcept{OrderedInput in=input;in.prefix.dt=0.;OrderedResult r=execute_ordered_tick(s.tick,in,b);return {r.status==OrderedStatus::CompletedEffectsOnly||r.status==OrderedStatus::NotLoaded?LifecycleStatus::CompletedEffectsOnly:LifecycleStatus::PartialFailure,r};}
}
LifecycleResult full_reset(Lifecycle& s,BeatUpdate beat,const OrderedInput& in,OrderedBackend b)noexcept{
 if(!valid(beat))return empty(LifecycleStatus::Invalid);
 if(!b.restart.restart_collection_and_noise)return empty(LifecycleStatus::MissingCapability);
 for(unsigned i=0;i<4;i++)s.tick.times.current[i]=0.;
 for(unsigned i=0;i<3;i++){s.tick.phases[i]=0.;s.tick.apply.cycle_complete[i]=false;s.tick.held_elapsed[i]=0.;s.tick.held[i]=0.;}
 s.tick.vibrato.cycle_complete=false;s.tick.apply.armed=true;s.new_note=false;
 if(beat.supplied){s.last_beat=beat.finite_nonnegative;}
 s.beat_accumulator=0.;
 if(!b.restart.restart_collection_and_noise(b.restart.context))return empty(LifecycleStatus::PartialFailure);
 return tick_zero(s,in,b);
}
LifecycleResult selective_retrigger(Lifecycle& s,BeatUpdate beat,const OrderedInput& in,OrderedBackend b)noexcept{
 if(!valid(beat))return empty(LifecycleStatus::Invalid);
 if(!s.new_note||!(s.retrigger[0]||s.retrigger[1]||s.retrigger[2]))return empty(LifecycleStatus::Noop);
 s.new_note=false;if(beat.supplied){s.last_beat=beat.finite_nonnegative;}
 s.beat_accumulator=0.;
 for(unsigned i=0;i<3;i++)if(s.retrigger[i]){s.tick.times.current[i]=0.;s.tick.phases[i]=0.;s.tick.apply.cycle_complete[i]=false;}
 if(s.retrigger[3]){s.tick.times.current[3]=0.;s.tick.vibrato.cycle_complete=false;}
 return tick_zero(s,in,b);
}
}
