typedef struct {float b0,b1,b2,a1,a2;} nf_coef;
typedef struct {float x1,x2,y1,y2;} nf_hist;
static inline float normal_f32(nf_hist *h,const nf_coef *c,float x)
{
    float y=fmaf(c->b0,x,c->b1*h->x1);
    y=fmaf(c->b2,h->x2,y);
    y=fmaf(-c->a1,h->y1,y);
    y=fmaf(-c->a2,h->y2,y);
    h->x2=h->x1; h->x1=x; h->y2=h->y1; h->y1=y;
    return y;
}
/* Distinct independent transposed candidate; coefficient transitions not
   equivalent without recomputing state from DF-I history. */
typedef struct {float s1,s2;} nf_trans;
static inline float normal_trans(nf_trans *h,const nf_coef *c,float x)
{
    float y=fmaf(c->b0,x,h->s1);
    h->s1=fmaf(c->b1,x,fmaf(-c->a1,y,h->s2));
    h->s2=fmaf(c->b2,x,-c->a2*y);
    return y;
}
/* b2=b0 and b1=2*b0 in the authoritative Normal fixture. */
static inline float normal_plain(nf_hist *h,const nf_coef *c,float x)
{
    const float fir=(x+h->x2)+(h->x1+h->x1);
    const float y=(c->b0*fir-c->a1*h->y1)-c->a2*h->y2;
    h->x2=h->x1; h->x1=x; h->y2=h->y1; h->y1=y;
    return y;
}
/* Precompute k from original doubles before narrowing. Same DF-I histories. */
static inline float normal_delta(nf_hist *h,const nf_coef *c,float x)
{
    const float fir=(x+h->x2)+(h->x1+h->x1);
    const float dy=(c->b0*fir-c->a1*h->y1)+c->a2*(h->y1-h->y2);
    const float y=h->y1+dy;
    h->x2=h->x1; h->x1=x; h->y2=h->y1; h->y1=y;
    return y;
}
