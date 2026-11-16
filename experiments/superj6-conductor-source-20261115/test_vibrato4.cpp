#include "vibrato4.hpp"
#include <cmath>
#include <cstdio>
#include <vector>
using namespace original_conductor_v1;
struct E{char kind;float normalized;float mapped;};struct Fake{LiveVibrato live;std::vector<E> effects;bool mutate;};
static float pw(float x,float y)noexcept{return std::pow(x,y);}
static bool read(void* p,LiveVibrato& v)noexcept{v=static_cast<Fake*>(p)->live;return true;}
static bool freq(void* p,float n,float v)noexcept{Fake& f=*static_cast<Fake*>(p);f.effects.push_back({'F',n,v});if(f.mutate)f.live.rate_target=.8f;return true;}
static bool depth(void* p,float n,float v)noexcept{static_cast<Fake*>(p)->effects.push_back({'D',n,v});return true;}
int main(){unsigned n=0,bad=0;
#define C(x) do{++n;if(!(x)){++bad;std::fprintf(stderr,"FAIL %d %s\n",__LINE__,#x);}}while(0)
 Fake f{{.1f,120.f,.6f,.5f,false},{},false};VibratoBackend b{&f,read,freq,depth};Providers p{0,pw,nullptr};Vibrato4State s{true,true,false,.1f};float row[6]={0.f,0.f,0.f,0.f,0.f,0.f};
 auto r=apply_vibrato4(2.f,row,s,p,b);C(r.status==VibratoStatus::AppliedCommandsOnly);C(r.effects==2&&!s.cycle_complete);if(f.effects.size()==2){C(f.effects[0].normalized==.3f);C(f.effects[0].mapped==16.f*std::pow(.3f,3.33f));C(f.effects[1].normalized==.25f);C(f.effects[1].mapped==12.f*std::pow(.25f,3.03f));}
 f.effects.clear();f.mutate=true;s.current_time=.5;r=apply_vibrato4(2,row,s,p,b);C(r.status==VibratoStatus::AppliedCommandsOnly&&r.cycle_changed&&s.cycle_complete);C(r.effects==3&&f.effects.size()==3);if(f.effects.size()==3){C(f.effects[0].normalized==.6f);C(f.effects[1].normalized==.8f);C(f.effects[2].normalized==.5f);}
 f.effects.clear();C(apply_vibrato4(2,row,s,p,b).status==VibratoStatus::Noop);C(f.effects.empty());
 s.cycle_complete=false;f.live.delay1=.019;r=apply_vibrato4(2,row,s,p,b);C(r.status==VibratoStatus::Noop&&r.cycle_changed&&s.cycle_complete);C(f.effects.empty());
 std::printf("checks=%u failures=%u\n",n,bad);return bad?1:0;
}
