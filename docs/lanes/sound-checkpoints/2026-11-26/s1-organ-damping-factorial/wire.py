from pathlib import Path
import sys
t=Path(sys.argv[1])
def edit(p,a,b):
 s=p.read_text();assert s.count(a)==1,(p,a);p.write_text(s.replace(a,b,1))
h=t/'source_fx.hpp'
edit(h,'#undef nil','''#undef nil''') # expected source header
s=h.read_text();h.write_text('''#include <cstdio>
#include <cstdlib>
static inline void room_tap(const float *a){
  static std::FILE *f=std::fopen(std::getenv("ROOM_TRACE"),"wb");
  if(!f||std::fwrite(a,sizeof(float),10,f)!=10)std::abort();
}
'''+s)
a='        reverbCostello->feedback = parameters[reverbFeedback];'
edit(h,a,'        '+a.lstrip())
a='        revCrossfadeL->pos = reverbMixFactor;'
edit(h,a,'''        const float room_trace[]={mixedDelayL,mixedDelayR,butCompressOutL,butCompressOutR,reverbWetL,reverbWetR,wetReverbLimiterL,wetReverbLimiterR,parameters[reverbFeedback],reverbMixFactor};
        room_tap(room_trace);
'''+a)
p=t/'dsp/s1c/s1c.c'
s=p.read_text();p.write_text('''#include <stdio.h>
#include <stdlib.h>
static void room_feed(float*l,float*r){
 static int init;static FILE*f;
 if(!init){init=1;const char*x=getenv("ROOM_FEED");if(x)f=fopen(x,"rb");}
 if(f){float row[10];if(fread(row,sizeof(float),10,f)!=10)abort();*l=row[2];*r=row[3];}
}
static void room_tap(const float*a){
 static FILE*f;if(!f)f=fopen(getenv("ROOM_TRACE"),"wb");
 if(!f||fwrite(a,sizeof(float),10,f)!=10)abort();
}
'''+s)
a='        float wl, wr;'
edit(p,a,'''        room_feed(&hl,&hr);
        float initialL=hl,initialR=hr;
'''+a)
a='        wl = comp(s->cmp[2][0], wl, &cw) * P(CWG); wr = comp(s->cmp[2][1], wr, &cw) * P(CWG);'
edit(p,a,'''        const float roomL=wl,roomR=wr;
'''+a)
a='        float rm = P(RVMIX) * P(RVON) * lfo10(s, P(LRVMIX));'
edit(p,a,a+'''
        const float room_trace[]={mL,mR,initialL,initialR,roomL,roomR,wl,wr,P(RVFB),rm};
        room_tap(room_trace);
''')
