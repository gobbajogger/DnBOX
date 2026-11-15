#include "ordered_tick.hpp"
#include <cstdio>
#include <cmath>
#include <string>
using namespace original_conductor_v1;
struct Fake{OrderedState* s;LiveLfo l[3];LiveVibrato v;std::string trace;float later_value;unsigned entropy;bool fail;bool expect_cycle,observed_cycle;};
static float sine(float x)noexcept{return std::sin(x);}
static double pow_(double x,double y)noexcept{return std::pow(x,y);}
static double exp_(double x)noexcept{return std::exp(x);}
static bool random_(void*p,std::uint64_t&v)noexcept{++static_cast<Fake*>(p)->entropy;v=UINT64_MAX;return true;}
static bool live(void*p,unsigned i,LiveLfo&l)noexcept{Fake&f=*static_cast<Fake*>(p);l=f.l[i];l.delay_finished=f.s->times.delay_finished[i];return true;}
static ResolveStatus resolve(void*,std::uint16_t id,ResolvedControl&r)noexcept{r={1,id,id};return ResolveStatus::Found;}
static bool set(void*p,ResolvedControl r,float v)noexcept{Fake&f=*static_cast<Fake*>(p);f.trace+=char('1'+r.parameter-10);if(r.parameter==10){f.l[1].amount=1.;f.v.rate_target=1.;}if(r.parameter==11)f.later_value=v;return !f.fail;}
static bool cutoff(void*p,ResolvedControl,float v)noexcept{Fake&f=*static_cast<Fake*>(p);f.s->cutoff.target=v;f.s->cutoff.active=true;return true;}
static bool notify(void*,std::uint16_t,float)noexcept{return true;}
static bool vlive(void*p,LiveVibrato&v)noexcept{v=static_cast<Fake*>(p)->v;return true;}
static bool freq(void*p,float,double)noexcept{Fake&f=*static_cast<Fake*>(p);f.trace+='F';if(f.expect_cycle)f.observed_cycle=f.s->vibrato.cycle_complete;return true;}
static bool depth(void*p,float,double)noexcept{static_cast<Fake*>(p)->trace+='D';return true;}
static bool ca(void*p,float)noexcept{static_cast<Fake*>(p)->trace+='C';return true;}
static bool cn(void*p,unsigned,float)noexcept{static_cast<Fake*>(p)->trace+='N';return true;}
static bool cr(void*p,float&v)noexcept{v=static_cast<Fake*>(p)->s->cutoff.current;return true;}
int main(){unsigned checks=0,bad=0;
#define C(x) do{++checks;if(!(x)){++bad;std::fprintf(stderr,"FAIL %d %s\n",__LINE__,#x);}}while(0)
OrderedState s{};s.apply.armed=true;s.apply.held_notes=1;s.times.delay_finished[0]=s.times.delay_finished[2]=true;s.times.current[3]=.4;s.cutoff={0,.2f,true};
Fake f{};f.s=&s;f.v={1.,120.,.5f,.5f,false};for(unsigned i=0;i<3;i++)f.l[i]={true,false,true,0,3,1.f,.2f,{0.f,0.f},{std::uint16_t(10+i),std::uint16_t(10+i)}};
OrderedInput in{{{1,1,1},.5f,{3,3,3},.01},1.,1.,true,true};OrderedBackend b{{sine},{&f,random_},100,{nullptr,nullptr},{0,pow_,exp_},{&f,live,resolve,set,cutoff,notify},{&f,ca,cn,cr},{&f,vlive,freq,depth}};
auto r=execute_ordered_tick(s,in,b);C(r.status==OrderedStatus::CompletedEffectsOnly);C(f.trace=="12233CNFD");C(f.later_value==.5f);C(!s.vibrato.cycle_complete);C(f.entropy==4);
// LFO1 changed live V4 rate to1 but pre-effect factor was2: .41 is below .5.
// Recomputing after callbacks would incorrectly use factor1 (also not complete),
// so choose pre-effect target0 (factor3) for the next explicit boundary.
s={};s.apply.armed=true;s.apply.held_notes=1;s.times.delay_finished[0]=s.times.delay_finished[2]=true;s.times.current[3]=.4;f.trace.clear();f.entropy=0;in.prefix.vibrato_rate_target=0;f.expect_cycle=true;
r=execute_ordered_tick(s,in,b);C(r.status==OrderedStatus::CompletedEffectsOnly);C(s.vibrato.cycle_complete);C(f.observed_cycle);C(f.trace=="12233FFD");
s={};f.trace.clear();f.entropy=0;in.fully_loaded=false;r=execute_ordered_tick(s,in,b);C(r.status==OrderedStatus::NotLoaded);C(f.entropy==0&&f.trace.empty());
in.fully_loaded=true;s.apply.armed=true;s.apply.held_notes=1;s.times.delay_finished[0]=s.times.delay_finished[2]=true;f.fail=true;r=execute_ordered_tick(s,in,b);C(r.status==OrderedStatus::PartialFailure&&r.stage==OrderedStage::Lfo1);C(f.trace=="1");C(s.phases[0]>0&&f.entropy==4);
std::printf("checks=%u failures=%u\n",checks,bad);return bad?1:0;}
