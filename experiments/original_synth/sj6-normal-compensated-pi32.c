/* Scoped, no-libm, no-double-in-hot-loop float expansion. */
#include <stdint.h>
typedef struct {float x1,x2,y1,y2;} dfi_state;
typedef struct {float hi,lo;} Pair;
static inline float high12(float x) {
    union {float v;uint32_t u;} b={.v=x};
    b.u&=UINT32_C(0xfffff000);
    return b.v;
}
typedef struct {float c0,c1,ch,cl;} prepared;
static inline prepared prep(Pair c) {
    float h=high12(c.hi);
    return (prepared){c.hi,c.lo,h,c.hi-h};
}
static inline Pair product(float x,prepared c) {
    float p=x*c.c0;
    float xh=high12(x),xl=x-xh,ch=c.ch,cl=c.cl;
    float e=((xh*ch-p)+xh*cl)+xl*ch;
    e+=xl*cl;
    e+=x*c.c1;
    return (Pair){p,e};
}
static inline Pair add(Pair a,Pair b) {
    float s=a.hi+b.hi;
    float t=s-a.hi;
    float e=(a.hi-(s-t))+(b.hi-t);
    e+=a.lo+b.lo;
    /* Maintain nonoverlapping high/low word until the final rounding.
     * The low accumulator preserves product and high-word addition tails. */
    return (Pair){s,e};
}
void normal_dfi_pair_run(dfi_state *restrict s,const float *x,float *y,unsigned n,const Pair *restrict c) {
    const prepared b0=prep(c[0]),b1=prep(c[1]),b2=prep(c[2]),a1=prep(c[3]),a2=prep(c[4]);
    for(unsigned i=0;i<n;i++) {
        Pair a=product(s->x1,b1);
        a=add(a,product(x[i],b0));
        a=add(a,product(s->x2,b2));
        a=add(a,product(-s->y1,a1));
        a=add(a,product(-s->y2,a2));
        const float rounded=a.hi+a.lo;
        s->x2=s->x1;s->x1=x[i];s->y2=s->y1;s->y1=rounded;
        y[i]=rounded;
    }
}
