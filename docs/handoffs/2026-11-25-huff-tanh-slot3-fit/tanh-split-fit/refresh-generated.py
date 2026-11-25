"""Regenerate just the split witness table in a copied guarded bench tree."""
from pathlib import Path
import sys,re
root=Path('tree')
sys.path.insert(0,str((root/'tools').resolve()))
import gen_drum_samples as g
for dir in ('smk37-elite','smk37'):
    h=root/'build'/dir/'gen/x0x_drum_tables.h'
    t=h.read_text()
    match=re.search(r'\bx0x_tanh_raw\[1026\]\s*=\s*\{([^}]+)\}',t)
    vals=[float(x.strip().rstrip('f')) for x in match[1].split(',') if x.strip()]
    compiled=[g.f32(g.math.tanh(g.TANH_MAX*i/g.TANH_N)) for i in range(g.TANH_N+2)]
    assert vals==compiled
    out=g.tanh_split_decl(vals)
    if '#define X0X_TANH_LAB_SPLIT 400' not in t:
        h.write_text(t+'\n#if X0X_TANH_SPLIT_LAB\n'+out+'\n#endif\n')
    print(dir,"1026/1026 exact; head 1600 B, tail 1252 B")
