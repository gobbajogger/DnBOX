"""Read-only semantic observation inserted only in the disposable host copy."""
import sys
from pathlib import Path
root=Path(sys.argv[1])/'firmware/src'
def modify(p,a,b):
    t=p.read_text()
    assert t.count(a)==1
    q=p.with_suffix('.scratch')
    q.write_text(t.replace(a,b))
    q.replace(p)
modify(root/'dsp/s1c/s1c.c','        y *= lfo10(s, P(LTREM));','''        extern void host_tremolo_tap(const s1c *,float,float);
        float before_tremolo=y;
        y *= lfo10(s, P(LTREM));
        host_tremolo_tap(s,before_tremolo,y);''')
p=root/'app/engine.c'
q=p.with_suffix('.scratch')
q.write_text(p.read_text()+'\nconst s1c *host_coarse_owner(unsigned o) { return &original_bass[o].dsp.synthone.e; }\n')
q.replace(p)
