#include <math.h>
#include <assert.h>
#include <stdint.h>
typedef struct {float up[2][2][2],down[2][2][2],pole[4],condition;} OTA;
static inline float ap(float x,float state[2],float a) {
    float y=fmaf(x-state[1],a,state[0]);
    state[0]=x;state[1]=y; return y;
}
static float clock_2x(OTA *s,float x,const float *q,float g,float high,float blend,float outgain) {
    const float dv=(x-s->condition)*high;
    const float lp=s->condition+dv;
    s->condition=dv+lp;
    const float passed=(x-lp)*q[4];
    const float gz=g/(1.f+g);
    const float gz_inverse=1.f-gz;
    float estimate[4], z=passed;
    for(unsigned i=0;i<4;i++) {
        z=fmaf(gz,z,gz_inverse*s->pole[i]);
        estimate[i]=z;
    }
    z=passed;
    for(unsigned i=0;i<4;i++) {
        const float p=estimate[i], last=s->pole[i];
        const float nonlinear=tanhf(z-p);
        const float derivative=g*fmaf(-nonlinear,nonlinear,1.f);
        const float inv=1.f/(1.f+derivative);
        const float residual=-fmaf(-g,nonlinear,p-last)*inv;
        const float y=p+residual;
        s->pole[i]=fmaf(2.f,y,-last);
        z=p;
        estimate[i]=y;
    }
    const float lo2=estimate[1];
    const float lo4=estimate[3]*outgain;
    if(blend==1.f)return lo4;
    if(blend==0.f)return lo2;
    return fmaf(blend,lo4-lo2,lo2);
}
static inline float physical_tick(OTA *s, float x,const float *params,const float *coeff) {
        float a=ap(ap(x,s->up[0][0],coeff[0]),s->up[0][1],coeff[1]);
        float b=ap(ap(x,s->up[1][0],coeff[2]),s->up[1][1],coeff[3]);
        a=clock_2x(s,a,coeff,params[0],params[1],params[2],params[3]);
        b=clock_2x(s,b,coeff,params[0],params[1],params[2],params[3]);
        b=ap(ap(b,s->down[0][0],coeff[0]),s->down[0][1],coeff[1]);
        a=ap(ap(a,s->down[1][0],coeff[2]),s->down[1][1],coeff[3]);
        return 40.f*((a+b)*.5f);
}
__attribute__((visibility("default"))) void physical_replay(const float *x,float *result,
     const float *params,const float *coeff,unsigned n) {
    OTA s={0};
    for(unsigned i=0;i<n;i++) result[i]=physical_tick(&s,x[i],params,coeff);
}
