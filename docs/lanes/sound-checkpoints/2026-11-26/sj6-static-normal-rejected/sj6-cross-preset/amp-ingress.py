"""One source-backed compact operation ordering variant, all controls unchanged.
Not independent Sub/Noise ownership. Unknown/mixed controls refuse substitution.
"""
from pathlib import Path
import hashlib, sys
p,q=map(Path,sys.argv[1:])
b=p.read_bytes()
assert hashlib.sha256(b).hexdigest()=='d58f87a1f598fb894a184ccd57f543a1ec451ee13aea020b6eec9e92ca0da625'
s=b.decode()
def rw(a,b):
 global s
 assert s.count(a)==1,a
 s=s.replace(a,b)
rw('''                /* Existing DnBOX two-pole recurrence''','''                /* Diagnostic authenticated Main ordering: AMP×velocity precedes
                 * the per-layer existing state. Preserve non-SJ6 owners.
                 * Sub/noise still share the compact ingress if enabled. */
                const int amp_before=s->superj6_amp_range &&
                    p->oscillator_mode==DNBX_OSC_SUPERJ6;
                const float output_amp=velocity*amp;
                if(amp_before)x=output_amp*x;
                /* Existing DnBOX two-pole recurrence''')
rw('mix[l]+=velocity*amp*sj6_ota_filter(lv,x,a,p->resonance);','mix[l]+=(amp_before?1.f:output_amp)*sj6_ota_filter(lv,x,a,p->resonance);')
rw('mix[l]+=velocity*amp*lv->filter_second;','mix[l]+=(amp_before?1.f:output_amp)*lv->filter_second;')
q.write_text(s)
