"""Pinned Mac boundary only; original signed input streams, not complete presets."""
from pathlib import Path
import hashlib,json,ctypes,sys
import numpy as np
dylib,base=sys.argv[1:];base=Path(base)
l=ctypes.CDLL(dylib)
l.route_begin.argtypes=[ctypes.c_uint];l.route_begin.restype=ctypes.c_int
l.route_span.argtypes=[ctypes.c_void_p,ctypes.c_void_p,ctypes.c_uint];l.route_span.restype=None
def digest(a):return hashlib.sha256(a.astype('<f4',copy=False).tobytes()).hexdigest()
def score(y,t):
 e=y.astype('f8')-t
 return dict(gain_db=float(20*np.log10(np.linalg.norm(y.astype('f8'))/np.linalg.norm(t.astype('f8')))),
             residual_dbr=float(20*np.log10(np.linalg.norm(e)/np.linalg.norm(t.astype('f8')))),
             max_abs=float(np.max(abs(e))), output_sha256=digest(y))
def record(p,x):
 d=json.loads((p/'capture.json').read_bytes())
 n=np.load(p/(x+'.npy'),allow_pickle=False)
 assert d['sample_rate']==44100 and n.shape==(2,148684) and n.dtype==np.float32 and np.isfinite(n).all() and digest(n)==d[x+'_pcm_sha256']
 return d,n
result={}
for arm,pins in dict(before=('2c88b98af86f8b870489dec68706b83f1718f9fb318d7da80207d5fed7ca13a9','64e62da06e73704dc7a2d100fcd2b7fe58f27e48078ec4adf4da7ef3c08d69f4'),candidate=('1d91604fda38b2cc8aaf0146375f24152ce1a899fdb54e0b939ca9f3c1d4a1d3','d9dab8327c72cc6c4a70f02467a2c216e101b86702be08171605c4c821f6c924')).items():
 hp=base/(arm+'-hpf');dist=base/(arm+'-crusher')
 h,x=record(hp,'input'); q,wet=record(dist,'output')
 assert h['au_component']=='aufx/hpas/appl' and h['native_before']==h['native_after']=={'0':10,'1':0}
 assert q['au_component']=='aufx/dist/appl' and q['forced_parameter_values']=={'3':.44387006759643555,'4':0,'5':0,'15':0}
 assert digest(x)==pins[0] and digest(wet)==pins[1]
 assert h['output_pcm_sha256']==q['input_pcm_sha256']
 assert q['native_before']==q['native_after']
 actual=wet # Current #107=0 source native Booster is unity
 rows={}
 for enabled in [0,1]:
  assert l.route_begin(enabled)
  y=x.copy();off=0
  for block in q['ordered_block_sizes']:
   l.route_span(y[0,off:].ctypes.data,y[1,off:].ctypes.data,block)
   off+=block
  assert off==x.shape[1]
  rows['guarded' if enabled else 'legacy']=score(y,actual)
 result[arm]=rows
print(json.dumps(dict(scope='same authenticated signed post-Unison/pre-HP input, actual compact runtime HP and Crusher paths, native exact HP→Distortion reference at connected zero and source-proven unity Booster. Other downstream effects explicitly off. Not full instrument parity.',arms=result),indent=2))
