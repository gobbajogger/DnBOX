#include "conductor.hpp"
#include <cstdio>
#include <cstdint>
#include <cstring>
using namespace original_conductor_v1;
static unsigned long long bits(double x){unsigned long long u;std::memcpy(&u,&x,8);return u;}
static double unpack(unsigned long long u){double x;std::memcpy(&x,&u,8);return x;}
static int ticks_double(double last,double acc,double beat,double tempo){
 if(!(acc>=0&&acc<0.0125))return -1;
 if(!(beat>=last))return 0;
 double elapsed=((beat-last)*60.)/(tempo<1.?1.:tempo);
 if(!(elapsed>0))return 0;
 if(elapsed>0.2)return 0;
 int n=0;acc+=elapsed;
 while(acc>=0.0125){++n;acc+=-0.0125;if(n>17)return -2;}
 return n;
}
static int ticks_port(double last,double acc,double beat,double tempo){
 Beat s{1,bits(last),bits(acc)},r{};
 Command c[32];std::size_t n=0;
 Status st=beat_catchup(s,bits(beat),bits(tempo),true,c,32,n,r);
 if(st==Status::Invalid)return -1;
 if(st!=Status::Clock)return -3;
 if(r.accumulator!=bits(unpack(r.accumulator)))return -4;
 double ref_acc=acc;
 if(!(beat>=last))ref_acc=0;
 else{
  double elapsed=((beat-last)*60.)/(tempo<1.?1.:tempo);
  if(elapsed>0){if(elapsed>0.2)ref_acc=0;else{ref_acc+=elapsed;while(ref_acc>=0.0125)ref_acc+=-0.0125;}}
 }
 if(r.accumulator!=bits(ref_acc))return -5;
 return int(n);
}
int main(){
 unsigned checks=0,tick_diff=0,wide_diff=0;
 auto expect=[&](bool ok){++checks;if(!ok){if(tick_diff+wide_diff<8)std::fprintf(stderr,"FAIL check %u\n",checks);}};
 for(int i=0;i<1000;++i){
  double acc=double(i%12)/1000.,beat=10.+double(i%103)/1000.;
  int a=ticks_double(10.,acc,beat,120.),b=ticks_port(10.,acc,beat,120.);
  if(a!=b)++tick_diff;
  expect(a==b&&a>=0);
 }
 unsigned long long rng=0x123456789abcdefULL;
 auto r64=[&](){rng=rng*6364136223846793005ULL+1;return rng;};
 for(int i=0;i<100000;++i){
  double tempo=1.+double(r64()%40000)/100.;
  double last=double(r64()%1000000)/1000.;
  double delta=double(r64()%50000)/100000.;
  double acc=double(r64()%12499)/1000000.;
  int a=ticks_double(last,acc,last+delta,tempo),b=ticks_port(last,acc,last+delta,tempo);
  if(a!=b)++wide_diff;
  expect(a==b&&a>=0);
 }
 std::printf("checks=%u probe1000_tick_diffs=%u wide100000_tick_diffs=%u\n",checks,tick_diff,wide_diff);
 return (tick_diff||wide_diff)?1:0;
}
