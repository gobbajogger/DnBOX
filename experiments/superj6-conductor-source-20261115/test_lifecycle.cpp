#include "lifecycle.hpp"
#include <cstdio>
#include <cmath>
using namespace original_conductor_v1;
struct F{Lifecycle*s;unsigned entropy,restarts;bool observed,fail;};
static float sine(float x)noexcept{return std::sin(x);}
static bool random(void*p,std::uint64_t&v)noexcept{++static_cast<F*>(p)->entropy;v=UINT64_MAX;return true;}
static bool restart(void*p)noexcept{F&f=*static_cast<F*>(p);++f.restarts;f.observed=f.s->tick.apply.armed&&f.s->tick.phases[0]==0&&f.s->tick.held[3]==.7&&!f.s->new_note;return !f.fail;}
static bool live(void*,unsigned,LiveLfo&l)noexcept{l={false,false,false,0,0,1,0,{0,0},{0,0}};return true;}
int main(){unsigned n=0,bad=0;
#define C(x) do{++n;if(!(x)){++bad;std::fprintf(stderr,"FAIL %d %s\n",__LINE__,#x);}}while(0)
Lifecycle s{};F f{&s,0,0,false,false};OrderedInput in{{{0,0,0},.5,{0,0,0},.9},1,1,true,true};OrderedBackend b{{sine},{&f,random},100,{&f,restart},{0,nullptr,nullptr},{nullptr,live,nullptr,nullptr,nullptr,nullptr},{nullptr,nullptr,nullptr,nullptr},{nullptr,nullptr,nullptr,nullptr}};
s.new_note=true;s.tick.phases[0]=.3;s.tick.held[0]=.2;s.tick.held[3]=.7;s.tick.times.delay_elapsed[1]=.8;s.last_beat=5;s.beat_accumulator=.01;
auto r=full_reset(s,{true,6},in,b);C(r.status==LifecycleStatus::CompletedEffectsOnly);C(f.restarts==1&&f.observed&&f.entropy==4);C(s.last_beat==6&&s.beat_accumulator==0);C(s.tick.held[0]==0&&s.tick.held[3]==.7&&s.tick.times.delay_elapsed[1]==.8);
for(unsigned mask=0;mask<16;mask++)for(unsigned fresh=0;fresh<2;fresh++){
 s={};s.new_note=fresh;for(unsigned i=0;i<4;i++)s.retrigger[i]=(mask>>i)&1;s.last_beat=7;s.beat_accumulator=.01;s.tick.held[0]=.4;s.tick.held_elapsed[0]=.5;for(unsigned i=0;i<3;i++){s.tick.phases[i]=.2;s.tick.times.current[i]=.3;s.tick.apply.cycle_complete[i]=true;}s.tick.times.current[3]=.6;s.tick.vibrato.cycle_complete=true;f.entropy=0;f.restarts=0;
 r=selective_retrigger(s,{false,0},in,b);bool run=fresh&&(mask&7);C(r.status==(run?LifecycleStatus::CompletedEffectsOnly:LifecycleStatus::Noop));C(f.entropy==(run?4U:0U)&&f.restarts==0);C(s.last_beat==7);C(s.beat_accumulator==(run?0:.01));C(s.tick.held[0]==.4&&s.tick.held_elapsed[0]==.5);for(unsigned i=0;i<3;i++){bool clear=run&&s.retrigger[i];C(s.tick.phases[i]==(clear?0:.2));C(s.tick.times.current[i]==(clear?0:.3));C(s.tick.apply.cycle_complete[i]==!clear);}C(s.tick.times.current[3]==((run&&s.retrigger[3])?0:.6));C(s.tick.vibrato.cycle_complete==!(run&&s.retrigger[3]));
}
s={};s.tick.held[3]=.7;f.fail=true;f.entropy=0;r=full_reset(s,{true,0},in,b);C(r.status==LifecycleStatus::PartialFailure&&f.entropy==0&&s.tick.apply.armed);
std::printf("checks=%u failures=%u\n",n,bad);return bad?1:0;}
