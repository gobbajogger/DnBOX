#include <initializer_list>
#include "conductor.hpp"
#include "cutoff_executor.hpp"
#include <cstring>
#include <cstdio>
using namespace original_conductor_v1;
static unsigned bits(float v){unsigned u;std::memcpy(&u,&v,4);return u;}
struct F{float a,n;};
static bool apply(void*p,float v)noexcept{static_cast<F*>(p)->a=v;return true;}
static bool post(void*p,unsigned,float v)noexcept{static_cast<F*>(p)->n=v;return true;}
static bool read(void*p,float&v)noexcept{v=static_cast<F*>(p)->a;return true;}
int main(){unsigned bad=0,n=0;
#define C(x) do{++n;if(!(x)){++bad;std::fprintf(stderr,"FAIL %d %s\n",__LINE__,#x);}}while(0)
for(unsigned sign=0;sign<2;++sign){unsigned u=sign<<31;float z;std::memcpy(&z,&u,4);
for(double dt:{0.,-1.}){Command cmd[2];std::size_t count=0;Cutoff next,ack;Cutoff s(1,z,z,true);Providers p{CutoffApply|GlobalDispatch,nullptr,nullptr};
C(cutoff_step(s,dt,p,cmd,2,count,next)==Status::PendingAck);C(count==2);C(bits(float(cmd[0].value))==u);C(bits(float(cmd[1].value))==u);C(bits(next.current)==u);C(bits(next.target)==u);
C(cutoff_ack(next,1,next.token,z,ack)==Status::State);C(!ack.active&&!ack.awaiting);C(bits(ack.current)==u&&bits(ack.target)==u);
RuntimeCutoff rs{z,z,true};F f{1,1};CutoffBackend backend{&f,apply,post,read};auto r=execute_cutoff(rs,dt,p,backend);C(r.status==CutoffStatus::CompletedEffectsOnly);C(bits(f.a)==u&&bits(f.n)==u);C(bits(rs.current)==u&&bits(rs.target)==u&&!rs.active);
}}
std::printf("checks=%u failures=%u\n",n,bad);return bad?1:0;}
