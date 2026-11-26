#include <stdio.h>
#include <math.h>
#include <assert.h>
#include "warm_amp.h"
int main(void){
 sj6_warm_amp a;
 float a0,d,s,r;assert(scanf("%f %f %f %f",&a0,&d,&s,&r)==4);
 sj6_amp_config_panel(&a,44100.f,a0,d,s,r);
 unsigned f,n,ev;
 while(scanf("%u %u %u",&f,&n,&ev)==3){
  if(ev==1) sj6_amp_on(&a);
  else if(ev==2) sj6_amp_off(&a);
  else if(ev==3) sj6_amp_restart(&a);
  printf("%u %u %d %a\n",f,n,a.stage,a.out);
  sj6_amp_prep(&a,n);
  for(unsigned i=0;i<n;i++)sj6_amp_sample(&a);
 }
}
