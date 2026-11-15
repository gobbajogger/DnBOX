#include "scheduler.hpp"
#include <cstdio>
#include <cstring>
#include <climits>
#include <limits>
#include <initializer_list>
using namespace s1scheduler;
static unsigned checks=0,failures=0;
#define C(x) do{++checks;if(!(x)){++failures;std::fprintf(stderr,"FAIL checked domain line%d: %s\n",__LINE__,#x);}}while(0)
int main(){
 Held h;NoteNumber n={60,0,100};C(h.insert_checked(n,0));C(h.count==1&&h[0].noteNumber==60);C(h.remove_checked(0));C(h.count==0);
 for(int i=0;i<128;i++)C(h.insert_checked({i,0,i%128},static_cast<size_t>(i)));
 unsigned char before[sizeof h];std::memcpy(before,&h,sizeof h);C(!h.insert_checked(n,128));C(!std::memcmp(before,&h,sizeof h));C(!h.remove_checked(128));C(!h.remove_checked(SIZE_MAX));C(!std::memcmp(before,&h,sizeof h));
 for(int bad:{-1,129,INT_MIN,INT_MAX}){h.count=bad;std::memcpy(before,&h,sizeof h);C(!h.insert_checked(n,0));C(!h.remove_checked(0));C(!std::memcmp(before,&h,sizeof h));}
 h.count=0;float p[150]={0};EmptySequencer d;C(d.process_checked(p,h));C(!d.process_checked(nullptr,h));d.pending_empty=false;C(!d.process_checked(p,h));d.pending_empty=true;p[arpIsOn]=1;C(!d.process_checked(p,h));p[arpIsOn]=std::numeric_limits<float>::quiet_NaN();C(!d.process_checked(p,h));p[arpIsOn]=0;h.count=-1;C(!d.process_checked(p,h));h.count=129;C(!d.process_checked(p,h));
 sp_data sp{};sp.sr=48000;sp_ftbl rows[52]{};sp_ftbl*ptrs[52];float bands[13];for(unsigned i=0;i<52;i++){rows[i].size=4096;ptrs[i]=&rows[i];}for(unsigned i=0;i<13;i++)bands[i]=static_cast<float>(i);
 sp_oscmorph2d osc{};C(init_morph_checked(&sp,&osc,ptrs,4,13,bands,0)==SP_OK);C(osc.freq==440.f&&osc.amp==.2f&&osc.nft==4&&osc.nbl==13&&osc.lphs==0&&osc.iphs==0);
 unsigned char ob[sizeof osc];std::memcpy(ob,&osc,sizeof osc);rows[51].size=4095;C(init_morph_checked(&sp,&osc,ptrs,4,13,bands,0)==SP_NOT_OK);C(!std::memcmp(ob,&osc,sizeof osc));rows[51].size=4096;bands[12]=-1;C(init_morph_checked(&sp,&osc,ptrs,4,13,bands,0)==SP_NOT_OK);C(!std::memcmp(ob,&osc,sizeof osc));bands[12]=12;C(init_morph_checked(&sp,&osc,ptrs,4,13,bands,.25f)==SP_NOT_OK);C(!std::memcmp(ob,&osc,sizeof osc));
 std::printf("{\"checks\":%u,\"failures\":%u}\n",checks,failures);return failures?1:0;
}
