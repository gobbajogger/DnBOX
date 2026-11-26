#include <math.h>
#include <assert.h>
#include <stdint.h>
/* Fixed-coefficient transposed first-order all-pass. Coefficient
 * replacement with persistent history is not equivalent to direct form. */
static inline float split_mul_add(float a,float b,float c){return a*b+c;}
typedef struct {float up[2][2],down[2][2],pole[4],condition;} OTA;
_Static_assert(sizeof(OTA)==52, "transposed history layout");
static inline float ap_pair(float x,float z[2],float a,float b) {
    float mid=split_mul_add(a,x,z[0]);
    z[0]=x-a*mid;
    float y=split_mul_add(b,mid,z[1]);
    z[1]=mid-b*y;
    return y;
}
void prepare_tile(const float *source, float *dst){
    const float alpha=source[0]/(1.f+source[0]);
    dst[0]=alpha; dst[1]=1.f-alpha;
    dst[2]=source[1]; dst[3]=source[2]; dst[4]=source[3];
}
static inline float cond_2x(OTA *s,float x,const float *q,const float *c) {
    const float dv=(x-s->condition)*c[2];
    const float lp=s->condition+dv;
    s->condition=dv+lp;
    return (x-lp)*q[4];
}
static inline void clock_pair(OTA *s,float *a,float *b,const float *q,const float *c) {
    float x=cond_2x(s,*a,q,c),y=cond_2x(s,*b,q,c);
    float lo2a=0.f,lo2b=0.f;
    const float g=c[0],z=c[1];
    for(unsigned i=0;i<4;i++){
        const float old=s->pole[i];
        x=split_mul_add(g,x,z*old);
        const float between=split_mul_add(2.f,x,-old);
        y=split_mul_add(g,y,z*between);
        s->pole[i]=split_mul_add(2.f,y,-between);
        if(i==1){lo2a=x;lo2b=y;}
    }
    const float lo4a=x*c[4],lo4b=y*c[4],blend=c[3];
    if(blend==1.f){*a=lo4a;*b=lo4b;return;}
    if(blend==0.f){*a=lo2a;*b=lo2b;return;}
    *a=split_mul_add(blend,lo4a-lo2a,lo2a);
    *b=split_mul_add(blend,lo4b-lo2b,lo2b);
}
static inline float physical_tick(OTA *s, float x,const float *params,const float *coeff) {
    float a=ap_pair(x,s->up[0],coeff[0],coeff[1]);
    float b=ap_pair(x,s->up[1],coeff[2],coeff[3]);
    clock_pair(s,&a,&b,coeff,params);
    b=ap_pair(b,s->down[0],coeff[0],coeff[1]);
    a=ap_pair(a,s->down[1],coeff[2],coeff[3]);
    return 40.f*((a+b)*.5f);
}
__attribute__((visibility("default"))) void physical_replay(const float *x,float *result,
     const float *params,const float *coeff,unsigned n) {
    OTA s={0}; float c[5]; prepare_tile(params,c);
    for(unsigned i=0;i<n;i++) result[i]=physical_tick(&s,x[i],c,coeff);
}

float physical_tile_tick(OTA *s,float x,const float *tile,const float*coeff){return physical_tick(s,x,tile,coeff);}
