/* Isolated source-aware approximation, not admitted production. */
#include "fastmath.h"
#include "si_table.h"
static float h_wrap(float t){return t-fm_floorf(t+.5f);}
static float h_si(float z,float sn,float cs){
 float a=fm_fabsf(z),y;
 if(a<16.f){float p=a*16.f;unsigned i=(unsigned)p;float f=p-i;y=si_table[i]+f*(si_table[i+1]-si_table[i]);}
 else {float u=1.f/a,q=u*u;
  float F=u*(1.f+q*(-2.f+q*(24.f+q*(-720.f+q*(40320.f-q*3628800.f)))));
  float G=q*(1.f+q*(-6.f+q*(120.f+q*(-5040.f+q*(362880.f-q*39916800.f)))));
  y=1.5707963267948966f-cs*F-(z<0.f?-sn:sn)*G;
 }
 return z<0.f?-y:y;
}
static float h_sum(unsigned n,float t){
 if(!n)return 0.f;t=h_wrap(t);
 if(n<=5){float y=0.f;for(unsigned k=1;k<=n;k++)y+=fm_sin_turns(k*t)/k;return y;}
 float m=n+.5f,turn=m*t,x=FM_TWO_PI*t,z=m*x;
 float sn=fm_sin_turns(turn),cs=fm_sin_turns(turn+.25f);
 return h_si(z,sn,cs)-x*.5f-(z*cs-sn)/(24.f*m*m);
}
static float h_cos_sum(unsigned n,float t){
 if(!n)return 0.f;t=h_wrap(t);
 if(n<=5){float y=0.f;for(unsigned k=1;k<=n;k++)y+=fm_sin_turns(k*t+.25f)/(k*k);return y;}
 float inv=1.f/n,q=inv*inv;
 float c0=1.6449340668482264f-inv+.5f*q-inv*q*(1.f/6.f-q*(1.f/30.f-q*(1.f/42.f)));
 float m=n+.5f,turn=m*t,x=FM_TWO_PI*t,z=m*x;
 float sn=fm_sin_turns(turn),cs=fm_sin_turns(turn+.25f);
 return c0-x*h_si(z,sn,cs)-(cs-1.f)/m+x*x*.25f+(x*sn+2.f*(cs-1.f)/m)/(24.f*m*m);
}
static float reuse_wave(unsigned shape,unsigned sample,unsigned n){
 float t=sample*(1.f/4096.f);
 if(!n){if(shape==0)return t<.5f?4.f*t-1.f:3.f-4.f*t;if(shape==3)return 2.f*t-1.f;return t<(shape==1?.5f:.125f)?-1.f:1.f;}
 if(shape==3)return -.625f*h_sum(n,t);
 if(shape==1)return -1.282051282051282f*(h_sum(n,t)-.5f*h_sum(n/2,t*2.f));
 if(shape==2)return .75f-.63661977236758134f*(h_sum(n,t)-h_sum(n,t-.125f));
 return -.8106913388749843f*(h_cos_sum(n,t)-.25f*h_cos_sum(n/2,t*2.f));
}
