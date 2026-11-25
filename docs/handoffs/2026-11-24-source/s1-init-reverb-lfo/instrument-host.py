"""Scratch-only observation at the genuine current reverb coefficient."""
from pathlib import Path
import sys
root=Path(sys.argv[1])/'firmware/src'
p=root/'dsp/s1c/s1c.c'; t=p.read_text()
needle='        float rm = P(RVMIX) * P(RVON) * lfo10(s, P(LRVMIX));'
assert t.count(needle)==1
q=p.with_suffix('.scratch')
q.write_text(t.replace(needle,needle+'\n        extern void host_reverb_lfo_tap(const s1c *,float,float);\n        host_reverb_lfo_tap(s,P(RVMIX)*P(RVON),rm);'))
q.replace(p)
p=root/'app/engine.c';q=p.with_suffix('.scratch')
q.write_text(p.read_text()+'\nconst s1c *host_coarse_owner(unsigned o) { return &original_bass[o].dsp.synthone.e; }\n')
q.replace(p)
