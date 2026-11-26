from pathlib import Path
import sys
p,q=map(Path,sys.argv[1:])
s=p.read_text().replace('#include <stdio.h>','#include <stdio.h>\n#include <stdlib.h>')
a='  prepare(bpm);'
assert s.count(a)==1
s=s.replace(a,'''  if(getenv("SJ6_NO_FIXED_FIXTURE"))word[205]=0.f;
'''+a)
a='(v&&bid(205))?87u:v'
assert s.count(a)==1
s=s.replace(a,'''(v&&bid(205))?
#if SJ6_MATCH_FIXED_TAPER
        ((word[86]==0.f && key>=12 && key<72)?(unsigned)((double)(87.f/127.f)*127.):87u):
#else
        87u:
#endif
        v''')
q.write_text(s)
