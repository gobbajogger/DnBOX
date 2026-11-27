from pathlib import Path
import sys
a=Path(sys.argv[1]); o=Path(sys.argv[2]); o.mkdir(exist_ok=True)
def patch(name, transforms):
 p=a/name
 s=p.read_text()
 for before,after in transforms:
  assert s.count(before)==1,(name,before,s.count(before))
  s=s.replace(before,after)
 (o/p.name).write_text(s)
patch('dsp/sj6_native_limiter.inc',[
('''static void sj6_limiter_process(sj6_limiter_t *s,float *l,float *r,unsigned frames)
{
    if(!s->armed)return;
    float gain=s->gain;
    for(unsigned i=0;i<frames;++i){''','''static inline float sj6_limiter_sample(sj6_limiter_t *s,float *l,float *r,float gain)
{'''),
('fm_fabsf(l[i]),fm_fabsf(r[i])','fm_fabsf(*l),fm_fabsf(*r)'),
('''        l[i]*=applied;r[i]*=applied;
    }
    s->gain=gain;
}''','''        *l*=applied;*r*=applied;
        return gain;
}
static void sj6_limiter_process(sj6_limiter_t *s,float *l,float *r,unsigned frames)
{
    if(!s->armed)return;
    float gain=s->gain;
    for(unsigned i=0;i<frames;++i)gain=sj6_limiter_sample(s,l+i,r+i,gain);
    s->gain=gain;
}''')])
patch('dsp/sj6_native_room.inc',[
('''float *left,float *right,unsigned frames)\n{\n    if(!s->armed''','''float *left,float *right,unsigned frames
#if X0X_SJ6_NATIVE_LIMITER
                             ,sj6_limiter_t *dry_limit
#endif
                             )
{
    if(!s->armed'''),
('''        if(s->enable||s->armed==2){
            if(s->enable){s->acc[0]+=l;s->acc[1]+=r;}''','''        if(s->enable||s->armed==2){
            if(s->enable){s->acc[0]+=l;s->acc[1]+=r;}
#if X0X_SJ6_NATIVE_LIMITER
            /* The room feed is from post-delay before the dry compressor.
             * This opt-in is admitted only for the existing additive return. */
            if(dry_limit)dry_limit->gain=sj6_limiter_sample(dry_limit,&l,&r,dry_limit->gain);
#endif''')])
patch('app/original_runtime.c',[
('''            sj6_room_process(&p->dsp.juno.room,left,right,frames);
#endif
#if X0X_SJ6_NATIVE_LIMITER
            sj6_limiter_process(&p->dsp.juno.limiter,left,right,frames);''','''#if X0X_SJ6_NATIVE_LIMITER
            sj6_room_t *room=&p->dsp.juno.room;
            sj6_limiter_t *lim=&p->dsp.juno.limiter;
#if DNBOX_SJ6_DRY_RAGE_PARALLEL
            const int dry_split=lim->armed && room->armed && room->type==1 &&
              (room->enable || room->armed==2) && !room->widen && room->depth==0.f;
#else
            const int dry_split=0;
#endif
            sj6_room_process(room,left,right,frames,dry_split?lim:0);
            if(!dry_split)sj6_limiter_process(lim,left,right,frames);
#else
            sj6_room_process(&p->dsp.juno.room,left,right,frames);
#endif
#elif X0X_SJ6_NATIVE_LIMITER
            sj6_limiter_process(&p->dsp.juno.limiter,left,right,frames);''')])
