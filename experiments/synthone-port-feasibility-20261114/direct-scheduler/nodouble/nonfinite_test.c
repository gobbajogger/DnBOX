/* Regression: non-finite edges reachable from Synth One parameter minimums. */
#include <stdio.h>
#include <math.h>
#include <string.h>
#include "exact.h"
static unsigned bad=0,n=0;
static void eq(const char*w,float a,float b){n++;if(memcmp(&a,&b,4)){bad++;printf("MISMATCH %s nd=%a ref=%a\n",w,a,b);}}
int main(){
 eq("exp2(-inf)",s1_exp2_f64(-INFINITY),(float)exp2(-INFINITY));eq("exp(-inf)",s1_exp_f64(-INFINITY),(float)exp(-INFINITY));
 eq("exp2(+inf)",s1_exp2_f64(INFINITY),(float)exp2(INFINITY));eq("exp(+inf)",s1_exp_f64(INFINITY),(float)exp(INFINITY));
 float srs[]={44100,48000,96000};
 for(int i=0;i<3;i++){float sr=srs[i];
  float c1,c2;s1_port_coeffs(s1_port_onedsr(sr),0.f,&c1,&c2);float o=(float)(1.0/sr),r2=(float)pow(0.5,o/0.f),r1=(float)(1.0-r2);eq("port c2 htime=0",c2,r2);eq("port c1 htime=0",c1,r1);
  eq("adsr pole tau=0",s1_adsr_pole(0.f,sr),(float)exp(-1.0/(0.f*sr)));
  float t[]={0.0005f,0.004f,0.005f,0.05f,0.5f,2.0f};for(int k=0;k<6;k++)eq("adsr pole",s1_adsr_pole(t[k],sr),(float)exp(-1.0/(t[k]*sr)));}
 printf("{\"check\":\"nonfinite-edges\",\"cases\":%u,\"mismatches\":%u}\n",n,bad);return bad!=0;}
