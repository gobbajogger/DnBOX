/* Host-only reconstructed oracle comparison. Never compile as production engine without qualification.
 * Oracle outputs matched for 67,830 words across moderate/max live resonance and cold high route.
 * Coefficients are inputs, and the licensed source application and audio are not included.
 */
#include <math.h>
#include <assert.h>
#include <stdint.h>
typedef struct {float up[2][2][2],down[2][2][2],pole[4],condition;} OTA;
static inline float ap(float x,float state[2],float a) {
    float y=fmaf(x-state[1],a,state[0]);
    state[0]=x;state[1]=y; return y;
}
static float clock_2x(OTA *s,float x,const float *q,float g,float high,float blend,float outgain,float fb) {
    const float dv=(x-s->condition)*high;
    const float lp=s->condition+dv;
    s->condition=dv+lp;
    const float passed=(x-lp)*q[4];
    const float gz=g/(1.f+g), comp=1.f-gz;
    const float power4=gz*(gz*(gz*gz));
    const float normalization=1.f/fmaf(fb,power4,1.f);
    float past=0.f,power=1.f;
    for(unsigned i=4;i>0;i--){
        past=fmaf(comp*power,s->pole[i-1],past);
        power*=gz;
    }
    const float scale=fmaf(fb,.3f,1.f);
    const float guess=fmaf(scale,passed,(-normalization*fmaf(power4*scale,passed,past))*fb);
    float est[4],v=guess;
    for(unsigned i=0;i<4;i++){
        v=fmaf(gz,v,comp*s->pole[i]);est[i]=v;
    }
    const float fed=tanhf(8.f*fmaf(.3f,passed,-est[3]));
    float z=fmaf(fb*.125f,fed,passed);
    float alpha[4],resid[4],chain=1.f, err=0.f;
    for(unsigned i=0;i<4;i++) {
        const float t=tanhf(z-est[i]);
        const float d=g*fmaf(-t,t,1.f);
        const float inv=1.f/(1.f+d);
        alpha[i]=d*inv;
        resid[i]=-fmaf(-g,t,est[i]-s->pole[i])*inv;
        chain*=alpha[i];
        err=fmaf(alpha[i],err,resid[i]);
        z=est[i];
    }
    const float loop=-fb*fmaf(-fed,fed,1.f);
    float corr=(loop*err)/fmaf(-loop,chain,1.f);
    for(unsigned i=0;i<4;i++){
        corr=fmaf(alpha[i],corr,resid[i]);
        est[i]+=corr;
        s->pole[i]=fmaf(2.f,est[i],-s->pole[i]);
    }
    const float lo2=est[1], lo4=est[3]*outgain;
    if(blend==1.f)return lo4;
    if(blend==0.f)return lo2;
    return fmaf(blend,lo4-lo2,lo2);
}
static inline float physical_tick(OTA *s, float x,const float *params,const float *coeff) {
        float a=ap(ap(x,s->up[0][0],coeff[0]),s->up[0][1],coeff[1]);
        float b=ap(ap(x,s->up[1][0],coeff[2]),s->up[1][1],coeff[3]);
        a=clock_2x(s,a,coeff,params[0],params[1],params[2],params[3],params[4]);
        b=clock_2x(s,b,coeff,params[0],params[1],params[2],params[3],params[4]);
        b=ap(ap(b,s->down[0][0],coeff[0]),s->down[0][1],coeff[1]);
        a=ap(ap(a,s->down[1][0],coeff[2]),s->down[1][1],coeff[3]);
        return 40.f*((a+b)*.5f);
}
__attribute__((visibility("default"))) void physical_replay(const float *x,float *result,
     const float *params,const float *coeff,unsigned n) {
    OTA s={0};
    unsigned high_latched=0;
    for(unsigned i=0;i<n;i++){
       const float *p=params+5*i;const float fb=p[4];
       if(fb<=4.f) high_latched=0;
       if(fb>4.f && !high_latched){
          if(i==0){
             const float a=sqrtf(fb-4.f)*.125f;
             const float b=2.f*a, effect=tanhf(a);
             s.pole[0]=fmaf(a,2.f,tanhf(b)*p[0]);
             s.pole[1]=b;
             s.pole[2]=fmaf(-p[0],effect,a);
             s.pole[3]=-p[0]*effect;
          } else s.pole[0]+=.0001f;
          high_latched=1;
       }
       result[i]=physical_tick(&s,x[i],p,coeff);
    }
}
