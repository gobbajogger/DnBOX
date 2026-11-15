#include "preset_executor.hpp"
#include <cmath>
#include <limits>
namespace original_conductor_v1 {
namespace {
bool readable_span(const void* p,std::size_t n,std::size_t width)noexcept{
 const auto max=std::numeric_limits<std::uintptr_t>::max();const auto a=reinterpret_cast<std::uintptr_t>(p);
 if(n&&(!p||n>max/width)){return false;}
 return n*width<=max-a;
}
bool present(const Record* r,std::size_t n,MainBinding binding,float& value)noexcept{
 for(std::size_t i=0;i<n;i++){if(r[i].identifier==binding.name.data()){value=static_cast<float>(r[i].value);return true;}}
 return false;
}
Command action(Op op,std::uint16_t id,float value,std::uint64_t gen,std::size_t order)noexcept{return {op,Domain::NormalizedControl,Receiver::InstrumentControl,id,double(value),gen,order,op==Op::DeferVolume?.15:0.,0};}
}
PresetResult execute_main(const Record* records,std::size_t nr,std::uint64_t generation,bool immediate,Providers p,PresetBackend backend)noexcept{
 if(nr>4096||!readable_span(records,nr,sizeof(Record))||(nr&&(reinterpret_cast<std::uintptr_t>(records)%alignof(Record))))return {PresetStatus::Invalid,0,0,218};
 for(std::size_t i=0;i<nr;i++)if(!readable_span(records[i].identifier.data(),records[i].identifier.size(),1)||!std::isfinite(records[i].value)||std::fabs(records[i].value)>std::numeric_limits<float>::max())return {PresetStatus::Invalid,0,0,218};
 // Conservative complete potential-capability preflight avoids any effect when
 // the adapter cannot execute an original branch. This is intentionally stricter
 // than the atomic planner's value-dependent capability check.
 const unsigned required=ControlApply|DelayPrepare|(immediate?0U:unsigned(DeferredVolume));
 if(!backend.execute||(backend.capabilities&required)!=required)return {PresetStatus::MissingCapability,0,0,218};
 for(std::size_t i=0;i<170;i++){
  MainBinding b{};if(main_binding(i,b)!=Status::State)return {PresetStatus::Invalid,0,0,218};if(b.id>=212&&b.id<=214)continue;
  float ignored;if(!present(records,nr,b,ignored)&&b.derived_default&&!p.pow)return {PresetStatus::MissingMath,0,0,b.id};
 }
 std::size_t effects=0,completed=0;
 for(std::size_t i=0;i<170;i++){
  MainBinding b{};main_binding(i,b);if(b.id>=212&&b.id<=214)continue;
  float value=0;if(!present(records,nr,b,value)&&query_default(b.id,p,value)!=Status::State)return {PresetStatus::PartialMathFailure,effects,completed,b.id};
  if(b.id==24&&value>0){Command c=action(Op::DelayEnablePrepare,b.id,value,generation,effects++);if(!backend.execute(backend.context,c))return {PresetStatus::PartialBackendFailure,effects,completed,b.id};}
  Command c=action(b.id==0&&!immediate?Op::DeferVolume:Op::ApplyControl,b.id,value,generation,effects++);
  if(!backend.execute(backend.context,c))return {PresetStatus::PartialBackendFailure,effects,completed,b.id};
  ++completed;
 }
 return {PresetStatus::CompletedMainPassEffectsOnly,effects,completed,218};
}
}

namespace original_conductor_v1 {
PresetResult execute_pwm_postpass(const Record* records,std::size_t nr,std::uint64_t generation,PwmPostpassSnapshot captured,Providers p,PresetBackend backend)noexcept{
 if(nr>4096||!readable_span(records,nr,sizeof(Record))||(nr&&(reinterpret_cast<std::uintptr_t>(records)%alignof(Record))))return {PresetStatus::Invalid,0,0,218};
 for(std::size_t i=0;i<nr;i++)if(!readable_span(records[i].identifier.data(),records[i].identifier.size(),1)||!std::isfinite(records[i].value)||std::fabs(records[i].value)>std::numeric_limits<float>::max())return {PresetStatus::Invalid,0,0,218};
 const std::size_t positions[4]={132,134,139,141};bool selected[4];MainBinding bindings[4];bool any=false;
 for(unsigned i=0;i<4;i++){selected[i]=(captured.route[i]&0xfeU)==78;any=any||selected[i];if(main_binding(positions[i],bindings[i])!=Status::State)return {PresetStatus::Invalid,0,0,218};}
 if(any&&(!backend.execute||(backend.capabilities&ControlApply)==0))return {PresetStatus::MissingCapability,0,0,218};
 for(unsigned i=0;i<4;i++)if(selected[i]){float ignored;if(!present(records,nr,bindings[i],ignored)&&bindings[i].derived_default&&!p.pow)return {PresetStatus::MissingMath,0,0,bindings[i].id};}
 std::size_t effects=0,completed=0;
 for(unsigned i=0;i<4;i++)if(selected[i]){const auto&b=bindings[i];float value=0;if(!present(records,nr,b,value)&&query_default(b.id,p,value)!=Status::State)return {PresetStatus::PartialMathFailure,effects,completed,b.id};Command c=action(Op::ApplyControl,b.id,value,generation,effects++);if(!backend.execute(backend.context,c))return {PresetStatus::PartialBackendFailure,effects,completed,b.id};++completed;}
 return {PresetStatus::CompletedPwmPostpassEffectsOnly,effects,completed,218};
}
}
