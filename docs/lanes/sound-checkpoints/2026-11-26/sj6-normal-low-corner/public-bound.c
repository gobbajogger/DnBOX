#include "normal_coeff.h"
#include "leaf.h"
#include <stdio.h>
#include <stdlib.h>
static double db(double e,double s){return 10*log10(e/s);}
int main(void)
{
 const double hz[]={12,30,100,300,20000},damping[]={0.1123311,.2,1};
 for(unsigned r=0;r<3;r++)for(unsigned f=0;f<5;f++){
    double E=0,P=0,D=0,R=0,RP=0,RD=0,S=0,SP=0,SD=0;float mp=0,md=0;
    const sj6_normal_coeff c0=sj6_normal_coefficients(44100.,hz[f],damping[r]);
    const sj6_normal_coeff c1=sj6_normal_coefficients(44100.,20000.,damping[r]);
    sj6_normal_hist hd={0};nf_hist hp={0},hh={0};
    for(unsigned n=0;n<110592;n++){
      const float ph=2*3.141592653589793*(double)n/44100;
      float x=.14f*sinf(ph*220)+.12f*sinf(ph*261.62556f)+.16f*sinf(ph*16.f)+.023f;
      if(n<8192||n>=98304)x=0.f;
      if(n>=65536&&n<98304)x*=expf(-.0005f*(n-65536));
      const sj6_normal_coeff *c=(n<32768||n>=49152)?&c0:&c1;
      const nf_coef p={(float)c->b0,(float)c->b1,(float)c->b2,(float)c->a1,(float)c->a2};
      const nf_coef d={(float)c->b0,(float)c->b1,(float)c->b2,(float)(1.+c->a1+c->a2),(float)c->a2};
      double a=sj6_normal_receiver(&hd,c,x),b=normal_plain(&hp,&p,x)-a,t=normal_delta(&hh,&d,x)-a;
      double a2=a*a,b2=b*b,t2=t*t;
      E+=a2;P+=b2;D+=t2;mp=fmaxf(mp,fabsf(b));md=fmaxf(md,fabsf(t));
      if(n>=65536){R+=a2;RP+=b2;RD+=t2;}
      if((n>=32768&&n<33024)||(n>=49152&&n<49408)){S+=a2;SP+=b2;SD+=t2;}
    }
    printf("%gHz k=%g overall %.3f %.3f release %.3f %.3f step %.3f %.3f absmax %.8f %.8f\n",hz[f],damping[r],db(P,E),db(D,E),db(RP,R),db(RD,R),db(SP,S),db(SD,S),mp,md);
 }
}
