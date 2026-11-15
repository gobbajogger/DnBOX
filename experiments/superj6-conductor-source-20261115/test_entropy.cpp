#include "entropy_table.hpp"
#include <cstdio>
#include <cstddef>
using namespace original_conductor_v1;
struct Tape{const std::uint64_t* words;std::size_t size,pos;};
static bool take(void* p,std::uint64_t& out)noexcept{Tape& t=*static_cast<Tape*>(p);if(t.pos==t.size)return false;out=t.words[t.pos++];return true;}
int main(){unsigned checks=0,bad=0;
#define C(x) do{++checks;if(!(x)){++bad;std::fprintf(stderr,"FAIL %d %s\n",__LINE__,#x);}}while(0)
 float out[256];for(float& f:out){f=9;}

 std::uint64_t maxs[]={UINT64_MAX,UINT64_MAX,UINT64_MAX,UINT64_MAX};Tape t{maxs,4,0};
 auto r=build_entropy_table12({&t,take},4,out);C(r.status==EntropyStatus::Complete);C(r.requests==4&&t.pos==4);for(float f:out)C(f==1.f);
 for(float& f:out){f=9;}
t={maxs,4,0};r=build_entropy_table12({&t,take},3,out);C(r.status==EntropyStatus::BudgetExceeded);C(r.requests==3&&t.pos==3);for(float f:out)C(f==9.f);
 t={maxs,2,0};r=build_entropy_table12({&t,take},9,out);C(r.status==EntropyStatus::SourceFailure);C(r.requests==3&&t.pos==2);for(float f:out)C(f==9.f);
 r=build_entropy_table12({nullptr,nullptr},9,out);C(r.status==EntropyStatus::MissingProvider);C(r.requests==0);
 std::printf("checks=%u failures=%u\n",checks,bad);return bad?1:0;
}
