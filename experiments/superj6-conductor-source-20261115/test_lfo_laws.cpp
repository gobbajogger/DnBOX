#include "lfo_laws.hpp"
#include <cstdio>
#include <cmath>
using namespace original_conductor_v1;
static float pw(float a,float b)noexcept{return std::pow(a,b);}
int main(){unsigned n=0,bad=0;
#define C(x) do{++n;if(!(x)){++bad;if(bad<10)std::fprintf(stderr,"FAIL %d %s\n",__LINE__,#x);}}while(0)
 float v=-9;float phase=0;std::int64_t ix=0;Providers p{0,pw,nullptr};
 C(lfo_phase(.9f,200.f,.0125f,phase)==Status::State);C(phase==2.4f);C(lfo_index(phase,ix)==Status::State);C(ix==102);
 C(lfo_index(-.1,ix)==Status::State);C(ix==-25);
 C(lfo_rate(1,true,120,p,v)==Status::State);C(v==16);C(lfo_rate(1,false,120,p,v)==Status::State);C(v==32);
 C(lfo_rate(0,false,120,p,v)==Status::State);C(v==.0625f);
 for(unsigned i=0;i<256;i++)for(unsigned kind=1;kind<=4;kind++){
 C(shape_table_sample(kind,i,v)==Status::State);
 float expected=kind==1?(i<128?float(i)/64-1:3-float(i)/64):kind==2?(i<128?-1.f:1.f):kind==3?float(i)/128-1:1-float(i)/128;
 C(v==expected);
 for(unsigned wave=0;wave<6;wave++){float y;C(lfo_modulate(.4f,v,.75,wave,y)==Status::State);C(y>=0&&y<=1);}
 }
 v=9;C(shape_table_sample(12,0,v)==Status::Unsupported);C(v==9);
 C(lfo_modulate(0,1,1,1,v)==Status::State);C(v==1);
 C(lfo_modulate(1,-1,1,2,v)==Status::State);C(v==0);
 std::printf("checks=%u failures=%u\n",n,bad);return bad?1:0;
}
