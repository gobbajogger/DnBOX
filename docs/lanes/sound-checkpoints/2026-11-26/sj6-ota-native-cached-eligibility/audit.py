from pathlib import Path
import json,tarfile,hashlib
d=Path('sj6-direct-stage')
sha=lambda data:hashlib.sha256(data).hexdigest()
signed={}
for p in sorted(d.glob('SJ6-*.tgz')):
 r=[]
 with tarfile.open(p,'r|gz') as t:
  for m in t:
   if not m.name.endswith('/capture.json') or '/._' in m.name:continue
   b=t.extractfile(m).read();a=json.loads(b); words=a.get('parameters') or a.get('active');after=a.get('parameter_readback_after') or a.get('after')
   if not words:continue
   q={int(v['address']):v['value'] for v in words}
   if not (q.get(1,0)>0 and all(q.get(k)==0 for k in (3,201,202))):continue
   assert after==words, (p,m.name)
   assert a['installed_au_sha256']=='1d6edf2bf5dd30906b148e10f50e2470a9f55d13d7f64ab3515e3c2294f1c257'
   assert len(words)==170 and len(a['schedule'])==8 and a['rate']==44100
   r.append(dict(arm=m.name,capture_sha=sha(b),terminal_pcm_sha=a['pcm_sha256'],params_endpoint_exact=True,frames=a['frames'],warm_frames=a['warm_frames'],endpoint_subset={str(k):q[k] for k in (1,3,7,8,129,130,131,136,137,179,181,183,185,187,189,191,201,202,209)},admitted_to_ota=False,first_failure='Public filter-model selector 209 is Normal, not OTA'))
 signed[p.name]=r
b=signed['SJ6-Brainwave-Direct-AUv3-Fanin.tgz']; assert len(b)==2 and all(a['endpoint_subset']['209']==0 for a in b)
assert sum(map(len,signed.values()))==2 and len(signed)==6
assert sha((d/'SJ6-Brainwave-Direct-AUv3-Fanin.tgz').read_bytes())=='d3821515187f4c086cde7ecbff57bafbae216fbd86becd1ff356fb1fe4a5df9f'
stock=Path('/tmp/phaser-warm-own.Y0XNbQ/tree/firmware/src/dsp/dnbx_core.c')
typed=Path('sj6-cross-preset/render.c')
assert 'juno6_sj6_set_ota(s,q(209,1))' in typed.read_text()
s=stock.read_text(); assert 'if(s->sj6_ota)mix' in s and 'float base=authored_track?p->cutoff_hz*keyboard_ratio[i]:p->cutoff_hz' in s
print(json.dumps(dict(custody='Six already captured direct-stage archives; only positive sole Main1 and externally zero other sources listed',projection_source_sha=sha(typed.read_bytes()),dsp_source_sha=sha(stock.read_bytes()),results=signed),indent=2))
