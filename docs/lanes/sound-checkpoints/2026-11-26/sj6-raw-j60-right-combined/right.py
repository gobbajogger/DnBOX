from pathlib import Path
import sys
p=Path(sys.argv[1]); s=p.read_text()
a="displacement*.967567563f*fm_fabsf(ph)"
b="displacement*((mode==1u && ch==1u)?1.0f:.967567563f)*fm_fabsf(ph)"
assert s.count(a)==1
p.write_text(s.replace(a,b))
