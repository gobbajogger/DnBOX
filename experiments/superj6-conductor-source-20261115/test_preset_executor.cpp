#include "preset_executor.hpp"
#include <cmath>
#include <cstdio>
#include <cstring>
#include <vector>
#include <string>
using namespace original_conductor_v1;
static unsigned pow_count,fail_pow;static std::string events;
static double power(double a,double b)noexcept{++pow_count;events+='P';if(pow_count==fail_pow)return NAN;return std::pow(a,b);}
struct Fake{std::vector<Command> commands;unsigned fail_effect;};
static bool effect(void*p,const Command&c)noexcept{Fake&f=*static_cast<Fake*>(p);events+='E';f.commands.push_back(c);return f.commands.size()!=f.fail_effect;}
static std::uint64_t bits(double x){std::uint64_t u;std::memcpy(&u,&x,8);return u;}
static bool same(const Command&a,const Command&b){return a.op==b.op&&a.domain==b.domain&&a.receiver==b.receiver&&a.id==b.id&&bits(a.value)==bits(b.value)&&a.generation==b.generation&&a.order==b.order&&bits(a.delay)==bits(b.delay)&&a.token==b.token;}
int main(){unsigned n=0,bad=0;
#define C(x) do{++n;if(!(x)){++bad;std::fprintf(stderr,"FAIL %d %s\n",__LINE__,#x);}}while(0)
Record records[]={{"neonvolumeControl",1.32},{"neondelayEnable",1},{"neonvolumeControl",.1},{"unknown",42}};
for(unsigned mode=0;mode<2;mode++)for(unsigned populated=0;populated<2;populated++){
 const Record*r=populated?records:nullptr;std::size_t count=populated?4:0;Providers p{ControlApply|DelayPrepare|DeferredVolume,power,nullptr};Command expected[171];std::size_t total=0;pow_count=fail_pow=0;C(plan_main(r,count,99,mode,p,expected,171,total)==Status::MainPassOnly);Fake f{};events.clear();pow_count=0;
 auto result=execute_main(r,count,99,mode,p,{&f,ControlApply|DelayPrepare|DeferredVolume,effect});C(result.status==PresetStatus::CompletedMainPassEffectsOnly);C(result.completed_bindings==167&&result.attempted_effects==total&&f.commands.size()==total);for(std::size_t i=0;i<f.commands.size()&&i<total;i++){C(same(f.commands[i],expected[i]));}C(!events.empty()&&events[0]=='E');C(events.find('P')>0&&events.find('P')!=std::string::npos);
}
Fake f{};Providers p{ControlApply|DelayPrepare|DeferredVolume,power,nullptr};auto r=execute_main(nullptr,0,5,true,p,{&f,ControlApply,effect});C(r.status==PresetStatus::MissingCapability&&f.commands.empty());
p.pow=nullptr;r=execute_main(nullptr,0,5,true,p,{&f,ControlApply|DelayPrepare,effect});C(r.status==PresetStatus::MissingMath&&f.commands.empty());p.pow=power;
f.fail_effect=3;pow_count=fail_pow=0;r=execute_main(records,4,5,false,p,{&f,ControlApply|DelayPrepare|DeferredVolume,effect});C(r.status==PresetStatus::PartialBackendFailure&&r.attempted_effects==3&&f.commands.size()==3);C(!f.commands.empty()&&f.commands[0].op==Op::DeferVolume&&f.commands[0].delay==.15&&f.commands[0].generation==5);
f={};pow_count=0;fail_pow=1;r=execute_main(nullptr,0,5,true,p,{&f,ControlApply|DelayPrepare,effect});C(r.status==PresetStatus::PartialMathFailure);C(!f.commands.empty()&&r.attempted_effects==f.commands.size());fail_pow=0;
f={};Record invalid[]={{"x",NAN}};r=execute_main(invalid,1,5,true,p,{&f,ControlApply|DelayPrepare,effect});C(r.status==PresetStatus::Invalid&&f.commands.empty());

Command expected[171];std::size_t total=0;pow_count=fail_pow=0;C(plan_main(records,4,77,false,p,expected,171,total)==Status::MainPassOnly);
for(unsigned stop=1;stop<=total;stop++){
 f={};f.fail_effect=stop;pow_count=fail_pow=0;r=execute_main(records,4,77,false,p,{&f,ControlApply|DelayPrepare|DeferredVolume,effect});C(r.status==PresetStatus::PartialBackendFailure);C(r.attempted_effects==stop&&f.commands.size()==stop);std::size_t done=0;for(unsigned i=0;i<stop;i++){C(same(f.commands[i],expected[i]));if(i+1<stop&&expected[i].op!=Op::DelayEnablePrepare)++done;}C(r.completed_bindings==done);
}
Record all[170];for(unsigned i=0;i<170;i++){MainBinding binding{};C(main_binding(i,binding)==Status::State);all[i]={binding.name,double(i)/100.};}f={};Providers no_math{0,nullptr,nullptr};r=execute_main(all,170,3,true,no_math,{&f,ControlApply|DelayPrepare,effect});C(r.status==PresetStatus::CompletedMainPassEffectsOnly&&r.completed_bindings==167);
MainBinding sentinel{999,Identifier("keep"),true};C(main_binding(170,sentinel)==Status::Invalid&&sentinel.id==999&&sentinel.name=="keep"&&sentinel.derived_default);
f={};r=execute_main(records,4097,1,true,p,{&f,ControlApply|DelayPrepare,effect});C(r.status==PresetStatus::Invalid&&f.commands.empty());
r=execute_main(reinterpret_cast<const Record*>(std::uintptr_t(1)),1,1,true,p,{&f,ControlApply|DelayPrepare,effect});C(r.status==PresetStatus::Invalid&&f.commands.empty());
Record badspan[]={{Identifier(reinterpret_cast<const char*>(UINTPTR_MAX),2),0}};r=execute_main(badspan,1,1,true,p,{&f,ControlApply|DelayPrepare,effect});C(r.status==PresetStatus::Invalid&&f.commands.empty());
Record empty[]={{Identifier(nullptr,0),0}};pow_count=fail_pow=0;r=execute_main(empty,1,1,true,p,{&f,ControlApply|DelayPrepare,effect});C(r.status==PresetStatus::CompletedMainPassEffectsOnly);
std::printf("checks=%u failures=%u\n",n,bad);return bad?1:0;}
