"""Exact reached mono sample helper with nonproprietary independent media."""
from pathlib import Path
import ctypes as ct,hashlib,struct,numpy as np,sys,json
e={}
p=Path('sj6-modern-owner/probe.py')
exec(compile(p.read_text().split('def trial(')[0],str(p),'exec'),e)
eng=e['Emu'](str(e['BIN'])); sha=lambda a:hashlib.sha256(a).hexdigest()
lib=ct.CDLL(sys.argv[1])
class Z(ct.Structure):
 _fields_=[('planar',ct.c_void_p),('length',ct.c_uint),('channels',ct.c_uint),('end',ct.c_float),('loop_start',ct.c_float),('loop_stop',ct.c_float),('loop',ct.c_uint)]
lib.cursor_ref.argtypes=[ct.POINTER(Z),ct.POINTER(ct.c_double),ct.c_double,ct.c_double,ct.c_float,ct.c_uint,ct.POINTER(ct.c_float)]
out=(ct.c_float*2)(); voice,zone,l,r=[eng.calloc(n) for n in (32,48,4,4)];results=[]
assert not lib.cursor_ref(ct.byref(Z()),ct.byref(ct.c_double()),1.,1.,1.,1,out)
assets=[np.linspace(-.75,.75,4096,dtype='f4'),np.linspace(.75,-.75,4096,dtype='f4')]
for cue,(scale,ratio,start,gate,repeat) in enumerate(((.5,.75,0,1,1024),(1.,1.001,128,1,1024),(.917,1.0297,2047.5125,0,1024),(.75,.988125,3071.75,1,1024))):
 run=[]
 for asset in assets:
  dur=len(asset);stop=3072;ls=1117;mem=eng.calloc(4*dur);eng.wr(mem,asset.tobytes())
  z=Z(asset.ctypes.data,dur,1,4095,ls,stop,1)
  eng.wr(zone,struct.pack('<QIII4xfB3xff',mem,0,1,dur,z.end,1,ls,stop))
  pos=ct.c_double(start)
  eng.wr(voice,struct.pack('<B7xddd',gate,start,scale,ratio))
  q=[];pp=[]
  for i in range(repeat):
    eng.call(0x1003b8224,voice,zone,0,l,r,floats=(.6875,))
    ret=lib.cursor_ref(ct.byref(z),ct.byref(pos),scale,ratio,.6875,gate,out)
    assert ret and eng.x(0)==0
    assert struct.pack('<d',pos.value)==eng.rd(voice+8,8)
    assert struct.pack('<ff',*out)==eng.rd(l,4)+eng.rd(r,4)
    q.append(struct.pack('<ff',*out));pp.append(struct.pack('<d',pos.value))
  run.append(dict(media_sha=sha(asset.tobytes()),cursor_sha=sha(b''.join(pp)),out_sha=sha(b''.join(q)),reached=repeat))
 assert run[0]['cursor_sha']==run[1]['cursor_sha'] and run[0]['out_sha']!=run[1]['out_sha']
 results.append(dict(cue=cue,start=start,gate=gate,rate_scale=scale,ratio=ratio,arms=run))
assert not eng.unknown
print(json.dumps(dict(verified_current=e['pin'](e['BIN']), execution='matching exact in-guest published local function .3b8224 on synthetic signed mono words, with same-cursor loop/open/negative sample histories', no_media_returns_closed=True, cases=results),indent=2))
