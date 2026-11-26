"""Independent signed synthetic pool views through reached current mono/stereo helper."""
from pathlib import Path
import ctypes as ct,hashlib,struct,numpy as np,sys,json
from bridge import Bridge,Media,Pcm,Owner
e={}
src=Path('sj6-modern-owner/probe.py')
exec(compile(src.read_text().split('def trial(')[0],str(src),'exec'),e)
g=e['Emu'](str(e['BIN'])); sha=lambda a:hashlib.sha256(a).hexdigest()
br=Bridge(sys.argv[1]); lib=br.lib
out=(ct.c_float*2)();voice,zone,l,r=[g.calloc(n) for n in (32,48,4,4)]
rng=np.random.default_rng(244)
rows=[]
for n in (43,257):
 for ch in (1,2):
  for hold in (0,1):
   med=rng.integers(-32768,32767,size=(ch,n),dtype=np.int16)
   w=(med.astype('f4')*np.float32(2**-15)).copy(); p=g.calloc(w.nbytes);g.wr(p,w.tobytes())
   start,end,first,last=.75,float(n-1),5.25,float(n//3)
   q=Pcm(med.ctypes.data,n,ch)
   m=Media(br.pair,ct.addressof(q),n,ch,1,start,end,first,last)
   o=Owner()
   assert lib.sub_commit(ct.byref(o),ct.byref(m)) and lib.sub_gate(ct.byref(o),hold)
   g.wr(zone,struct.pack('<QIII4xfB3xff',p,0,ch,n,end,1,first,last))
   g.wr(voice,struct.pack('<B7xddd',hold,start,.5,.75))
   h=hashlib.sha256();wraps=0
   for i in range(1024):
    before=o.cursor; gain=(-.5 if i%2 else .6875)
    g.call(0x1003b8224,voice,zone,0,l,r,floats=(gain,))
    ret=lib.sub_frame(ct.byref(o),.5,.75,gain,out)
    assert ret in (0,1) and g.x(0)==(not ret)
    if not ret:break
    cu=struct.pack('<d',o.cursor); pair=struct.pack('<ff',*out)
    assert cu==g.rd(voice+8,8) and pair==g.rd(l,4)+g.rd(r,4)
    wraps+=o.cursor<before;h.update(cu+pair)
   assert bool(wraps)==bool(hold)
   rows.append(dict(shape=[n,ch],gate=hold,reached=i+int(ret),wraps=wraps,words_sha=h.hexdigest()))
assert not g.unknown
print(json.dumps(dict(build=e['pin'](e['BIN']),actual_PC='0x1003b8224',cases=rows),indent=2))
