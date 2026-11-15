#include "conductor.hpp"
#include <cstdio>
#include <string>
#include <limits>
using namespace original_conductor_v1;
struct Fake {std::string trace;std::uint64_t gen=3;bool can_lock=true,can_apply=true;float value=-9.f;};
static bool lock(void* p)noexcept{auto& f=*static_cast<Fake*>(p);f.trace+='L';return f.can_lock;}
static std::uint64_t gen(void* p)noexcept{auto& f=*static_cast<Fake*>(p);f.trace+='G';return f.gen;}
static bool apply(void* p,float v)noexcept{auto& f=*static_cast<Fake*>(p);f.trace+='A';f.value=v;return f.can_apply;}
static void clear(void* p)noexcept{static_cast<Fake*>(p)->trace+='C';}
static void unlock(void* p)noexcept{static_cast<Fake*>(p)->trace+='U';}
int main(){int checks=0,bad=0;
#define C(x) do{++checks;if(!(x)){++bad;std::fprintf(stderr,"FAIL %d %s\n",__LINE__,#x);}}while(0)
 for(unsigned captured=0;captured<8;captured++)for(bool alive:{false,true})for(bool locks:{false,true})for(bool applies:{false,true}){
 Fake f;f.can_lock=locks;f.can_apply=applies;DeferredBackend b{&f,lock,gen,apply,clear,unlock};auto s=execute_deferred_volume(alive,captured,1.32f,b);
 auto expected=!alive?DeferredStatus::ExpiredOwner:!locks?DeferredStatus::LockFailed:captured!=3?DeferredStatus::StaleGeneration:!applies?DeferredStatus::ApplyFailed:DeferredStatus::Applied;
 C(s==expected);C(f.trace==(!alive?"":!locks?"L":captured!=3?"LGU":!applies?"LGAU":"LGACU"));
 }
 Fake f;DeferredBackend b{&f,lock,gen,nullptr,clear,unlock};C(execute_deferred_volume(true,3,1.f,b)==DeferredStatus::MissingCapability);C(f.trace.empty());
 b.apply_volume=apply;C(execute_deferred_volume(true,3,std::numeric_limits<float>::infinity(),b)==DeferredStatus::Invalid);C(f.trace.empty());
 std::printf("checks=%d failures=%d\n",checks,bad);return bad?1:0;
}
