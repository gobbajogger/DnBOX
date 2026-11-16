#include "tick_time.hpp"
#include <cstdio>
using namespace original_conductor_v1;
struct Probe{TickTimes* state;unsigned calls;bool succeed,observed;};
static bool restart(void* p)noexcept{Probe& x=*static_cast<Probe*>(p);++x.calls;x.observed=x.state->delay_finished[0]&&x.state->current[0]==0&&x.state->current[3]==0;return x.succeed;}
int main(){unsigned checks=0,bad=0;
#define C(x) do{++checks;if(!(x)){++bad;std::fprintf(stderr,"FAIL %d %s\n",__LINE__,#x);}}while(0)
 TickTimes t{{0,0,0,0},{0,.7f,0},{false,true,false}};Probe p{&t,0,true,false};
 C(advance_tick_times(t,.2f,.2f,.2f,{&p,restart})==TimeStatus::Updated);C(!t.delay_finished[0]&&!t.delay_finished[2]&&p.calls==0);C(t.current[1]==.2f&&t.current[0]==0&&t.current[3]==0&&t.delay_elapsed[1]==.7f);
 C(advance_tick_times(t,.01f,.2f,.2f,{&p,restart})==TimeStatus::Updated);C(p.calls==1&&p.observed);C(t.current[0]==.01f&&t.current[3]==.01f&&t.current[2]==.01f&&t.delay_finished[0]&&t.delay_finished[2]);
 t={{0,0,0,0},{.2f,.7f,.2f},{false,true,false}};p={&t,0,false,false};C(advance_tick_times(t,.01f,.2f,.2f,{&p,restart})==TimeStatus::RestartFailedAfterStateChange);C(p.calls==1&&p.observed);C(t.delay_finished[0]&&t.delay_elapsed[0]==.2f+.01f&&t.current[1]==0&&t.delay_elapsed[2]==.2f);
 t={{0,0,0,0},{.2f,.7f,.2f},{false,true,false}};C(advance_tick_times(t,.01f,.2f,.2f,{nullptr,nullptr})==TimeStatus::MissingCapability);C(!t.delay_finished[0]&&t.delay_elapsed[0]==.2f);
 std::printf("checks=%u failures=%u\n",checks,bad);return bad?1:0;
}
