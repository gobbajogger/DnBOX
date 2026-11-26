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
static inline float prepare(OTA *s,float x,const float *q,float high){
    const float dv=(x-s->condition)*high;
    const float lp=s->condition+dv;
    s->condition=dv+lp;
    return (x-lp)*q[4];
}
static inline float refine(float x,float p,float last,float g){
    const float nonlinear=tanhf(x-p);
    const float derivative=g*split_mul_add(-nonlinear,nonlinear,1.f);
    const float inv=1.f/(1.f+derivative);
    const float residual=-split_mul_add(-g,nonlinear,p-last)*inv;
    return p+residual;
}
static inline void clock_pair(OTA *s,float *first,float *second,const float*q,
                              float g,float high,float blend,float outgain,float gz,float zi) {
    float a=prepare(s,*first,q,high),b=prepare(s,*second,q,high);
    float lo2a=0.f,lo2b=0.f,lo4a=0.f,lo4b=0.f;
    for(unsigned i=0;i<4;i++){
        const float old=s->pole[i];
        const float pa=split_mul_add(gz,a,zi*old);
        lo4a=refine(a,pa,old,g);
        const float mid=split_mul_add(2.f,lo4a,-old);
        const float pb=split_mul_add(gz,b,zi*mid);
        lo4b=refine(b,pb,mid,g);
        s->pole[i]=split_mul_add(2.f,lo4b,-mid);
        if(i==1){lo2a=lo4a;lo2b=lo4b;}
        a=pa;b=pb;
    }
    lo4a*=outgain;lo4b*=outgain;
    if(blend==1.f){*first=lo4a;*second=lo4b;return;}
    if(blend==0.f){*first=lo2a;*second=lo2b;return;}
    *first=split_mul_add(blend,lo4a-lo2a,lo2a);
    *second=split_mul_add(blend,lo4b-lo2b,lo2b);
}
static inline float physical_tick(OTA *s, float x,const float *params,const float *coeff) {
    const float g=params[0];
    const float gz=g/(1.f+g);
    const float gz_inverse=1.f-gz;
    float a=ap_pair(x,s->up[0],coeff[0],coeff[1]);
    float b=ap_pair(x,s->up[1],coeff[2],coeff[3]);
    clock_pair(s,&a,&b,coeff,g,params[1],params[2],params[3],gz,gz_inverse);
    b=ap_pair(b,s->down[0],coeff[0],coeff[1]);
    a=ap_pair(a,s->down[1],coeff[2],coeff[3]);
    return 40.f*((a+b)*.5f);
}
__attribute__((visibility("default"))) void physical_replay(const float *x,float *result,
     const float *params,const float *coeff,unsigned n) {
    OTA s={0};
    for(unsigned i=0;i<n;i++) result[i]=physical_tick(&s,x[i],params,coeff);
}
