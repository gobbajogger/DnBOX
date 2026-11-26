#include <assert.h>
#include <string.h>
#include <stddef.h>
#include <math.h>
#include "hp_zero_leaf.h"
static const float *tape;
static unsigned total,at;
static int take,physical,prefix;
static sj6_zero_fx_channel st[2];
static float fx[2],fy[2];
static unsigned counts[3];
__attribute__((visibility("default"))) void upstream(const float *mem,unsigned n,int on,int guard,int stages)
{
 tape=mem;total=n;at=0;take=on;physical=guard;prefix=stages;
 assert(!guard || take);
 memset(st,0,sizeof st);memset(fx,0,sizeof fx);memset(fy,0,sizeof fy);memset(counts,0,sizeof counts);
}
__attribute__((visibility("default"))) unsigned reached_frames(void){return at;}
__attribute__((visibility("default"))) void physical_counts(unsigned *c){memcpy(c,counts,sizeof counts);}
void postvoice_signal(float*l,float*r,unsigned n,float hpf)
{
 assert(n && at+n<=total);
 /* This exact Raw profile maps panel zero to compact cached 0 Hz.
    Do not force the authentic Apple HP floor inside this attribution. */
 assert(hpf==-1.f);
 if(take){memcpy(l,tape+at,n*sizeof(float));memcpy(r,tape+total+at,n*sizeof(float));}
 if(physical){
  if(prefix&1)for(unsigned i=0;i<n;i++){l[i]*=(5.f/6.f);r[i]*=(5.f/6.f);}
  for(unsigned i=0;i<n;i++){l[i]=sj6_hp10_44100(&st[0],l[i]);r[i]=sj6_hp10_44100(&st[1],r[i]);}
  counts[0]+=n;
 }
 at+=n;
}

void physical_crusher(float*l,float*r,unsigned n,int active,float step) {
 if(!physical)return;
 assert(!active && step==1.f);
 for(unsigned i=0;i<n;i++){l[i]=sj6_zero_crusher_44100(&st[0],l[i]);r[i]=sj6_zero_crusher_44100(&st[1],r[i]);}
 if(prefix&2)for(unsigned i=0;i<n;i++){l[i]=(float)((double)l[i]*0x1.6a09e5e333598p-1);r[i]=(float)((double)r[i]*0x1.6a09e6ecb41fep-1);}
 counts[1]+=n;
}
void physical_terminal(float*l,float*r,unsigned n) {
 if(!physical)return;
 float *z[2]={l,r};
 for(unsigned c=0;c<2;c++)for(unsigned j=0;j<n;j++){
   float x=z[c][j];
   z[c][j]=fy[c]=fmaf(.999f,fy[c],x-fx[c]);
   fx[c]=x;
 }
 counts[2]+=n;
}
typedef void (*oracle_fn)(float*,float*,unsigned,unsigned);
static oracle_fn callback;
static int substitute;
__attribute__((visibility("default"))) void j60_bind(oracle_fn f,int use){
 callback=f;substitute=use;
}
int j60_probe(float*l,float*r,unsigned n,unsigned mode){
 if(!callback)return 0;
 assert(physical && take);
 callback(l,r,n,mode);
 return substitute;
}
