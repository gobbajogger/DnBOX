#ifndef DNBX_HOST_SJ6_NORMAL_RECEIVER_H
#define DNBX_HOST_SJ6_NORMAL_RECEIVER_H
#include <math.h>
/* Original unsmoothed Main Normal biquad, older source's 0x1003a82ec path.
 * The source narrows y before using it in output feedback. Coefficient
 * refresh is deliberately not a reset or a crossfade. */
typedef struct {double b0,b1,b2,a1,a2;} sj6_normal_coeff;
typedef struct {double x1,x2,y1,y2;} sj6_normal_hist;
static inline float sj6_normal_receiver(sj6_normal_hist *h,const sj6_normal_coeff *c,float input)
{
    const double x=input;
    double y=fma(c->b0,x,c->b1*h->x1);
    y=fma(c->b2,h->x2,y);
    y=fma(-c->a1,h->y1,y);
    y=fma(-c->a2,h->y2,y);
    const float f=(float)y;
    h->x2=h->x1; h->x1=x; h->y2=h->y1; h->y1=f;
    return f;
}
#endif
