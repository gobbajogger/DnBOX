#include "preset_executor.hpp"
#include <cstdio>
#include <vector>
#include <cstring>
#include <cmath>
using namespace original_conductor_v1;
struct Context{std::vector<Command> commands;PwmPostpassSnapshot* external;unsigned fail;};
static bool effect(void*p,const Command&c)noexcept{auto&x=*static_cast<Context*>(p);x.commands.push_back(c);if(x.external)for(auto&r:x.external->route)r=0;return x.commands.size()!=x.fail;}
static double power(double a,double b)noexcept{return std::pow(a,b);}
static unsigned long long bits(double x){unsigned long long b;std::memcpy(&b,&x,8);return b;}
int main(){unsigned checks=0,bad=0;
#define C(x) do{++checks;if(!(x)){++bad;std::fprintf(stderr,"FAIL %d\n",__LINE__);}}while(0)
const unsigned ids[4]={184,186,188,190};Record records[]={{"neonlfo2Dest1BaseValue",-.0},{"neonlfo2Dest2BaseValue",.22},{"neonlfo3Dest1BaseValue",.33},{"neonlfo3Dest2BaseValue",1.32},{"neonlfo2Dest1BaseValue",.99},{"neonpulseWidthLayer1",.8}};Providers p{0,power,nullptr};
for(unsigned mask=0;mask<16;mask++){PwmPostpassSnapshot snap{};unsigned n=0;for(unsigned i=0;i<4;i++){snap.route[i]=(mask&(1U<<i))?(i&1?79:78):77;if(mask&(1U<<i))n++;}Context x{{},&snap,0};auto r=execute_pwm_postpass(records,6,99,snap,p,{&x,ControlApply,effect});C(r.status==PresetStatus::CompletedPwmPostpassEffectsOnly);C(r.attempted_effects==n&&r.completed_bindings==n&&x.commands.size()==n);unsigned j=0;for(unsigned i=0;i<4;i++)if(mask&(1U<<i)){if(j<x.commands.size()){const auto&c=x.commands[j];C(c.id==ids[i]&&c.order==j&&c.generation==99&&c.op==Op::ApplyControl&&c.receiver==Receiver::InstrumentControl&&c.domain==Domain::NormalizedControl&&c.delay==0&&c.token==0);C(bits(c.value)==bits(double(float(records[i].value))));}j++;}}
for(unsigned route=0;route<256;route++){Context x{};PwmPostpassSnapshot snap{{std::uint8_t(route),0,0,0}};auto r=execute_pwm_postpass(nullptr,0,7,snap,p,{&x,ControlApply,effect});C(r.status==PresetStatus::CompletedPwmPostpassEffectsOnly);C(x.commands.size()==unsigned(route==78||route==79));if(!x.commands.empty()){float v=-1;C(query_default(184,p,v)==Status::State);C(bits(x.commands[0].value)==bits(double(v)));}}
PwmPostpassSnapshot all{{78,79,78,79}};for(unsigned fail=1;fail<=4;fail++){Context x{{},nullptr,fail};auto r=execute_pwm_postpass(records,6,8,all,p,{&x,ControlApply,effect});C(r.status==PresetStatus::PartialBackendFailure&&r.attempted_effects==fail&&r.completed_bindings==fail-1&&r.failed_id==ids[fail-1]);C(x.commands.size()==fail);}
Context x{};auto r=execute_pwm_postpass(records,6,8,all,p,{&x,0,effect});C(r.status==PresetStatus::MissingCapability&&x.commands.empty());r=execute_pwm_postpass(records,6,8,{{0,1,77,80}},p,{nullptr,0,nullptr});C(r.status==PresetStatus::CompletedPwmPostpassEffectsOnly&&r.attempted_effects==0);Record badrecord[]={{"x",NAN}};r=execute_pwm_postpass(badrecord,1,8,all,p,{&x,ControlApply,effect});C(r.status==PresetStatus::Invalid&&x.commands.empty());r=execute_pwm_postpass(records,4097,8,all,p,{&x,ControlApply,effect});C(r.status==PresetStatus::Invalid&&x.commands.empty());
std::printf("checks=%u failures=%u\n",checks,bad);return bad?1:0;}
