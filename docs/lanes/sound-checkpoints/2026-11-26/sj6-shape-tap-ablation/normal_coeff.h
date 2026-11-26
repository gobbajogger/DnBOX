#ifndef DNBX_HOST_SJ6_NORMAL_COEFF_H
#define DNBX_HOST_SJ6_NORMAL_COEFF_H
#include "normal_receiver.h"
/* Build9 ancestor LUT; host-only exact benchmark computes the two table
 * entries on demand; the original builds its 2048 Float32 entries once. */
static inline float sj6_normal_lut_entry(unsigned i)
{
    const double p=(2.0*((double)i/2048.0))*0x1.921fb54442d18p+1;
    return (float)sin(p);
}
static inline float sj6_normal_lut(float phase)
{
    if(phase>=1.f)phase-=1.f;
    const float p=2048.f*phase;
    const unsigned lo=(unsigned)p, hi=(lo+1u)&2047u;
    const float frac=p-(float)lo;
    const float a=sj6_normal_lut_entry(lo), b=sj6_normal_lut_entry(hi);
    return (float)fma(1.0-(double)frac,(double)a,(double)(b*frac));
}
static inline sj6_normal_coeff sj6_normal_coefficients(double sr,double fc,double damping)
{
    const double f=fmax(12.0,fc), k=fmin(10.0,fmax((double).1f,damping));
    const float phase=(float)(.5*fmin((f+f)/sr,.99));
    const double s=(double)sj6_normal_lut(phase), c=(double)sj6_normal_lut(phase+.25f);
    const double alpha=pow(k,1.91)*s;
    const double beta=((1.-alpha)*.5)/(alpha+1.);
    const double u=beta+.5;
    const double v=u*c;
    const double t=(u-v)*.25;
    return (sj6_normal_coeff){2.*t,4.*t,2.*t,-2.*v,2.*beta};
}
#endif
