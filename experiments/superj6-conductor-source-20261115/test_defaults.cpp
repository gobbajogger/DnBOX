#include "conductor.hpp"
#include "preset_executor.hpp"
#include "control_data.hpp"
#include "default_reference_testonly.hpp"
#include <cmath>
#include <cstdio>
#include <cstring>
#include <limits>
using namespace original_conductor_v1;
static unsigned mode,calls;static float aa,bb;
static float value(float a,float b){switch(mode){case 0:return std::pow(a,b);case 1:return 0.f;case 2:return 1.f;case 3:return -0.f;case 4:return -1.f;case 5:return 2.f;case 6:return NAN;default:return INFINITY;}}
static float pw(float a,float b)noexcept{++calls;aa=a;bb=b;return value(a,b);}
static float narrowed(std::uint64_t x){double y;std::memcpy(&y,&x,8);return static_cast<float>(y);}static float f(std::uint32_t x){float y;std::memcpy(&y,&x,4);return y;}static std::uint32_t fb(float x){std::uint32_t y;std::memcpy(&y,&x,4);return y;}
int main(){unsigned checks=0,bad=0;
#define C(x) do{++checks;if(!(x)){++bad;std::fprintf(stderr,"FAIL %d\n",__LINE__);}}while(0)
C(sizeof(default_literal_bits)+sizeof(default_expression_index)+sizeof(default_expressions)==1554);
for(unsigned i=0;i<218;i++){const auto&r=frozen_r9_testonly::default_data[i];unsigned ix=default_expression_index[i];C(bool(ix)==r.derived);C(default_literal_bits[i]==r.bits);if(ix){const auto&e=default_expressions[ix-1];C(e.input_bits==fb(narrowed(r.input_bits))&&e.lo_bits==fb(narrowed(r.lo_bits))&&e.hi_bits==fb(narrowed(r.hi_bits))&&e.taper_bits==fb(narrowed(r.taper_bits)));}}
for(mode=0;mode<9;mode++)for(unsigned i=0;i<220;i++){float actual=-123.25f,expected=actual;calls=0;Status want=Status::Invalid;unsigned need=0;float a=0.f,b=0.f;if(i<218){const auto&r=frozen_r9_testonly::default_data[i];if(!r.derived){want=Status::State;expected=f(r.bits);}else if(mode==8)want=Status::MissingPow;else{need=1;a=(narrowed(r.input_bits)-narrowed(r.lo_bits))/(narrowed(r.hi_bits)-narrowed(r.lo_bits));b=1.f/narrowed(r.taper_bits);float x=value(a,b);if(!std::isfinite(x)||x<0.f||x>1.f)want=Status::MathFailure;else{want=Status::State;expected=x;}}}auto got=query_default(i,{0,mode==8?nullptr:pw,nullptr},actual);C(got==want);C(fb(actual)==fb(expected));C(calls==need);if(need){C(fb(aa)==fb(a));C(fb(bb)==fb(b));}}
for(unsigned i=0;i<170;i++){MainBinding b{};C(main_binding(i,b)==Status::State);const auto&r=frozen_r9_testonly::order_data[i];C(b.id==r.id&&b.name==r.name&&b.derived_default==frozen_r9_testonly::default_data[r.id].derived);}
std::printf("checks=%u failures=%u\n",checks,bad);return bad?1:0;}
