/* Narrow fixed-source AHDSHR shadow. Source-authenticated control routing and
 * boundary count are caller-owned. No sample production, override or fallback. */
typedef struct {double y, mul, add; unsigned char stage;} sj6_shadow;
enum { NONE, ATTACK, HOLD0, DECAY, RELEASE };
extern int sj6_fused(double,double,double,double *);
void shadow_entry(sj6_shadow *s,int stage,double y,double mul,double add)
{
    if(stage<ATTACK || stage>RELEASE || !(y>=0.&&y<=1.)) { s->stage=NONE; return; }
    *s=(sj6_shadow){y,mul,add,(unsigned char)stage};
}
/* Exact one control tick, not one audio sample. Signals Attack→Hold and
 * Hold→Decay gateways; the caller supplies immutable source coefficients. */
int shadow_tick(sj6_shadow *s,double sustain)
{
    if(s->stage==NONE)return NONE;
    if(s->stage==HOLD0)return DECAY;
    if(!sj6_fused(s->mul,s->y,s->add,&s->y))return -1;
    if(s->stage==ATTACK && s->y>=1.){s->y=1.;return s->stage=HOLD0;}
    if(s->stage==DECAY && s->y<=sustain){s->y=sustain;s->stage=NONE;return NONE;}
    if(s->stage==RELEASE && s->y<=0.){s->y=0.;s->stage=NONE;return NONE;}
    return s->stage;
}
unsigned shadow_size(void){return sizeof(sj6_shadow);}
