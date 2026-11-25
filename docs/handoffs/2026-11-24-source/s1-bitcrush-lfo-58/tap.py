from pathlib import Path
p=Path('firmware/src/dsp/s1c/s1c.c');s=p.read_text()
a='        /* bitcrush (sample-and-hold), tremolo */\n'
s=s.replace(a,a+'''        extern void host_crush_tap(const s1c*,int,int,float,float,float,float);
        unsigned tap_pre=1;
        host_crush_tap(s,tap_pre,f,y,P(BCSR),s->lfo[0].o01,s->lfo[1].o01);
''')
a='        y *= lfo10(s, P(LTREM));'
s=s.replace(a,'''        host_crush_tap(s,0,f,y,P(BCSR),s->lfo[0].o01,s->lfo[1].o01);
'''+a)
p.write_text(s)
