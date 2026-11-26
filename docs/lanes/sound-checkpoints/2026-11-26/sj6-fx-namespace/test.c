#include "fx.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
int main(void){
 sj6_applied_fx a,b,c;
 sj6_fx_legacy(&a);
 assert(sj6_fx_write(&a,107,0)&&sj6_fx_write(&a,109,0)&&!sj6_fx_zero_ready(&a));
 assert(sj6_fx_from_readback(&a,SJ6_G_CONNECTED,0,1,0,1)&&sj6_fx_zero_ready(&a));
 assert(sj6_fx_from_readback(&b,SJ6_G_BYPASSED,0,1,0,1)&&!sj6_fx_zero_ready(&b));
 assert(sj6_fx_from_readback(&c,SJ6_G_CONNECTED,0,1,0,0)&&!sj6_fx_zero_ready(&c));
 b=a;
 assert(sj6_fx_write(&a,108,.4f)&&sj6_fx_write(&a,107,0.f)&&sj6_fx_zero_ready(&a));
 assert(sj6_fx_write(&b,107,0.f)&&sj6_fx_write(&b,108,.4f)&&!sj6_fx_zero_ready(&b));
 assert(a.booster==b.booster&&a.mix!=b.mix);
 assert(sj6_fx_write(&b,108,.25f)&&b.mix==.125f);
 assert(sj6_fx_write(&a,107,1.f)&&a.mix==.3f&&a.booster==1.3f&&!sj6_fx_zero_ready(&a));
 assert(sj6_fx_write(&a,108,0.f)&&a.booster==1.3f&&!sj6_fx_zero_ready(&a));
 assert(sj6_fx_write(&a,107,0.f)&&sj6_fx_zero_ready(&a));
 assert(sj6_fx_write(&a,109,.5f)&&sj6_fx_write(&a,109,0.f)&&!sj6_fx_zero_ready(&a));
 b=a;
 assert(!sj6_fx_write(&a,107,.5f)&&!sj6_fx_write(&a,108,NAN)&&!sj6_fx_write(&a,109,INFINITY)&&!memcmp(&a,&b,sizeof a));
 assert(!sj6_fx_from_readback(&a,0,0,1,0,1)&&!sj6_fx_zero_ready(&a));
 printf("%zu B applied state; write-order, bypass and legacy unknown pass\n",sizeof a);
}
