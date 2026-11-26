#include <math.h>
#include <assert.h>
#include <stdint.h>
/* Diagnostic: the separate operations deliberately differ from fused rounding. */
static inline float split_mul_add(float a,float b,float c) { return a*b+c; }
/* Fixed-coefficient transposed first-order all-pass. Coefficient
 * replacement with persistent history is not equivalent to direct form. */
typedef struct {float up[2][2],down[2][2],pole[4],condition;} OTA;
_Static_assert(sizeof(OTA)==52, "transposed history layout");
static inline float ap_pair(float x,float z[2],float a,float b) {
    float mid=split_mul_add(a,x,z[0]);
    z[0]=x-a*mid;
    float y=split_mul_add(b,mid,z[1]);
    z[1]=mid-b*y;
    return y;
}
static float clock_2x(OTA *s,float x,const float *q,float g,float high,float blend,float outgain,float gz,float gz_inverse) {
    const float dv=(x-s->condition)*high;
    const float lp=s->condition+dv;
    s->condition=dv+lp;
    const float passed=(x-lp)*q[4];
    float estimate[4], z=passed;
    for(unsigned i=0;i<4;i++) {
        z=split_mul_add(gz,z,gz_inverse*s->pole[i]);
        estimate[i]=z;
    }
    z=passed;
    for(unsigned i=0;i<4;i++) {
        const float p=estimate[i], last=s->pole[i];
        const float nonlinear=tanhf(z-p);
        const float derivative=g*split_mul_add(-nonlinear,nonlinear,1.f);
        const float inv=1.f/(1.f+derivative);
        const float residual=-split_mul_add(-g,nonlinear,p-last)*inv;
        const float y=p+residual;
        s->pole[i]=split_mul_add(2.f,y,-last);
        z=p;
        estimate[i]=y;
    }
    const float lo2=estimate[1];
    const float lo4=estimate[3]*outgain;
    if(blend==1.f)return lo4;
    if(blend==0.f)return lo2;
    return split_mul_add(blend,lo4-lo2,lo2);
}
static inline float physical_tick(OTA *s, float x,const float *params,const float *coeff) {
    const float g=params[0];
    const float gz=g/(1.f+g);
    const float gz_inverse=1.f-gz;
    float a=ap_pair(x,s->up[0],coeff[0],coeff[1]);
    float b=ap_pair(x,s->up[1],coeff[2],coeff[3]);
    a=clock_2x(s,a,coeff,g,params[1],params[2],params[3],gz,gz_inverse);
    b=clock_2x(s,b,coeff,g,params[1],params[2],params[3],gz,gz_inverse);
    b=ap_pair(b,s->down[0],coeff[0],coeff[1]);
    a=ap_pair(a,s->down[1],coeff[2],coeff[3]);
    return 40.f*((a+b)*.5f);
}
__attribute__((visibility("default"))) void physical_replay(const float *x,float *result,
     const float *params,const float *coeff,unsigned n) {
    OTA s={0};
    for(unsigned i=0;i<n;i++) result[i]=physical_tick(&s,x[i],params,coeff);
}
