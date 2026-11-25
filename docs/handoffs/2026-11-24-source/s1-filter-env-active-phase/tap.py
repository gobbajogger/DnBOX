from pathlib import Path
p=Path('firmware/src/dsp/s1c/s1c.c');s=p.read_text()
x='        const float hz=clampf(base-base*amount*(1.f-v->fenv),64.f,22050.f);'
assert s.count(x)==1
s=s.replace(x,x+'''
        extern void host_env_tap(const s1c*,const s1c_voice*,unsigned,float,float,float,float,float);
        host_env_tap(s,v,(unsigned)ft,base,P(FENV),s->lfo[0].o10,s->lfo[1].o10,hz);
''')
p.write_text(s)
