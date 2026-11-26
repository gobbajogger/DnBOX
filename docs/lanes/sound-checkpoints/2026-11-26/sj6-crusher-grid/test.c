#include "converter.h"
#include <assert.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
int main(void) {
 for(unsigned q=0;q<128;q++) {
  const float x=(float)q/127.f;
  fm_bits_t a,b;
  assert(sj6_crush_percent(108,q,x,&a.f));assert(sj6_crush_percent(109,q,x,&b.f));
  printf("%u %u\n",a.u,b.u);
 }
 sj6_crush_applied a={0};
 const float x=(float)64u/127.f;
 assert(sj6_crush_commit(&a,107,0,0.f)==(SJ6_WET|SJ6_BOOSTER)&&a.wet_pct==0.f&&a.booster==1.f&&!a.cadence_seen);
 assert(sj6_crush_commit(&a,108,64,x)==SJ6_WET&&a.wet_pct>0.f&&a.booster==1.f);
 float wet=a.wet_pct;
 assert(sj6_crush_commit(&a,109,127,1.f)==(SJ6_CADENCE|SJ6_ROUNDING_ZERO)&&a.cadence_pct==6.06f&&a.wet_pct==wet);
 assert(sj6_crush_commit(&a,107,127,1.f)&&a.cadence_pct==6.06f&&a.wet_pct==30.f&&a.booster==1.3f);
 assert(sj6_crush_commit(&a,108,0,0.f)&&a.wet_pct==0.f&&a.booster==1.3f);
 assert(sj6_crush_commit(&a,107,0,0.f)==(SJ6_WET|SJ6_BOOSTER)&&a.booster==1.f&&a.cadence_pct==6.06f);
 const sj6_crush_applied snapshot=a;
 assert(!sj6_crush_commit(&a,109,64,.5f)&&!sj6_crush_commit(&a,108,128,1.f)&&
        !sj6_crush_commit(&a,108,0,-0.f)&&!sj6_crush_commit(&a,108,64,NAN)&&
        !sj6_crush_commit(&a,107,64,x)&&!sj6_crush_commit(&a,110,1,0.f)&&
        !memcmp(&a,&snapshot,sizeof a));
}
