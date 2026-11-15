#include "conductor.hpp"
#include <cmath>
#include <cstdio>
#include <cstring>
using namespace original_conductor_v1;
static double pw(double a,double b)noexcept{return std::pow(a,b);}
static double ex(double x)noexcept{return std::exp(x);}
static unsigned fb(float f){unsigned u;std::memcpy(&u,&f,4);return u;}
static unsigned long long db(double d){unsigned long long u;std::memcpy(&u,&d,8);return u;}
int main(){Providers p{31,pw,ex};Command c[256];std::size_t n;
 for(unsigned i=0;i<218;i++){float v;if(query_default(i,p,v)!=Status::State)return 4;std::printf("D %u %08x\n",i,fb(v));}
 for(int flag=0;flag<2;flag++){if(plan_main(nullptr,0,17,flag,p,c,256,n)!=Status::MainPassOnly)return 1;for(std::size_t j=0;j<n;j++)std::printf("P %d %zu %u %u %016llx %016llx\n",flag,j,c[j].id,unsigned(c[j].op),db(c[j].value),db(c[j].delay));}
 for(int i=0;i<1000;i++){Cutoff s{2,float((i%101)/100.),float(((i*13)%101)/100.),true},r{};double dt=double(i%37)/48000.;if(cutoff_step(s,dt,p,c,256,n,r)!=Status::PendingAck)return 2;std::printf("C %d %08x %08x %08x %llu\n",i,fb(float(c[0].value)),fb(r.current),fb(r.target),(unsigned long long)c[0].token);}
 for(int i=0;i<1000;i++){Beat s{2,10.,double(i%12)/1000.},r{};double beat=10.+double(i%103)/1000.;if(beat_catchup(s,beat,120.,true,c,256,n,r)!=Status::Clock)return 3;std::printf("B %d %zu %016llx\n",i,n,db(r.accumulator));}
}
