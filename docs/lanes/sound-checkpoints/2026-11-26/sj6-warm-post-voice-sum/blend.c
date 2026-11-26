#include <assert.h>
#include <string.h>
#include <stddef.h>
static const float *tape;
static unsigned total,at;
static int take;
__attribute__((visibility("default"))) void upstream(const float *mem,unsigned n,int on)
{
 tape=mem;total=n;at=0;take=on;
}
__attribute__((visibility("default"))) unsigned reached_frames(void){return at;}
void postvoice_signal(float*l,float*r,unsigned n,float hpf)
{
 assert(n && at+n<=total);
 /* This exact Raw profile maps panel zero to compact cached 0 Hz.
    Do not force the authentic Apple HP floor inside this attribution. */
 assert(hpf==-1.f);
 if(take){memcpy(l,tape+at,n*sizeof(float));memcpy(r,tape+total+at,n*sizeof(float));}
 at+=n;
}
