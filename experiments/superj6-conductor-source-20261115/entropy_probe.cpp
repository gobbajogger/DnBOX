#include "entropy_table.hpp"
#include <cstdio>
#include <cstring>
using namespace original_conductor_v1;
struct Tape{std::uint64_t word;unsigned position;};
static bool next(void* p,std::uint64_t& value)noexcept{Tape& t=*static_cast<Tape*>(p);if(t.position++==0){value=0;return true;}t.word=t.word*6364136223846793005ULL+1442695040888963407ULL;value=t.word;return true;}
int main(){for(unsigned i=0;i<1000;i++){Tape t{std::uint64_t(i)+1,0};float table[256];auto r=build_entropy_table12({&t,next},1000,table);if(r.status!=EntropyStatus::Complete)return 1;std::printf("%u %llu",i,(unsigned long long)r.requests);for(float f:table){std::uint32_t u;std::memcpy(&u,&f,4);std::printf(" %08x",u);}std::printf("\n");}}
