"""HP-only insertion in existing owner before compact Crusher/Room.
Do not claim the rest of the compact FX graph has reproduced source order."""
from pathlib import Path
import sys
s=Path(sys.argv[1]).read_text()
mark='            sj6_crush_process(p,left,right,frames);'
assert s.count(mark)==1
s=s.replace(mark,'''            extern void original_hp_floor(float *,float *,unsigned);
            original_hp_floor(left,right,frames);
'''+mark)
Path(sys.argv[2]).write_text(s)
