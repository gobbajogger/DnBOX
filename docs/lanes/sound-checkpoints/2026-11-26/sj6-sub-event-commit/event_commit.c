/* A due prep ordinal is computed from exact source rounding at NOTE time.
 * There is deliberately no binary64 arithmetic in prep/render. */
typedef struct {
    double start;
    unsigned short ticks, initial_due, due;
    unsigned char pending;
} sj6_event_clock;

static double exact_step(double start,unsigned ticks){return -start/(double)ticks;}

int sj6_event_offer(sj6_event_clock *s,double start,unsigned ticks,unsigned note)
{
    if(!(start>0.)||start>1.||!ticks||ticks>128||note>127)return 0;
    const double step=exact_step(start,ticks);
    if(!(step<0.))return 0;
    double y=start;
    unsigned n=0;
    /* Simulate original rounding but only when arming/retargeting. */
    do{y+=step;++n;}while(y>0.&&n<ticks+1u);
    if(y>0.)return 0; /* fail closed beyond tested bounded domain */
    s->start=start;
    s->ticks=(unsigned short)ticks;s->initial_due=s->due=(unsigned short)n;s->pending=(unsigned char)note;
    return 1;
}

int sj6_event_retarget(sj6_event_clock *s,unsigned note)
{
    if(!s->due)return 0;
    double y=s->start;
    const double delta=exact_step(y,s->ticks);
    for(unsigned i=0;i<(unsigned)(s->initial_due-s->due);++i)y+=delta;
    return sj6_event_offer(s,y,s->ticks,note);
}

/* Call exactly once per admitted actual DSP preparation boundary. */
int sj6_event_prep(sj6_event_clock *s)
{
    if(!s->due)return -1;
    return --s->due ? -1 : s->pending;
}
void sj6_event_cancel(sj6_event_clock *s){s->due=0;}
