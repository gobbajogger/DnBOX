#include "preset_executor.hpp"
#include "conductor.hpp"
#include "control_data.hpp"
#include <cmath>
#include <cstring>
#include <limits>
#include <algorithm>
namespace original_conductor_v1 {
bool Identifier::operator==(const char* s)const noexcept{return length==std::strlen(s)&&(length==0||std::memcmp(pointer,s,length)==0);}
static_assert(sizeof(float)==4 && std::numeric_limits<float>::digits==24 && sizeof(double)==8 && std::numeric_limits<double>::digits==53,"IEEE source widths required");
namespace {
struct Span {std::uintptr_t first,last;bool valid;};
Span span(const void* p,std::size_t count,std::size_t width) noexcept {
 auto a=reinterpret_cast<std::uintptr_t>(p);auto max=std::numeric_limits<std::uintptr_t>::max();
 if(count && (!p || count>max/width))return {0,0,false};
 auto n=count*width;if(n>max-a)return {0,0,false};return {a,a+n,true};
}
bool overlaps(Span a,Span b) noexcept{return a.first<a.last&&b.first<b.last&&a.first<b.last&&b.first<a.last;}
bool f32(double x) noexcept{return std::isfinite(x)&&std::fabs(x)<=std::numeric_limits<float>::max();}
float bits(std::uint32_t x)noexcept{float y;std::memcpy(&y,&x,4);return y;}
float fsub(float a,float b)noexcept{volatile float x=a-b;return x;}
float fmul(float a,float b)noexcept{volatile float x=a*b;return x;}
float fadd(float a,float b)noexcept{volatile float x=a+b;return x;}
float clamp(float v)noexcept{if(v<0.f)return 0.f;if(v>1.f)return 1.f;return v;}
float epsilon()noexcept{return bits(0x3a03126f);}
double dbits(std::uint64_t u)noexcept{double v;std::memcpy(&v,&u,8);return v;}
Status default_value(unsigned id,Providers p,float& out)noexcept{
 const auto index=default_expression_index[id];if(!index){out=bits(default_literal_bits[id]);return Status::State;}
 const auto& d=default_expressions[index-1];
 if(!p.pow)return Status::MissingPow;
 double x=p.pow((dbits(d.input_bits)-dbits(d.lo_bits))/(dbits(d.hi_bits)-dbits(d.lo_bits)),1./dbits(d.taper_bits));if(!f32(x)||x<0.||x>1.)return Status::MathFailure;
 out=static_cast<float>(x);return Status::State;
}
Command command(Op op,Receiver r,std::uint16_t id,double value,std::uint64_t gen,std::size_t order,double delay=0.,std::uint64_t token=0)noexcept{
 return {op,op==Op::TickLfo?Domain::ElapsedSeconds:Domain::NormalizedControl,r,id,value,gen,order,delay,token};
}
bool state_spans(const void* input,void* result,std::size_t width,Command* out,std::size_t cap,std::size_t* count)noexcept{
 auto a=span(input,1,width),b=span(result,1,width),c=span(out,cap,sizeof(Command)),d=span(count,1,sizeof(*count));
 return a.valid&&b.valid&&c.valid&&d.valid&&!overlaps(c,a)&&!overlaps(c,b)&&!overlaps(c,d)&&!overlaps(d,a)&&!overlaps(d,b)&&(!overlaps(a,b)||input==result);
}
}
Status query_default(std::uint16_t id,Providers providers,float& out) noexcept{
 if(id>=218)return Status::Invalid;
 float value;auto status=default_value(id,providers,value);if(status==Status::State)out=value;return status;
}
Status plan_main(const Record* records,std::size_t nr,std::uint64_t gen,bool immediate,Providers providers,Command* out,std::size_t capacity,std::size_t& count)noexcept{
 if(nr>4096)return Status::Invalid;
 auto in=span(records,nr,sizeof(Record)),dest=span(out,capacity,sizeof(Command)),cnt=span(&count,1,sizeof(count));
 if(!in.valid||!dest.valid||!cnt.valid||overlaps(in,dest)||overlaps(in,cnt)||overlaps(dest,cnt))return Status::Invalid;
 for(std::size_t i=0;i<nr;++i){auto str=span(records[i].identifier.data(),records[i].identifier.size(),1);if(!str.valid||overlaps(str,dest)||overlaps(str,cnt)||!f32(records[i].value))return Status::Invalid;}
 Command tmp[171];std::size_t used=0;unsigned required=ControlApply;
 for(const auto& binding:order_data){
  if(binding.id>=212&&binding.id<=214)continue;
  float value=0;bool found=false;
  for(std::size_t j=0;j<nr;++j)if(records[j].identifier==binding.name){value=static_cast<float>(records[j].value);found=true;break;}
  if(!found){auto st=default_value(binding.id,providers,value);if(st!=Status::State)return st;}
  if(binding.id==24&&value>0){required|=DelayPrepare;tmp[used]=command(Op::DelayEnablePrepare,Receiver::InstrumentControl,binding.id,value,gen,used);++used;}
  const bool defer=binding.id==0&&!immediate;if(defer)required|=DeferredVolume;
  tmp[used]=command(defer?Op::DeferVolume:Op::ApplyControl,Receiver::InstrumentControl,binding.id,value,gen,used,defer?.15:0.);++used;
 }
 if(used>capacity)return Status::Capacity;
 if((providers.capabilities&required)!=required)return Status::MissingCapability;
 std::copy(tmp,tmp+used,out);count=used;return Status::MainPassOnly;
}
Status cutoff_step(const Cutoff& s,double dt,Providers p,Command* out,std::size_t capacity,std::size_t& count,Cutoff& result)noexcept{
 if(!state_spans(&s,&result,sizeof(s),out,capacity,&count)||!std::isfinite(dt)||!std::isfinite(s.current)||!std::isfinite(s.target)||s.token==UINT64_MAX)return Status::Invalid;
 if(s.awaiting)return Status::AwaitingAck;
 if(!s.active){result=s;count=0;return Status::State;}
 if(capacity<2)return Status::Capacity;
 if((p.capabilities&(CutoffApply|GlobalDispatch))!=(CutoffApply|GlobalDispatch))return Status::MissingCapability;
 float target=clamp(s.target),current=target;
 if(dt>0){if(!p.exp)return Status::MissingExp;double e=p.exp(dt/-.015);if(!std::isfinite(e)||e<0.||e>1.||!f32(1.-e))return Status::MathFailure;float alpha=static_cast<float>(1.-e);current=fadd(s.current,fmul(fsub(target,s.current),alpha));if(!std::isfinite(current))return Status::MathFailure;}
 float emitted=clamp(current);Cutoff next=s;next.current=std::fabs(fsub(emitted,target))<=epsilon()?target:emitted;next.target=target;next.token++;next.awaiting=true;
 Command tmp[2]={command(Op::CutoffApply,Receiver::CoreCutoff,7,emitted,s.generation,0,0.,next.token),command(Op::GlobalDispatch,Receiver::ConductorGlobal,7,emitted,s.generation,1,0.,next.token)};
 std::copy(tmp,tmp+2,out);result=next;count=2;return Status::PendingAck;
}
Status cutoff_ack(const Cutoff& s,std::uint64_t gen,std::uint64_t token,float actual,Cutoff& result)noexcept{
 if(!std::isfinite(actual)||!std::isfinite(s.current)||!std::isfinite(s.target))return Status::Invalid;
 auto a=span(&s,1,sizeof(s)),b=span(&result,1,sizeof(result));if(!a.valid||!b.valid||(overlaps(a,b)&&&s!=&result))return Status::Invalid;
 if(!s.awaiting||gen!=s.generation||token!=s.token)return Status::StaleAck;
 Cutoff next=s;next.active=!(std::fabs(fsub(s.current,s.target))<=epsilon()&&std::fabs(fsub(actual,s.target))<=epsilon());next.awaiting=false;result=next;return Status::State;
}
Status free_block_time(std::uint32_t frames,double sample_rate,double& dt)noexcept{
 if(frames>INT32_MAX||!std::isfinite(sample_rate)||sample_rate<=0)return Status::Invalid;
 double value=double(frames)/sample_rate;if(!std::isfinite(value))return Status::Invalid;dt=value;return Status::Clock;
}
Status beat_catchup(const Beat& s,double beat,double tempo,bool loaded,Command* out,std::size_t capacity,std::size_t& count,Beat& result)noexcept{
 if(!state_spans(&s,&result,sizeof(s),out,capacity,&count)||!std::isfinite(s.last)||!std::isfinite(s.accumulator)||s.accumulator<0||s.accumulator>=.0125||!std::isfinite(beat)||!std::isfinite(tempo))return Status::Invalid;
 Beat next=s;next.last=beat;Command tmp[17];std::size_t used=0;
 if(!loaded||beat<s.last)next.accumulator=0.;
 else{
  double elapsed=((beat-s.last)*60.)/std::max(tempo,1.);
  if(std::isfinite(elapsed)&&elapsed>0){
   if(elapsed>.2)next.accumulator=0.;
   else{next.accumulator+=elapsed;while(next.accumulator>=.0125){if(used==capacity)return Status::Capacity;if(used==17)return Status::Invalid;tmp[used]=command(Op::TickLfo,Receiver::Conductor,0,.0125,s.generation,used);++used;next.accumulator=next.accumulator+(-.0125);}}
  }
 }
 if(used){std::copy(tmp,tmp+used,out);}
 result=next;count=used;return Status::Clock;
}
Status enter_note(const Note& s,std::size_t held,Note& result)noexcept{
 auto a=span(&s,1,sizeof(s)),b=span(&result,1,sizeof(result));if(!a.valid||!b.valid||(overlaps(a,b)&&&s!=&result))return Status::Invalid;
 for(unsigned i=0;i<3;++i)if(s.modes[i]>3||!std::isfinite(s.delay_passed[i])||s.delay_passed[i]<0)return Status::Invalid;
 Note next=s;next.armed=true;next.retrigger4=true;
 for(unsigned i=0;i<3;++i){if(s.modes[i]==2)next.retriggers[i]=true;if(!held){next.delay_finished[i]=false;next.delay_passed[i]=0.;}}
 if(!held){next.new_note=true;}
 result=next;return Status::State;
}
}
namespace original_conductor_v1 {
Status restore(const RestoreState& s,bool forced,const RestoreControl* controls,std::size_t nc,unsigned capabilities,RestoreCommand* out,std::size_t capacity,std::size_t& count,RestoreState& result) noexcept{
 if(nc>218)return Status::Invalid;
 auto a=span(&s,1,sizeof(s)),b=span(&result,1,sizeof(result)),c=span(controls,nc,sizeof(*controls)),d=span(out,capacity,sizeof(*out)),e=span(&count,1,sizeof(count));
 if(!a.valid||!b.valid||!c.valid||!d.valid||!e.valid||(overlaps(a,b)&&&s!=&result)||overlaps(d,a)||overlaps(d,b)||overlaps(d,c)||overlaps(d,e)||overlaps(e,a)||overlaps(e,b)||overlaps(e,c)||overlaps(b,c))return Status::Invalid;
 for(std::size_t i=0;i<nc;i++)if(controls[i].parameter>218||controls[i].global>218)return Status::Invalid;
 for(const auto& route:s.routes)for(unsigned j=0;j<2;j++)if(route.destination[j]>218||!std::isfinite(route.base[j]))return Status::Invalid;
 RestoreState next=s;RestoreCommand tmp[12];std::size_t used=0;unsigned required=0;
 for(unsigned i=0;i<3;i++){
  const auto& route=s.routes[i];if(forced?i!=2:!(route.enabled&&route.retrigger&&route.snapback))continue;
  next.cycles[i]=true;
  for(unsigned j=0;j<2;j++){
   std::size_t k=0;while(k<nc&&controls[k].parameter!=route.destination[j])++k;if(k==nc)continue;
   const bool cutoff=controls[k].parameter==7;required|=cutoff?SetCutoffTarget:SetParameterFlagZero;
   tmp[used]={cutoff?RestoreCommand::Kind::CutoffTarget:RestoreCommand::Kind::ParameterValue,k,controls[k].parameter,route.base[j],s.generation,used};++used;
   if(controls[k].global!=218){required|=DispatchGlobal;tmp[used]={RestoreCommand::Kind::GlobalDispatch,k,controls[k].global,route.base[j],s.generation,used};++used;}
  }
 }
 if(used>capacity)return Status::Capacity;
 if((capabilities&required)!=required)return Status::MissingCapability;
 if(used)std::copy(tmp,tmp+used,out);
 result=next;count=used;return Status::State;
}
}
namespace original_conductor_v1 {
DeferredStatus execute_deferred_volume(bool alive,std::uint64_t captured,float value,DeferredBackend b) noexcept{
 if(!alive)return DeferredStatus::ExpiredOwner;
 if(!std::isfinite(value))return DeferredStatus::Invalid;
 if(!b.lock||!b.generation||!b.apply_volume||!b.clear_pending||!b.unlock)return DeferredStatus::MissingCapability;
 if(!b.lock(b.context))return DeferredStatus::LockFailed;
 if(b.generation(b.context)!=captured){b.unlock(b.context);return DeferredStatus::StaleGeneration;}
 if(!b.apply_volume(b.context,value)){b.unlock(b.context);return DeferredStatus::ApplyFailed;}
 b.clear_pending(b.context);b.unlock(b.context);return DeferredStatus::Applied;
}
}

namespace original_conductor_v1 {
Status main_binding(std::size_t index,MainBinding& out)noexcept{
 if(index>=170)return Status::Invalid;
 const Binding& b=order_data[index];MainBinding next{b.id,Identifier(b.name,std::strlen(b.name)),default_expression_index[b.id]!=0};out=next;return Status::State;
}
}
