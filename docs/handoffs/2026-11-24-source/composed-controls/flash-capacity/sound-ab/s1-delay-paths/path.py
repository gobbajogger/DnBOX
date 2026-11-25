from pathlib import Path
import sys
d=Path(__file__).parent;s=(d/'block.c').read_text()
w=[int(v) for v in sys.argv[1:4]]
mode=int(sys.argv[4]) if len(sys.argv)>4 else 0
a=s.index('    const int short_line=16*')
b=s.index('#else\n/* Carve the delay',a)
s=s[:a]+f'''
    static const unsigned active[3]={{0,1,3}};
    static const unsigned weight[3]={{{','.join(map(str,w))}}};
    const unsigned blocks=s->pool_bytes/9, total={sum(w)};
    uint8_t *pool=s->pool;
    memset(s->pool,0,(size_t)s->pool_bytes);memset(s->dl,0,sizeof s->dl);
    const float time=s->t[DLT]>P(DLT)?s->t[DLT]:P(DLT);
    unsigned remaining=blocks;
    for(unsigned k=0;k<3;k++){{
        s1c_dline *d=&s->dl[active[k]];
        unsigned blk=k==2?remaining:(blocks*weight[k])/total;
        remaining-=blk;
        d->m=(int8_t*)pool;d->len=16*blk;pool+=8*blk;
        d->ex=(int8_t*)pool;pool+=blk;
        float span=time*s->sr*(k<2?2.f:1.f)/(float)(d->len-16);
        d->ai=(int)span+(span>(int)span); if(d->ai<1)d->ai=1;
    }}
    s->dmono=0;s->ddec=s->dl[0].ai;
}}
/* Lab driver deliberately renders fixed authored delay times only. */
static void dly_regrid(s1c *s) {{ (void)s; }}
/* stg: separately addressed float accumulator and previous (memcpy avoids aliasing). */
static float path_tick(s1c_dline *d,float in,float samples,float fb)
{{
   float acc,old;
   memcpy(&acc,d->stg,4);memcpy(&old,d->stg+2,4);
   if(d->ai==1)return DL_TICK(d,in,samples,fb);
   acc+=in;
   if(++d->ap>=d->ai){{
        old=d->prev;
        DL_TICK(d,acc/(float)d->ai,samples/(float)d->ai,fb);
        d->ap=0;acc=0.f;
   }}
   memcpy(d->stg,&acc,4);memcpy(d->stg+2,&old,4);
   return old+(d->prev-old)*(float)d->ap/(float)d->ai;
}}
'''+s[b:]
a=s.index('        int dd = s->ddec;');b=s.index('#if X0X_S1_DELAY_HISTORY_NATIVE\n        mL=',a)
s=s[:a]+f'''
        float span=P(DLT)*s->sr,fb=P(DLFB)*1.25f;
        if(fb>0.98f)fb=0.98f;
        if(s->delay_feedback_direct)fb=P(DLFB);
        float oL=path_tick(&s->dl[0],dL,span*2.f,fb);
        float chain=path_tick(&s->dl[1],dR,span*2.f,fb);
        float oRR=path_tick(&s->dl[3],{ {0:'dR+chain',1:'dR',2:'chain'}[mode] },span,fb);
        float dm=P(DLMIX)*P(DLON);
'''+s[b:]
(d/'firmware/src/dsp/s1c/s1c.c').write_text(s)
