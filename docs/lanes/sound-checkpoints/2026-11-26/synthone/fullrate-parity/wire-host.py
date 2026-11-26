#!/usr/bin/env python3
"""Inject original-source room downstream of actual DnBOX voice+delay, no recorded feedback input."""
from pathlib import Path
import sys
base=Path(sys.argv[1])
def exact(s,a,b):
    assert s.count(a)==1,a
    return s.replace(a,b)
p=base/'dsp/s1c/s1c.h';s=p.read_text()
s=exact(s,'    int32_t wpos, wlen;','    int32_t wpos, wlen;\n    unsigned host_width_pos;\n    float host_width[2205];\n#if S1_ROOM_EXACT\n    void *host_exact_room;\n#endif\n#if S1_DELAY_EXACT\n    struct dnbox_s1_exact_delay *host_exact_delay;\n    float host_delay_mono;\n#endif')
p.write_text(s)
p=base/'dsp/s1c/s1c.c';s=p.read_text()
s=exact(s,'static void room_tap(const float*a){','''#include "exact-room.h"
#include "exact-delay.h"
static void room_tap(const float*a){''')
s=exact(s,' static FILE*f;if(!f)f=fopen(getenv("ROOM_TRACE"),"wb");',' static FILE*f; const char *trace=getenv("ROOM_TRACE"); if(!trace)return; if(!f)f=fopen(trace,"wb");')
a='        /* ping-pong delay with tracking low-pass on its input */\n'
s=exact(s,a,a+"""#if S1_DELAY_EXACT
        if(!s->host_exact_delay){
            s->host_exact_delay=dnbox_s1_delay_new((unsigned)s->sr);
            if(!s->host_exact_delay)abort();
        }
        const float mono_now=P(MONO);
        if(mono_now!=s->host_delay_mono){
            dnbox_s1_delay_reset(s->host_exact_delay);
            s->host_delay_mono=mono_now;
        }
        float mL,mR;
        dnbox_s1_delay_insert(s->host_exact_delay,P(DLT),P(DLFB),P(FTYPE),P(CUT),P(DICT),
                              P(DLMIX),P(DLON),L,R,&mL,&mR);
#else
""")
a='        /* reverb: high-pass, pre-gain, input compressor, network, wet compressor */\n'
s=exact(s,a,'#endif /* S1_DELAY_EXACT */\n'+a+'''#if S1_ROOM_EXACT
        if(!s->host_exact_room)s->host_exact_room=exact_room_new(s->sr);
        float hl=mL,hr=mR,wl=0.f,wr=0.f;
        exact_room_hp(s->host_exact_room,P(RVHP),&hl,&hr);
        exact_room_comp(s->host_exact_room,ROOM_INPUT_COMP,P(CRR),P(CRT),P(CRA),P(CRREL),P(CRG),&hl,&hr);
        const float initialL=hl,initialR=hr;
        exact_room_return(s->host_exact_room,P(RVFB),hl,hr,&wl,&wr);
        const float roomL=wl,roomR=wr;
        exact_room_comp(s->host_exact_room,ROOM_WET_COMP,P(CWR),P(CWT),P(CWA),P(CWREL),P(CWG),&wl,&wr);
#else
''')
a='        float rm = P(RVMIX) * P(RVON) * lfo10(s, P(LRVMIX));'
s=exact(s,a,'#endif /* S1_ROOM_EXACT */\n'+a)
a='        L = ((1.0f - rm) * mL + rm * wl) * 2.0f * P(MVOL);\n        R = ((1.0f - rm) * mR + rm * wr) * 2.0f * P(MVOL);'
s=exact(s,a,'''#if S1_ROOM_EXACT
        L=(mL*(1.f-rm)+wl*rm)*(2.f*P(MVOL));
        R=(mR*(1.f-rm)+wr*rm)*(2.f*P(MVOL));
#else
'''+a+'\n#endif')
a='        L = comp(s->cmp[0][0], L, &cm) * P(CMG); R = comp(s->cmp[0][1], R, &cm) * P(CMG);'
s=exact(s,a,'''#if S1_ROOM_EXACT && S1_ROOM_EXACT_MASTER
        exact_room_comp(s->host_exact_room,ROOM_MASTER_COMP,P(CMR),P(CMT),P(CMA),P(CMREL),P(CMG),&L,&R);
#else
'''+a+'\n#endif')
start=s.index('        const float wsample[]={L,R,P(WIDEN)};')
end=s.index('        outl[f] = L; outr[f] = R;',start)
s=s[:start]+"""        if(s->sr!=44100.f)abort();
        const unsigned pos=s->host_width_pos;
        const float delayed=s->host_width[pos];
        s->host_width[pos]=R;
        s->host_width_pos=pos+1u==2205u?0u:pos+1u;
        const float width=P(WIDEN);
        R=width*delayed+(1.f-width)*R;
"""+s[end:]
p.write_text(s)
p=base/'part.c';s=p.read_text()
s='#include "exact-room.h"\n#include "exact-delay.h"\n'+s
a='    load_init(e);'
# first unconditional program load, not the decode error branch (which matches a contextual line)
s=exact(s,a+'\n','''#if S1_ROOM_EXACT
    exact_room_drop(e->host_exact_room);
#endif
#if S1_DELAY_EXACT
    dnbox_s1_delay_free(e->host_exact_delay);
#endif
'''+a+'\n')
p.write_text(s)
p=base/'render.c';s=p.read_text()
s='#include "exact-room.h"\n#include "exact-delay.h"\n'+s
a='    free(pack);'
s=exact(s,a,'''#if S1_ROOM_EXACT
    exact_room_drop(part.e.host_exact_room);
#endif
#if S1_DELAY_EXACT
    dnbox_s1_delay_free(part.e.host_exact_delay);
#endif
'''+a)
p.write_text(s)
