"""Meter-only diagnostic. No transforms of the sampled streams."""
import sys
from pathlib import Path
a=Path(sys.argv[1]).read_text()
n=a.index('static void sj6_run_effects(')
p=a.index('\n{\n',n)+3
q='            /* The original master attenuator follows the instrument effects. */'
assert a.count(q)==1
a=a[:p]+'    extern void witness(unsigned,const float*,const float*,unsigned); witness(0,left,right,frames);\n'+a[p:]
a=a.replace(q,'            witness(1,left,right,frames);\n'+q)
Path(sys.argv[2]).write_text(a)
