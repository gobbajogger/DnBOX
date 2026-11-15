#include "cutoff_executor.hpp"
#include <cmath>
#include <cstring>
namespace original_conductor_v1 {
namespace {
float sub(float a,float b)noexcept{volatile float x=a-b;return x;}
float mul(float a,float b)noexcept{volatile float x=a*b;return x;}
float add(float a,float b)noexcept{volatile float x=a+b;return x;}
float clamp(float x)noexcept{if(x<0)return 0;if(x>1)return 1;return x;}
float epsilon()noexcept{std::uint32_t u=0x3a03126f;float f;std::memcpy(&f,&u,4);return f;}
}
CutoffResult execute_cutoff(RuntimeCutoff& state,double dt,Providers p,CutoffBackend backend)noexcept{
 if(!std::isfinite(state.current)||!std::isfinite(state.target)||!std::isfinite(dt))return {CutoffStatus::Invalid,0};
 if(!state.active)return {CutoffStatus::Noop,0};
 if(!backend.apply_core||!backend.post_modulation||!backend.read_core||(dt>0&&!p.exp))return {CutoffStatus::MissingCapability,0};
 const float captured=clamp(state.target);float emitted=captured;
 if(dt>0){double exponent=p.exp(dt/-.015);if(!std::isfinite(exponent)||exponent<0||exponent>1)return {CutoffStatus::MathFailure,0};float alpha=static_cast<float>(1.-exponent);emitted=add(state.current,mul(sub(captured,state.current),alpha));if(!std::isfinite(emitted))return {CutoffStatus::MathFailure,0};emitted=clamp(emitted);}
 state.current=emitted;
 if(!backend.apply_core(backend.context,state.current))return {CutoffStatus::PartialFailure,1};
 if(!std::isfinite(state.current))return {CutoffStatus::PartialFailure,1};
 if(!backend.post_modulation(backend.context,7,state.current))return {CutoffStatus::PartialFailure,2};
 if(!std::isfinite(state.current))return {CutoffStatus::PartialFailure,2};
 if(std::fabs(sub(state.current,captured))<=epsilon()){
  state.current=captured;float actual;
  if(!backend.read_core(backend.context,actual)||!std::isfinite(actual))return {CutoffStatus::PartialFailure,2};
  if(std::fabs(sub(captured,actual))<=epsilon())state.active=false;
 }
 return {CutoffStatus::CompletedEffectsOnly,2};
}
}
