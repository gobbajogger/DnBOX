
from pathlib import Path
p=Path('firmware/src/app/engine.c')
s=p.read_text()
old="        bass_adapt_post(bv, bb[bv], bb[1-bv], (unsigned)n);"
new=old+"""
        {
            extern void host_lfo_capture(unsigned, const float *, const float *, unsigned);
            host_lfo_capture((unsigned)bv, bb[bv], bb[1-bv], (unsigned)n);
        }
"""
assert s.count(old)==1
p.write_text(s.replace(old,new))
