#ifndef SJ6_SCOPED_DRY_SUFFIX
#define SJ6_SCOPED_DRY_SUFFIX
#include <math.h>
#include "hp_zero_leaf.h"
typedef struct {
 sj6_zero_fx_channel pre[2];
 float dc_x[2],dc_y[2];
} sj6_scoped_dry_suffix;
static inline void sj6_pre_dry(sj6_scoped_dry_suffix *s,float *l,float *r,unsigned n) {
 float *z[2]={l,r};
 for(unsigned c=0;c<2;c++)for(unsigned k=0;k<n;k++)
   z[c][k]=sj6_zero_crusher_44100(&s->pre[c],sj6_hp10_44100(&s->pre[c],z[c][k]));
}
static inline void sj6_post_dry(sj6_scoped_dry_suffix *s,float *l,float *r,unsigned n) {
 float *z[2]={l,r};
 for(unsigned c=0;c<2;c++)for(unsigned k=0;k<n;k++){
   float x=z[c][k];
   z[c][k]=s->dc_y[c]=fmaf(.999f,s->dc_y[c],x-s->dc_x[c]);
   s->dc_x[c]=x;
 }
}
#endif
