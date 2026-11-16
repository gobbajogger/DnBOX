#include "tick_prefix.hpp"
#include <cmath>
#include <cstdio>
using namespace original_conductor_v1;
static unsigned sine_calls,entropy_calls;static bool order_ok;
static float sine(float x)noexcept{++sine_calls;return std::sin(x);}
static bool entropy(void*,std::uint64_t& v)noexcept{if(sine_calls!=256)order_ok=false;++entropy_calls;v=UINT64_MAX;return true;}
int main(){int checks=0,bad=0;
#define C(x) do{++checks;if(!(x)){++bad;std::fprintf(stderr,"FAIL %d %s\n",__LINE__,#x);}}while(0)
 TickPrefixState state{{0,0,0},{.1f,.2f,.3f,.4f},{0,0,0},0,false,false};TickPrefixInput in{{1,1,1},.5f,{0,0,0},.0125f};TickPrefixOutput out{};
 sine_calls=entropy_calls=0;order_ok=true;auto status=prepare_tick_prefix(state,in,{sine},{nullptr,entropy},100,out);C(status==PrefixStatus::PreparedPrefixOnly);
 if(status==PrefixStatus::PreparedPrefixOnly){C(sine_calls==256&&entropy_calls==4&&out.entropy_requests==4&&order_ok);C(out.state.phase[0]==0&&out.state.phase[1]==.0125f&&out.state.phase[2]==0);C(out.rows[0][0]==0&&out.rows[0][1]==-1&&out.rows[0][2]==1&&out.rows[0][3]==1&&out.rows[0][4]==.1f);C(out.rows[3][4]==.4f);}
 in={{4,4,4},.5f,{4,4,4},.3f};state.delay1_finished=state.delay3_finished=true;sine_calls=entropy_calls=0;order_ok=true;status=prepare_tick_prefix(state,in,{sine},{nullptr,entropy},100,out);C(status==PrefixStatus::PreparedPrefixOnly);
 if(status==PrefixStatus::PreparedPrefixOnly){C(entropy_calls==7&&out.entropy_requests==7&&order_ok);for(unsigned i=0;i<3;i++){C(out.state.held[i]==1);C(out.state.held_elapsed[i]==0);}}
 sine_calls=entropy_calls=0;C(prepare_tick_prefix(state,in,{nullptr},{nullptr,entropy},100,out)==PrefixStatus::MissingCapability);C(!sine_calls&&!entropy_calls);
 std::printf("checks=%d failures=%d\n",checks,bad);return bad?1:0;
}
