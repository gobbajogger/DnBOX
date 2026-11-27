"""Unshifted, unnormalized signed stereo Init comparison."""
from pathlib import Path
import argparse,hashlib,json,re
import numpy as np
d=Path(__file__).resolve().parent
p=argparse.ArgumentParser()
p.add_argument('compact',type=Path)
p.add_argument('--log',type=Path,default=d/'compact.events')
p.add_argument('--native',type=Path)
p.add_argument('--repeat',type=Path)
args=p.parse_args()
assert bool(args.native)==bool(args.repeat)
cue=(d/'cue.json').read_bytes()
cfg=json.loads(cue)
h=lambda b:hashlib.sha256(b).hexdigest()
expected=[(e['frame'],e['note'],e['velocity'],int(e['velocity']!=0)) for e in cfg['schedule']]
assert expected==[tuple(map(int,x)) for x in re.findall(r'^event (\d+) (\d+) (\d+) (\d+)$',args.log.read_text(),re.M)]
N,rate=cfg['frames'],cfg['rate']
assert f'pass unmodified compact Init, frames={N} events={len(expected)}' in args.log.read_text()
def load(b):
 assert len(b)==N*8
 x=np.frombuffer(b,dtype='<f4').reshape(2,N).astype('f8')
 assert np.isfinite(x).all() and abs(x).max()>1.e-5
 return x
def native(folder):
 r=json.loads((folder/'capture.json').read_bytes())
 b=(folder/'audio.f32').read_bytes()
 assert r['uid']=='8733EFD7-4000-4C31-BC52-5AD9E4B707E1' and r['name']=='Your Init Preset'
 assert r['component_version']==67072 and len(r['raw_controls'])>=160
 assert (r['frames'],r['rate'],r['events'],r['blocks'])==(N,rate,len(expected),len(cfg['buffers']))
 assert r['cue_sha256']==h(cue) and r['signed_pcm_sha256']==h(b)
 return load(b),r
def rms(x):return np.sqrt(np.mean(x*x,axis=1))
def db(x,y):return (20*np.log10(np.maximum(x,1.e-70)/np.maximum(y,1.e-70))).tolist()
slices=dict(held=slice(16384,38434),chord=slice(49459,82534),release=slice(82534,N),whole=slice(16384,N))
b=args.compact.read_bytes(); got=load(b)
assert not got[:,:16384].any()
out=dict(cue_sha256=h(cue),compact_sha256=h(b),frames=N,blocks=len(cfg['buffers']),events=len(expected),
         compact_peak=abs(got).max(axis=1).tolist(),compact_rms_dbfs={k:db(rms(got[:,v]),1.) for k,v in slices.items()},
         original_init_render_admitted=bool(args.native))
if args.native:
 want,wr=native(args.native); repeat,rr=native(args.repeat)
 assert wr['raw_controls']==rr['raw_controls'] and wr['identifiers']==rr['identifiers']
 out['original_pcm_sha256']=[wr['signed_pcm_sha256'],rr['signed_pcm_sha256']]
 out['comparison']={}
 for k,i in slices.items():
  a,c,t=want[:,i],got[:,i],repeat[:,i]
  out['comparison'][k]=dict(level_delta_db=db(rms(c),rms(a)),signed_null_dbr=db(rms(c-a),rms(a)),original_repeat_dbr=db(rms(t-a),rms(a)))
print(json.dumps(out,indent=2,allow_nan=False))
