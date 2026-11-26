/* Source-only private oracle interpolation. No embedded data. */
#include <math.h>
#include <assert.h>
static const float *store;
__attribute__((visibility("default"))) void witness_bank(const float *x){assert(x);store=x;}
int witness_has_bank(void){return store!=0;}
static inline float cubic(const float *x,float p) {
    float u=p*1024.f; unsigned i=(unsigned)u;
    const float t=u-(float)i,q=.5f*t;
    const float a=x[(i-1)&1023], b=x[i&1023], c=x[(i+1)&1023],d=x[(i+2)&1023];
    float v=b*-5.f;
    v=fmaf(a,2.f,v);
    v=fmaf(c,4.f,v);
    v-=d;
    float w=fmaf(b-c,3.f,d)-a;
    return fmaf(q,fmaf(t,fmaf(t,w,v),c-a),b);
}
__attribute__((visibility("default"))) float sample(float phase,float step,void *ctx) {
  assert(store&&phase>=0.f&&phase<1.f&&step>0.f);
  (void)ctx; unsigned row=0;
  float x=step*256.f;
  while(row<6 && x>=.49f){ x*=.5f; ++row; }
  const float y=cubic(store+row*1024,phase);
  if(row==6 || x<=.4f)return y;
  float blend=(x-.4f)/.09f;
  if(blend>1.f)blend=1.f;
  return fmaf(blend,cubic(store+(row+1)*1024,phase)-y,y);
}
