#include "s1c_front.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
/* Compiler uses exact front-end helper. Stub is only observation/acceptance,
 * not target-fit: give every physically admissible time premium scratch. */
void s1c_set(s1c *s,unsigned id,float x){s->t[id]=x;}
int s1c_delay_time_fit(s1c *s,float x,int cold){
 (void)s; (void)cold;
 return x>0.f && x<=2.5f;
}
int main(void){
 s1c q={0};float lo,hi,min=3.f,max=0;
 s1c_front_bounds(43,&lo,&hi);
 assert(lo==.0003628117914f&&hi==2.5f);
 const float bpm[]={-30.f,0.f,12.f,30.f,48.f,95.f,120.f,173.f,200.f,300.f,1600.f};
 const float direct[]={-32767.f,0.f,.0003628117914f,.12345f,1.75f,2.5f,2.50001f,4.999f,9000.f};
 unsigned rejected=0,tested=0,admitted=0;
 for(unsigned i=0;i<sizeof bpm/sizeof bpm[0];++i)for(unsigned sync=0;sync<=1;++sync){
   q.delay_time_code=0;
   assert(s1c_front_set(&q,64,bpm[i])&&s1c_front_set(&q,139,(float)sync));
   for(unsigned j=0;j<sizeof direct/sizeof*direct;j++){
      assert(s1c_front_set(&q,43,direct[j]));assert(q.t[43]>=lo&&q.t[43]<=hi);tested++;
   }
   for(unsigned c=0;c<256;c++){
      int ok=s1c_front_delay_apply(&q,c,1);
      if(c>143u){assert(!ok);rejected++;}
      if(ok){float x=q.t[43];assert(x>=lo&&x<=hi);if(x<min)min=x;if(x>max)max=x;admitted++;}
      assert(s1c_front_set(&q,64,bpm[(i+1)%(sizeof bpm/sizeof bpm[0])]));
      assert(q.t[43]>=lo&&q.t[43]<=hi);tested++;
      assert(s1c_front_set(&q,64,bpm[i]));assert(q.t[43]>=lo&&q.t[43]<=hi);tested++;
   }
 }
 printf("actual front helper bounded %u writes + %u admitted codes, %u invalid refusals, seen range %.11g..%.11g and public bound %.11g..%.11g\n",tested,admitted,rejected,min,max,lo,hi);
}
