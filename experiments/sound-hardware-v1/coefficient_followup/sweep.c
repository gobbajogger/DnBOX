#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
extern float coeff_fast(float,float,int);
extern float coeff_exp2(float,float,int);
extern float coeff_double_x(float,float,int);
extern float coeff_oracle(float,float,int);
extern float coeff_fixed(float,float,int);
static uint32_t word(float f){uint32_t u;memcpy(&u,&f,4);return u;}
typedef float (*fn)(float,float,int);
int main(void){
 const char *names[]={"fm_exp_float","fm_exp2_float","fm_exp_double_x","fixed_integer"};
 fn methods[]={coeff_fast,coeff_exp2,coeff_double_x,coeff_fixed};
 float rates[]={44100.f,8000.f,48000.f,192000.f};
 float mins[]={.0005f,.005f,.004f,1e-10f};
 const char *roles[]={"attack","decay","amp_release","filter_release_positive"};
 puts("rate,role,method,points,differing_coefficients,max_ulp,max_absolute_error,first_time,first_oracle,first_candidate,worst_time,worst_oracle,worst_candidate");
 for(int rr=0;rr<4;rr++)for(int role=0;role<4;role++)for(int m=0;m<4;m++){
  unsigned count=0,diffs=0,maxulp=0;double maxabs=0;float first=0,worst=0;uint32_t fo=0,fc=0,wo=0,wc=0;
  for(int mode=0;mode<2;mode++)for(int i=0;i<=100000;i++){
   double lo=mins[role];
   float t=(float)(mode? exp(log(lo)+(log(2.)-log(lo))*((double)i/100000.)) : lo+(2.-lo)*((double)i/100000.));
   float o=coeff_oracle(t,rates[rr],role==0),c=methods[m](t,rates[rr],role==0);
   uint32_t u=word(o),v=word(c),d=u>v?u-v:v-u;
   count++;
   if(d){if(!diffs){first=t;fo=u;fc=v;}diffs++;}
   if(d>maxulp){maxulp=d;worst=t;wo=u;wc=v;}
   if(fabs((double)c-o)>maxabs)maxabs=fabs((double)c-o);
  }
  printf("%.0f,%s,%s,%u,%u,%u,%.17g,%.10g,%08x,%08x,%.10g,%08x,%08x\n",rates[rr],roles[role],names[m],count,diffs,maxulp,maxabs,first,fo,fc,worst,wo,wc);
 }
}
