/* Tested diagnostic model: separate Sub release VCA scheduling, same owner
 * and no interpolation/phase changes. Does not claim separate authentic
 * per-sampler filter: envelope compensation occurs before compact shared OTA. */
#include "warm_amp.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static sj6_warm_amp model[6];
static unsigned mode,gated[6],last[6];
static float a,d,s,r;
static unsigned at,n;
__attribute__((visibility("default"))) void set_shape(int x){mode=x; memset(gated,0,sizeof gated);memset(last,0xff,sizeof last);}
void shape_setup(float p,float q,float t,float u){a=p;d=q;s=t;r=u;}
void shape_reset(unsigned i) {
 assert(i<6);
 sj6_amp_config_panel(&model[i],44100.f,a,d,s,r);
 sj6_amp_on(&model[i]);gated[i]=1;last[i]=~0u;
}
void shape_block(unsigned begin,unsigned count) {at=begin;n=count;}
float shape_carrier(unsigned k,unsigned i,unsigned gate,float stock,float sample) {
 if(!mode)return sample;
 assert(i<6 && k<n && (last[i]==~0u || last[i]+1==at+k));
 last[i]=at+k;
 sj6_warm_amp *x=&model[i];
 if(gated[i]&&!gate){sj6_amp_off(x);x->remain=0;}
 gated[i]=gate;
 if(k==0 || !x->remain){unsigned f=n-k;sj6_amp_prep(x,f>16?16:f);}
 float level=sj6_amp_sample(x);
 if((at+k>=38434 && at+k<39100))fprintf(stderr,"SHAMP %u %u %a %a\n",at+k,i,(double)level,(double)stock);
 if(gate)return sample;
 assert(stock>0.f || level==0.f);
 return stock>0.f?sample*(level/stock):0.f;
}
