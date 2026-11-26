from pathlib import Path
import ctypes as c, hashlib, io, json, math, re, sys, tarfile, zipfile,struct
import numpy as np
o,b=map(Path,sys.argv[1:])
# Reuse authorized matching sample capsule decoder, no compiled app import.
s=Path('recovered-sound/sj6-sub-handoff/probe.py').read_text().split('ipa=')[1].split('nat=Path(')[0]
exec(compile('ipa='+s,'existing private SFZ container decoder','exec'))
sha=lambda z:hashlib.sha256(z).hexdigest()
assert sha(data)=='ec994e3fabf42b2761d7ca3799f87dacb4ac12670e2909451ff14f78b9a5f740'
assert sha(sfz)=='3668ef83c102b4952a19bc6fd5adc585eb69d0eff4dd4743be9488b0771fc75d'
arc=b/'SJ6-Three-Factory-Presets-original-AUv3.tgz'
assert sha(arc.read_bytes())=='0aaf6375083dfb2bb1224fe690340ed2eff4ad978f878d099e2c19913af60603'
with tarfile.open(arc) as t:
 def take(a):
  p=f'./program-2-{a}/'
  m=json.load(t.extractfile(p+'capture.json'))
  r=np.load(io.BytesIO(t.extractfile(p+'output.npy').read()),allow_pickle=False).copy()
  assert sha(r.tobytes())==m['pcm_sha256'] and m['active']==m['after'] and m['frames']==r.shape[1]==148684
  return r,m
 refs=dict((a,take(a)) for a in ('authored','dry','authored-repeat'))
assert np.array_equal(refs['authored'][0],refs['authored-repeat'][0])
from dose import collect
from bridge import Bridge
m=refs['dry'][1];raw={int(v['address']):v['value'] for v in m['active']}
a,pin=collect(m,raw,regions,pack,sub_only=True)
adapter=Bridge(o)
b,other=collect(m,raw,regions,pack,adapter.install,sub_only=True)
assert pin==other
prior=json.load(open('sj6-warm-raw-postvoice/result-final.json'))
x,c1,k1=a['Sub'];y,c2,k2=b['Sub']
assert c1==c2 and k1==k2 and np.array_equal(x.view('u4'),y.view('u4'))
assert adapter.calls==123515 and adapter.commits==4 and adapter.wraps==0
assert sha(x.tobytes())==prior['source_roles']['Sub']['output_sha']
print(json.dumps(dict(pin=pin, original_current_sub_sha=sha(x.tobytes()), external_read_pair_sub_sha=sha(y.tobytes()),
 logical_replacement={'active_helper_calls':adapter.calls,'actual_commits':adapter.commits,'gate_edges':adapter.gates,'loops':adapter.wraps, 'first_commits':adapter.entries}, exact_signed_all_frames=True,
 real_source_array_copied_into_host_adapter=False, unconditional_fallback=False),indent=2))
