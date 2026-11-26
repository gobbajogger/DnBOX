from pathlib import Path
import sys,hashlib
leaf,c,out=map(Path,sys.argv[1:])
p=out/'juno6_j60_native.inc'
s=leaf.read_text()
def replace(a,b):
 global s
 assert s.count(a)==1,a
 s=s.replace(a,b)
replace('#if X0X_SJ6_NATIVE_J60','''#if X0X_SJ6_NATIVE_J60
#include <stdio.h>
static unsigned n_core, n_bound, n_post[2], n_emitted, n_mix;''')
replace('    const float ramp=','    ++n_core;\n    const float ramp=')
replace('        s->wet[ch]=s->post1[ch];','        s->wet[ch]=s->post1[ch];\n        ++n_post[ch];')
replace('    if(m<JUNO6_CHORUS_I||m>JUNO6_CHORUS_BOTH){s->reset=1;return;}','''    n_core=n_bound=n_emitted=n_mix=n_post[0]=n_post[1]=0;
    if(m<JUNO6_CHORUS_I||m>JUNO6_CHORUS_BOTH){
        s->reset=1; /* bit-transparent: returns caller's buffers, no internal step */
        fprintf(stderr,"J60CLOCK %u %.1f %u %u 0 0 0 0 0 0\\n",m,c->sample_rate,frames,frames);
        return;
    }
    const float start_bridge=s->reset?0.f:s->bridge;''')
replace('            j60_core(c,s,m-JUNO6_CHORUS_OFF,lerp);','            ++n_bound;\n            j60_core(c,s,m-JUNO6_CHORUS_OFF,lerp);')
replace('        if(s->mix!=0.f){','        ++n_emitted;\n        if(s->mix!=0.f){\n            ++n_mix;')
replace('    }\n}\n#endif','''    }
    fprintf(stderr,"J60CLOCK %u %.1f %u %u %u %u %u %u %a %a\\n",
        m,c->sample_rate,frames,n_emitted,n_mix,n_bound,n_core,n_post[0],(double)start_bridge,(double)s->bridge);
    if(n_post[0]!=n_post[1])abort();
}
#endif''')
replace('#include <stdio.h>','#include <stdio.h>\n#include <stdlib.h>')
p.write_text(s)
(out/'juno6.c').write_bytes(c.read_bytes())
