#include "conductor.hpp"
#include <cmath>
#include <cstdio>
#include <cstring>
using namespace original_conductor_v1;
static float pw(float a,float b)noexcept{return std::pow(a,b);}
static float ex(float x)noexcept{return std::exp(x);}
static unsigned fb(float f){unsigned u;std::memcpy(&u,&f,4);return u;}
int main(){Providers p{31,pw,ex};Command c[256];std::size_t n;
 for(unsigned i=0;i<218;i++){float v;if(query_default(i,p,v)!=Status::State)return 4;std::printf("D %u %08x\n",i,fb(v));}
 for(int flag=0;flag<2;flag++){if(plan_main(nullptr,0,17,flag,p,c,256,n)!=Status::MainPassOnly)return 1;for(std::size_t j=0;j<n;j++)std::printf("P %d %zu %u %u %08x %08x\n",flag,j,c[j].id,unsigned(c[j].op),fb(c[j].value),fb(c[j].delay));}
 for(int i=0;i<1000;i++){Cutoff s{2,float(i%101)/100.f,float((i*13)%101)/100.f,true},r{};float dt=float(i%37)/48000.f;if(cutoff_step(s,dt,p,c,256,n,r)!=Status::PendingAck)return 2;std::printf("C %d %08x %08x %08x %llu\n",i,fb(c[0].value),fb(r.current),fb(r.target),(unsigned long long)c[0].token);}
 for(int i=0;i<1000;i++){Beat s{2,10.f,float(i%12)/1000.f},r{};float beat=10.f+float(i%103)/1000.f;if(beat_catchup(s,beat,120.f,true,c,256,n,r)!=Status::Clock)return 3;std::printf("B %d %zu %08x\n",i,n,fb(r.accumulator));}
}
