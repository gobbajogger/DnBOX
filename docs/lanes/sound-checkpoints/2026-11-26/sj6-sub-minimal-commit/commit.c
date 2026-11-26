/* Exact stage-1 crossing kernel, no dependencies and no voice/preset data.
 * Caller supplies authentic current binary64 AMP anchor, constructor rate
 * tick count, and emits exactly the receiver prep boundaries. */
typedef struct {
    double y, step;
    unsigned char pending_key, armed;
} sj6_stage1;

int sj6_stage1_offer(sj6_stage1 *s,double anchor,unsigned ticks,unsigned note)
{
    if(!(anchor>0.)||anchor>1.0||!ticks||ticks>1024||note>127) return 0;
    const double step=-anchor/(double)ticks;
    if(!(step<0.))return 0;
    s->y=anchor;
    s->step=step;
    s->pending_key=(unsigned char)note;
    s->armed=1;
    return 1;
}

/* Returns new ownership key on the one boundary or -1 before it. */
int sj6_stage1_prep(sj6_stage1 *s)
{
    if(!s->armed)return -1;
    s->y+=s->step;
    if(s->y>0.)return -1;
    s->armed=0;
    return s->pending_key;
}

void sj6_stage1_cancel(sj6_stage1 *s){s->armed=0;}
