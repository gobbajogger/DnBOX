#include "lfo_apply.hpp"
#include <cstdio>
#include <vector>
using namespace original_conductor_v1;
struct Effect{unsigned id;float value;char kind;};
struct Fake{LiveLfo live[3];std::vector<Effect> effects;bool mutate=false,fail=false;};
static bool read(void* p,unsigned i,LiveLfo& out)noexcept{out=static_cast<Fake*>(p)->live[i];return true;}
static ResolveStatus resolve(void*,std::uint16_t id,ResolvedControl& out)noexcept{if(id==207)return ResolveStatus::Missing;out={id,id,id};return ResolveStatus::Found;}
static bool set(void* p,ResolvedControl c,float value)noexcept{Fake& f=*static_cast<Fake*>(p);f.effects.push_back({c.parameter,value,'P'});if(f.mutate){f.live[1].amount=.8;f.live[1].bases[1]=.6f;}return !f.fail;}
static bool cutoff(void* p,ResolvedControl c,float value)noexcept{static_cast<Fake*>(p)->effects.push_back({c.parameter,value,'C'});return true;}
static bool dispatch(void* p,std::uint16_t id,float value)noexcept{static_cast<Fake*>(p)->effects.push_back({id,value,'G'});return true;}
int main(){unsigned n=0,bad=0;
#define C(x) do{++n;if(!(x)){++bad;std::fprintf(stderr,"FAIL %d %s\n",__LINE__,#x);}}while(0)
 Fake f{};for(LiveLfo& l:f.live)l={true,false,true,0,0,1,.2,{.1f,.2f},{126,126}};ApplyState state{true,{false,false,false},{0,0,0},1};ApplyBackend b{&f,read,resolve,set,cutoff,dispatch};float row[6]={1.f,1.f,-1.f,1.f,.5f,0.f};
 auto r=apply_tick_lfo(0,row,state,b);C(r.status==ApplyStatus::AppliedCommandsOnly);C(r.effects==2&&f.effects.size()==2);
 f.effects.clear();f.mutate=true;r=apply_tick_lfo(1,row,state,b);C(r.status==ApplyStatus::AppliedCommandsOnly);C(r.effects==4&&f.effects.size()==4);if(f.effects.size()==4){C(f.effects[0].value==.1f+.1f);C(f.effects[2].value==.6f+.1f);}
 f.effects.clear();r=apply_tick_lfo(1,row,state,b);C(r.status==ApplyStatus::AppliedCommandsOnly);if(f.effects.size()==4)C(f.effects[0].value==.1f+.4f);
 f.mutate=false;f.effects.clear();f.live[2].mode=3;f.live[2].waveform=1;state.current_time[2]=1;r=apply_tick_lfo(2,row,state,b);C(r.status==ApplyStatus::AppliedCommandsOnly&&r.cycle_changed&&state.cycle_complete[2]);if(f.effects.size()==4)C(f.effects[0].value==.1f+.2f);
 f.effects.clear();C(apply_tick_lfo(2,row,state,b).status==ApplyStatus::Noop);C(f.effects.empty());
 f.live[0].amount=.01;C(apply_tick_lfo(0,row,state,b).status==ApplyStatus::Noop);
 f.live[1].snapback=true;f.live[1].mode=0;state.held_notes=0;C(apply_tick_lfo(1,row,state,b).status==ApplyStatus::AppliedCommandsOnly);f.live[1].mode=1;C(apply_tick_lfo(1,row,state,b).status==ApplyStatus::Noop);
 std::printf("checks=%u failures=%u\n",n,bad);return bad?1:0;
}
