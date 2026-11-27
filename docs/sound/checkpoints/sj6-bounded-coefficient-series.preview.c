/* Standalone diagnostic only. 3072/3072 pinned g/k cells on native PI32.
 * SDK libm alternative disagreed on 1536 g and 456 k cells. This bounded
 * version matched all four source cue coefficients; it still missed g on
 * 540/1,000,000 wider host controls including 37 high cells. Target 140.73
 * us per pair and 5060 B isolated with runtime helpers, not production fit.
 * No reference audio/assets included. Do not enable as a firmware default.
 */
/* Finite supported panel/cutoff controls only, no native math library calls.
 * Deliberately retain each original binary32 rounding transition.
 * Comparison experiment: not a claim of correctly rounded libm for all floats. */
#include <stdint.h>
typedef union{uint32_t u;float f;} U;
typedef union{uint64_t u;double d;} D;
static const double lg[12]={0x1.0000000000000p+0,0x1.5555555555555p-2,0x1.999999999999ap-3,0x1.2492492492492p-3,0x1.c71c71c71c71cp-4,0x1.745d1745d1746p-4,0x1.3b13b13b13b14p-4,0x1.1111111111111p-4,0x1.e1e1e1e1e1e1ep-5,0x1.af286bca1af28p-5,0x1.8618618618618p-5,0x1.642c8590b2164p-5};
static const double ex[15]={0x1.0000000000000p+0,0x1.0000000000000p+0,0x1.0000000000000p-1,0x1.5555555555555p-3,0x1.5555555555555p-5,0x1.1111111111111p-7,0x1.6c16c16c16c17p-10,0x1.a01a01a01a01ap-13,0x1.a01a01a01a01ap-16,0x1.71de3a556c734p-19,0x1.27e4fb7789f5cp-22,0x1.ae64567f544e4p-26,0x1.1eed8eff8d898p-29,0x1.6124613a86d09p-33,0x1.93974a8c07c9dp-37};
static const double sn[10]={0x1.0000000000000p+0,-0x1.5555555555555p-3,0x1.1111111111111p-7,-0x1.a01a01a01a01ap-13,0x1.71de3a556c734p-19,-0x1.ae64567f544e4p-26,0x1.6124613a86d09p-33,-0x1.ae7f3e733b81fp-41,0x1.952c77030ad4ap-49,-0x1.2f49b46814157p-57};
static const double cs[11]={0x1.0000000000000p+0,-0x1.0000000000000p-1,0x1.5555555555555p-5,-0x1.6c16c16c16c17p-10,0x1.a01a01a01a01ap-16,-0x1.27e4fb7789f5cp-22,0x1.1eed8eff8d898p-29,-0x1.93974a8c07c9dp-37,0x1.ae7f3e733b81fp-45,-0x1.6827863b97d97p-53,0x1.e542ba4020225p-62};
static double eval(double x,const double *a,unsigned n){
 double r=a[n-1];
 while(--n)r=r*x+a[n-1];
 return r;
}
static double l2(float x) {
 U v={.f=x}; int e=(int)(v.u>>23)-127;
 v.u=(v.u&0x7fffffu)|0x3f800000u;
 double a=(double)v.f;
 if(a>0x1.6a09e667f3bcdp+0){ a*=.5; ++e; }
 double z=(a-1.)/(a+1.);
 return (double)e + 0x1.71547652b82fep+1*z*eval(z*z,lg,12);
}
static double e2(double x) {
 int e=(int)(x<0.0?x-.5:x+.5);
 double f=(x-(double)e)*0x1.62e42fefa39efp-1;
 D s={.u=(uint64_t)(e+1023)<<52};
 return eval(f,ex,15)*s.d;
}
static float tn(float x) {
 double a=(double)x,b=a*a;
 return (float)((a*eval(b,sn,10))/eval(b,cs,11));
}
static inline float mx(float a,float b){return a>b?a:b;}
static inline float mn(float a,float b){return a<b?a:b;}
static inline float fused(float a,float b,float c){return (float)((double)a*(double)b+(double)c);}
float bounded_g(float hz,float fs) {
 const float h=mx(3.f,mn(hz,mn(fs*.9f,30000.f)));
 const float l=(float)l2(h), c=(float)e2((double)l), a=(c*0x1.921fb6p+1f)/(fs+fs);
 return tn(a);
}
static inline float bounded_r_to_k(float r,float hz,float fs) {
 r=mx(0.f,mn(r,1.f));
 float v,k;
 if(r<=.5f) {
  v=r/.9f;
  k=(v*v)*fused(v,-0x1.bae148p+2f,0x1.5d70a4p+3f);
 } else if(r<.75f){
  v=(r-.5f)*4.f;
  k=fused(v,fused(v,fused(v,0x1.90faa0p-1f,-0x1.6852a0p+0f),0x1.972d50p+0f),0x1.178734p+1f);
 } else if(r<.9f){
  v=(r-.75f)/.15f;
  k=fused(v,fused(v,fused(v,-0x1.b0a3d2p-1f,0x1.051eb4p+0f),0x1.59999ap-1f),0x1.933334p+1f);
 } else k=fused(r-.9f,0x1.333334p+0f,4.f);
 hz=mx(3.f,mn(hz,mn(fs*.9f,30000.f)));
 const float t=mx(0.f,mn(1.f,((hz/fs)+(-0x1.ccccccp-2f))/.05f));
 const float excess=mx(k-0x1.f33334p+1f,0.f)*t;
 return fused(-(t*excess),fused(t,-2.f,3.f),k);
}

float bounded_k(float raw,float hz,float fs){
 const float db=(float)(18.99*(double)raw);
 const float v=(float)e2(((double)db*(-.05))*0x1.a934f0979a371p+1);
 const float log10=(float)(l2(mx(v,.0001f))*0x1.34413509f79ffp-2);
 const float r=(-20.f*log10)/0x1.2fd70ap+4f;
 return bounded_r_to_k(r,hz,fs);
}
