#include "micro.h"
#include <assert.h>
#include <stdint.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
int main(void){
 micro_config(1.1f,.98f,.15f,1.f);
 void *a=micro_new(),*b=micro_new(),*c=micro_new();
 assert(a&&b&&c&&micro_size()<=4096);
 float peak=0.0f;
 for(unsigned i=0;i<44100u*8u;i++){
  float f=(i<44100u?.89f:i<88200u?.985f:i<132300u?.25f:.9f);
  float l=i%173u==3u?.21f:0.f,r=i%311u==14u?-.64f:0.f;
  float x,y,m,n,u,v;
  micro_tick(a,f,l,r,&x,&y);micro_tick(b,1.f-f,-r,l,&m,&n);
  micro_tick(c,f,l,r,&u,&v);
  assert(memcmp(&x,&u,4)==0&&memcmp(&y,&v,4)==0);
  assert(isfinite(x)&&isfinite(y)&&isfinite(m)&&isfinite(n));
  if(fabsf(x)>peak)peak=fabsf(x);
 }
 printf("independent_owners_frames=352800 current_owner_bytes=%zu peak=%g\n",micro_size(),peak);
 micro_drop(a);micro_drop(b);micro_drop(c);
}
