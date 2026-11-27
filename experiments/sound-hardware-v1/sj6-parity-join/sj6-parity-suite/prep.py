from pathlib import Path
import hashlib,io,json,tarfile,sys
import numpy as np
D=Path(__file__).resolve().parent
P=D/'private'
p=P/'warm-brainwave-native.private.tgz'
h=lambda b:hashlib.sha256(b).hexdigest()
assert h(p.read_bytes())=='01b8ef6012068c808e6362e45af735d76d24a2fe8ad2f8b78d24446d1971d175'
installed='1d6edf2bf5dd30906b148e10f50e2470a9f55d13d7f64ab3515e3c2294f1c257'
uid='1E5E5DDF-B5C6-4503-BDBC-CAE9CD1D0A94'
F={}; raw={}; indexes={}
with tarfile.open(p) as f:
 for role in ('authored','verb-off','authored-repeat','verb-off-repeat'):
  front='native-warm-brainwave-'+role+'/'
  c=json.load(f.extractfile(front+'capture.json'))
  x=np.load(io.BytesIO(f.extractfile(front+'output.npy').read()),allow_pickle=False)
  assert c['installed_au_sha256']==installed and c['observed_state']['lastLoadedPreset.uid']==uid
  assert c['parameters']==c['parameter_readback_after'] and len(c['parameters'])==170
  assert c['warm_frames']==16384 and c['frames']==148684 and c['rate']==44100
  assert c['s4_input_sha256']=='17b7b5389af054fd713c31acadbf170955c4146628872c9482ed23ce07f33943'
  assert x.shape==(2,148684) and x.dtype==np.dtype('<f4') and np.isfinite(x).all()
  assert c['pcm_sha256']==h(x.tobytes())
  vals={int(a['address']):float(a['value']) for a in c['parameters']}; assert len(vals)==170
  F[role]=c; raw[role]=x; indexes[role]=vals
base=F['authored']; v=indexes['authored']; L=base['buffers']; E=base['schedule']
assert len(L)==1164 and sum(L)==148684 and 0<min(L)<=max(L)<=128 and len(E)==8
assert {e['frame'] for e in E}<=set(np.r_[0,np.cumsum(L)].tolist())
for role,y in F.items():
 assert y['buffers']==L and y['schedule']==E
 assert {k for k,x in indexes[role].items() if np.float32(v[k]).tobytes()!=np.float32(x).tobytes()}==({11} if 'verb-off' in role else set())
 assert indexes[role][11]==int('verb-off' not in role)
(P/'reference-manifest.json').write_text(json.dumps(dict(rate=44100,frames=148684,installed_au_sha256=installed,preset_uid=uid,cue_sha256=base['s4_input_sha256'],
 controls={r:{str(i):np.float32(y).tobytes().hex() for i,y in vals.items()} for r,vals in indexes.items()},
 schedule=E,buffers=L,pcm_sha256={r:c['pcm_sha256'] for r,c in F.items()}),indent=2))
q=lambda i: int(np.floor(v[i]*127+.5))
assert all(v[i]==0 for i in (12,16,17,22,23,24,63,142))
(P/'cue.h').write_text(
 'enum{FRAMES=148684};\nstatic const unsigned blocks[1164]={'+','.join(map(str,L))+'};\n'
 +'static const struct note{unsigned at,key,vel;} notes[8]={'+','.join('{%d,%d,%d}'%(e['frame'],e['note'],e['velocity']) for e in E)+'};\n'
 +'#define CAND_SIZE %d\n#define CAND_LOCUT %d\n#define CAND_MIX %d\n'%(q(13),q(14),q(15)))
