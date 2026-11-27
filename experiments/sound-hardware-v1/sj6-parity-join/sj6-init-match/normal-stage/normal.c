#include <math.h>
static inline float clip(float x, float lo,float hi) { return x<lo?lo:x>hi?hi:x; }
/* Float32 at the input and output, but the source recurrence is double
 * with fused subtracts and Float32 feedback words. Keep separate histories. */
void run_stage(const double *c,float cutoff,float damping,unsigned n,const float *x,float *y,float *stock)
{
    float x1=0.f,x2=0.f,y1=0.f,y2=0.f;
    float f=0.f,s=0.f;
    float a=clip(6.2831853071795864769f*cutoff/44100.f,.001f,.75f);
    float resonance=clip((float)(-20.*log10((double)damping)/18.99),0.f,1.f);
    for(unsigned i=0;i<n;i++){
        double z=fma(c[0],x[i],c[1]*(double)x1);
        z=fma(c[2],x2,z);
        z=fma(-c[3],y1,z);
        z=fma(-c[4],y2,z);
        y[i]=(float)z;x2=x1;x1=x[i];y2=y1;y1=y[i];
        f=clip(f+a*(x[i]-f-resonance*.8f*s),-4.f,4.f);
        s=clip(s+a*(f-s),-4.f,4.f);
        stock[i]=s;
    }
}
