#include "conductor.hpp"
#include <cstdio>
using namespace original_conductor_v1;
int main(){int checks=0,bad=0;
#define C(x) do{++checks;if(!(x)){++bad;std::fprintf(stderr,"FAIL %d %s\n",__LINE__,#x);}}while(0)
 RestoreControl controls[]={{7,7},{126,126},{78,78},{79,79},{218,218}};RestoreCommand commands[12];
 for(unsigned f=0;f<512;f++){
  RestoreState s{4,{{{7,126},{.4f,.8f},bool(f&1),bool(f&2),bool(f&4)},{{78,79},{.2f,.3f},bool(f&8),bool(f&16),bool(f&32)},{{7,7},{.1f,.9f},bool(f&64),bool(f&128),bool(f&256)}},{false,false,false}},r{};
  std::size_t n=999;auto st=restore(s,false,controls,5,7,commands,12,n,r);C(st==Status::State);
  if(st!=Status::State)continue;
  unsigned eligible=0;for(unsigned i=0;i<3;i++){bool on=s.routes[i].enabled&&s.routes[i].retrigger&&s.routes[i].snapback;C(r.cycles[i]==on);eligible+=on;}
  C(n==4*eligible);if(r.cycles[2]){C(commands[n-4].value==.1f&&commands[n-2].value==.9f);C(commands[n-4].kind==RestoreCommand::Kind::CutoffTarget);}
 }
 RestoreState s{4,{{{7,126},{.4f,.8f},false,false,false},{{78,79},{.2f,.3f},false,false,false},{{7,7},{.1f,.9f},false,false,false}},{false,false,false}},r{};std::size_t n=999;
 C(restore(s,true,controls,5,7,commands,12,n,r)==Status::State);C(n==4&&r.cycles[2]&&!r.cycles[0]);
 n=999;C(restore(s,true,controls,5,7,commands,3,n,r)==Status::Capacity);C(n==999);
 n=999;C(restore(s,true,controls,5,0,commands,12,n,r)==Status::MissingCapability);C(n==999);
 std::printf("checks=%d failures=%d\n",checks,bad);return bad?1:0;
}
