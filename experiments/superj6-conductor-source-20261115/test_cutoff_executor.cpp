#include "cutoff_executor.hpp"
#include <cstdio>
#include <cmath>
#include <string>
using namespace original_conductor_v1;
struct Fake{RuntimeCutoff* state;float core;std::string trace;float posted;bool mutate_current,mutate_target,fail;};
static float ex(float x)noexcept{return std::exp(x);}
static bool apply(void* p,float v)noexcept{Fake& f=*static_cast<Fake*>(p);f.trace+='A';f.core=v;return !f.fail;}
static bool post(void* p,unsigned,float v)noexcept{Fake& f=*static_cast<Fake*>(p);f.trace+='N';f.posted=v;if(f.mutate_current)f.state->current=.2f;if(f.mutate_target)f.state->target=.9f;return true;}
static bool read(void* p,float& v)noexcept{Fake& f=*static_cast<Fake*>(p);f.trace+='R';v=f.core;return true;}
int main(){unsigned n=0,bad=0;
#define C(x) do{++n;if(!(x)){++bad;std::fprintf(stderr,"FAIL %d %s\n",__LINE__,#x);}}while(0)
 RuntimeCutoff s{.4999f,.5f,true};Fake f{&s,0,"",0,false,false,false};CutoffBackend b{&f,apply,post,read};Providers p{0,nullptr,ex};
 auto r=execute_cutoff(s,.001,p,b);C(r.status==CutoffStatus::CompletedEffectsOnly);C(f.trace=="ANR");C(f.posted!=.5f&&s.current==.5f&&!s.active);
 s={.4999f,.5f,true};f.trace.clear();f.mutate_current=true;r=execute_cutoff(s,.001,p,b);C(r.status==CutoffStatus::CompletedEffectsOnly);C(f.trace=="AN"&&s.current==.2f&&s.active);
 s={.4999f,.5f,true};f.trace.clear();f.mutate_current=false;f.mutate_target=true;r=execute_cutoff(s,.001,p,b);C(r.status==CutoffStatus::CompletedEffectsOnly);C(f.trace=="ANR"&&s.current==.5f&&s.target==.9f&&!s.active);
 s={.4999f,.5f,true};f.trace.clear();f.mutate_target=false;f.fail=true;r=execute_cutoff(s,.001,p,b);C(r.status==CutoffStatus::PartialFailure&&r.effects==1);C(f.trace=="A"&&s.current==f.core&&s.active);
 std::printf("checks=%u failures=%u\n",n,bad);return bad?1:0;
}
