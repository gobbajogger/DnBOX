#include "juno6.h"
#include "fastmath.h"
#include <math.h>
#include <string.h>
#include "native-donor.inc"
__attribute__((visibility("default"))) void witness(float *l,float *r,unsigned n,unsigned mode) {
 juno6_chorus_t c={0};
 c.sample_rate=96000.f;
 c.factory_mode=(uint8_t)(mode+JUNO6_CHORUS_OFF);
 uint8_t b[4]={1,0,0,128};juno6_j60_config(&c,b);
 if(!mode) {
  for(unsigned at=0;at<n;at+=128)juno6_j60_process(&c,l+at,r+at,n-at<128?n-at:128);
  return;
 }
 for(unsigned i=0;i<n;i++) {
  const float x[2]={l[i],r[i]};
  j60_core(&c,&c.j60,mode,x);
  l[i]=(float)(1.17*(double)c.j60.wet[0]);r[i]=(float)(1.17*(double)c.j60.wet[1]);
 }
}
