#include "tick_prefix.hpp"
#include <cstdio>
#include <cmath>
#include <cstring>
#include <limits>
using namespace original_conductor_v1;
namespace original_conductor_v1 {PrefixStatus prepare_tick_prefix_reference(const TickPrefixState&,const TickPrefixInput&,Sine,EntropyProvider,std::uint64_t,TickPrefixOutput&) noexcept;}
static unsigned sine_count,entropy_count,fail_sine,fail_entropy;static std::uint64_t seed,trace;
static std::uint64_t word(double x){std::uint64_t u;std::memcpy(&u,&x,8);return u;}
static unsigned fword(float x){unsigned u;std::memcpy(&u,&x,4);return u;}
static float sine(float x)noexcept{++sine_count;trace=trace*UINT64_C(6364136223846793005)+fword(x);if(sine_count==fail_sine)return std::numeric_limits<float>::quiet_NaN();return std::sin(x);}
static bool random(void*,std::uint64_t&x)noexcept{++entropy_count;trace=trace*UINT64_C(6364136223846793005)+UINT64_C(0xe0000000)+sine_count;seed=seed*UINT64_C(2862933555777941757)+3037000493U;x=seed;return entropy_count!=fail_entropy;}
static void emit(double x){std::printf(" %016llx",static_cast<unsigned long long>(word(x)));}
int main(){unsigned completed=0,failed=0;for(unsigned c=0;c<6144;c++){
 TickPrefixState s{};for(unsigned i=0;i<3;i++){s.phase[i]=double((c*(i+1))%257)/256.;s.held_elapsed[i]=double((c+i)%11)/10.;}for(unsigned i=0;i<4;i++)s.held[i]=double((c+i)%17)/8.-1.;s.time4=double(c%129)/128.;s.delay1_finished=(c&1)!=0;s.delay3_finished=(c&2)!=0;
 TickPrefixInput in{};const float rates[6]={0,.25f,.5f,1,4,1000};for(unsigned i=0;i<3;i++){in.rates[i]=rates[(c+i)%6];in.waveforms[i]=(c+i)%6;}in.vibrato_rate_target=float(c%33)/32;in.dt=(c%3==0)?0.:(c%3==1)?.0125:1.;
 TickPrefixOutput out{};out.state.phase[0]=-.125;out.state.held[3]=-.5;out.rows[0][0]=-.25;out.entropy_requests=999;out.captured_factor4=-2;
 sine_count=entropy_count=0;seed=c+1;trace=0;fail_sine=c%19==0?((c%256)+1):0;fail_entropy=c%17==0?((c%7)+1):0;std::uint64_t budget=c%13==0?c%8:1000;
 if(c%137==0){in.waveforms[0]=6;}
 if(c%139==0){in.rates[1]=-1;}
 TickPrefixOutput expected=out;
 PrefixStatus st=prepare_tick_prefix(s,in,{sine},{nullptr,random},budget,out);
 unsigned saved_sine=sine_count,saved_entropy=entropy_count;std::uint64_t saved_trace=trace;
 seed=c+1;trace=0;sine_count=entropy_count=0;
 PrefixStatus ref=prepare_tick_prefix_reference(s,in,{sine},{nullptr,random},budget,expected);
 bool equal=st==ref&&saved_sine==sine_count&&saved_entropy==entropy_count&&saved_trace==trace&&out.entropy_requests==expected.entropy_requests&&fword(out.captured_factor4)==fword(expected.captured_factor4)&&out.state.delay1_finished==expected.state.delay1_finished&&out.state.delay3_finished==expected.state.delay3_finished&&word(out.state.time4)==word(expected.state.time4);
 for(unsigned i=0;i<3;i++){equal=equal&&word(out.state.phase[i])==word(expected.state.phase[i])&&word(out.state.held_elapsed[i])==word(expected.state.held_elapsed[i]);}
 for(unsigned i=0;i<4;i++){equal=equal&&word(out.state.held[i])==word(expected.state.held[i]);for(unsigned j=0;j<6;j++){equal=equal&&word(out.rows[i][j])==word(expected.rows[i][j]);}}
 if(!equal){std::fprintf(stderr,"reference mismatch case=%u\n",c);return 1;}
 if(st==PrefixStatus::PreparedPrefixOnly)++completed;else ++failed;
 std::printf("%u %u %u %u %016llx %llu %08x",c,unsigned(st),sine_count,entropy_count,static_cast<unsigned long long>(trace),static_cast<unsigned long long>(out.entropy_requests),fword(out.captured_factor4));
 for(double x:out.state.phase){emit(x);}
 for(double x:out.state.held){emit(x);}
 for(double x:out.state.held_elapsed){emit(x);}
 emit(out.state.time4);std::printf(" %u %u",out.state.delay1_finished,out.state.delay3_finished);for(auto&row:out.rows)for(double x:row)emit(x);std::puts("");
 }std::fprintf(stderr,"cases=6144 completed=%u rejected=%u\n",completed,failed);return completed&&failed?0:1;}
