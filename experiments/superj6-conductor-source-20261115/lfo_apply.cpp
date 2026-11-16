#include "lfo_apply.hpp"
#include <cmath>
namespace original_conductor_v1 {
namespace {
bool valid(const LiveLfo& c)noexcept{
 if(c.mode>3||c.waveform>5||!std::isfinite(c.rate)||c.rate<0||!std::isfinite(c.amount)||c.amount<0||c.amount>1)return false;
 for(unsigned i=0;i<2;i++)if(!std::isfinite(c.bases[i])||c.destinations[i]>218)return false;
 return true;
}
float add_base(float base,float delta)noexcept{float x=delta+float(base);if(x<0)x=0;if(x>1)x=1;return static_cast<float>(x);}
}
ApplyResult apply_tick_lfo(unsigned i,const float (&row)[6],ApplyState& state,ApplyBackend backend)noexcept{
 if(i>=3||!std::isfinite(state.current_time[i])||state.current_time[i]<0)return {ApplyStatus::Invalid,0,false};
 for(float x:row)if(!std::isfinite(x)||x< -1||x>1)return {ApplyStatus::Invalid,0,false};
 if(!backend.read_live)return {ApplyStatus::MissingCapability,0,false};
 LiveLfo initial;if(!backend.read_live(backend.context,i,initial))return {ApplyStatus::ReadFailure,0,false};
 if(!valid(initial))return {ApplyStatus::Invalid,0,false};
 if(i==0&&initial.amount<=.01f)return {ApplyStatus::Noop,0,false};
 if((initial.mode>=2&&!state.armed)||!initial.enabled||(i!=1&&!initial.delay_finished)||(initial.mode!=0&&state.held_notes==0&&initial.snapback)||state.cycle_complete[i])return {ApplyStatus::Noop,0,false};
 if(!backend.resolve_first||!backend.set_parameter_flag_zero||!backend.set_cutoff_target||!backend.dispatch_global)return {ApplyStatus::MissingCapability,0,false};
 bool changed=false;
 if(initial.mode>=2&&state.current_time[i]>=1.f/float(initial.rate)){state.cycle_complete[i]=true;changed=true;}
 LiveLfo current;if(!backend.read_live(backend.context,i,current)||!valid(current))return {ApplyStatus::PartialFailure,0,changed};
 if(!current.enabled)return {ApplyStatus::Noop,0,changed};
 float wave=row[current.waveform];
 if(current.mode>=2&&state.cycle_complete[i])wave=current.waveform==1?1.f:current.waveform==2?-1.f:0.f;
 const float delta=current.waveform==1?current.amount*((wave+1.f)*.5f):current.waveform==2?current.amount*((wave-1.f)*.5f):(wave*current.amount)*.5f;
 unsigned effects=0;
 for(unsigned route=0;route<(i==0?1U:2U);route++){
  if(route&&(!backend.read_live(backend.context,i,current)||!valid(current)))return {ApplyStatus::PartialFailure,effects,changed};
  const std::uint16_t destination=current.destinations[route];const float value=add_base(current.bases[route],delta);
  ResolvedControl control{};ResolveStatus found=backend.resolve_first(backend.context,destination,control);
  if(found==ResolveStatus::Missing)continue;
  if(found!=ResolveStatus::Found||control.parameter!=destination||control.global>218)return {ApplyStatus::PartialFailure,effects,changed};
  ++effects;bool ok=destination==7?backend.set_cutoff_target(backend.context,control,value):backend.set_parameter_flag_zero(backend.context,control,value);
  if(!ok)return {ApplyStatus::PartialFailure,effects,changed};
  if(control.global!=218){++effects;if(!backend.dispatch_global(backend.context,control.global,value))return {ApplyStatus::PartialFailure,effects,changed};}
 }
 return {ApplyStatus::AppliedCommandsOnly,effects,changed};
}
}
