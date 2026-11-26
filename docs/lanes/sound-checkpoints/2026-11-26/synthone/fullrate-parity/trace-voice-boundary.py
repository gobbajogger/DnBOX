#!/usr/bin/env python3
"""Add diagnostic per-frame voice-boundary taps to the two exact host fixtures."""
from pathlib import Path
import sys
a,b=map(Path,sys.argv[1:])
b.mkdir(exist_ok=False)
def patch(filename,needle,replace,pre=''):
 p=a/filename;s=p.read_text()
 assert s.count(needle)==1,(p,needle)
 (b/filename).write_text(pre+s.replace(needle,replace))
patch('orig.cpp','    ++ctx.frame;','''    ++ctx.frame;
    const char *path=std::getenv("S1_REFERENCE_VOICETAP");
    static std::FILE *tap=path?std::fopen(path,"wb"):nullptr;
    if(path&&!tap)std::abort();
    if(tap){
        const bool mono=s.parameters[isMono]>0.f;
        const auto &n=mono?*s.monoNote:(*s.noteStates)[s.playingNoteStatesIndex];
        const float v[]={float(ctx.frame-1),s.monoFrequency,s.monoFrequencySmooth,
        float(n.rootNoteNumber),n.internalGate,n.amp,n.filter,s.lfo1_0_1,s.lfo2_0_1,
        ctx.l[i],s.parameters[cutoff],float(n.stage)};
        if(std::fwrite(v,sizeof(float),12,tap)!=12)std::abort();
    }''')
patch('s1c.c','        /* bitcrush (sample-and-hold), tremolo */','''        {
        const char *path=getenv("S1_CANDIDATE_VOICETAP");
        static FILE *tap;
        static unsigned host_frame;
        if(path&&!tap)tap=fopen(path,"wb");
        if(path&&!tap)abort();
        if(tap){
            const s1c_voice *v=&s->v[P(MONO)>0.f?0: s->play_idx];
            const float row[]={ (float)host_frame,s->monof,s->monos,(float)v->root,v->gate,
                         v->amp,v->fenv,s->lfo[0].o01,s->lfo[1].o01,y,P(CUT),(float)v->stage};
            if(fwrite(row,sizeof(float),12,tap)!=12)abort();
        }
        ++host_frame;
        }
        /* bitcrush (sample-and-hold), tremolo */''')
for f in ('render.c','tuning-load.h'):
 (b/f).write_bytes((a/f).read_bytes())
