"""Verify report's conditional settled pan against actual current _sp_panst_compute."""
from pathlib import Path
import hashlib,json,numpy as np
e={}
p=Path('sj6-modern-owner/probe.py')
exec(compile(p.read_text().split('def trial(')[0],str(p),'exec'),e)
uc=e['Emu'](str(e['BIN']))
f=uc.im.sym_by_name['_sp_panst_compute'];assert f==0x100354d10
state=uc.calloc(8);b=[uc.calloc(4) for _ in range(4)]
rng=np.random.default_rng(244)
inp=np.concatenate([np.array([[1,-1],[0.,0.],[-0.,-0.]],dtype='f4'),rng.uniform(-4,4,(1024,2)).astype('f4')]);r=[]
for pair in inp:
 for s,x in zip(b,pair):uc.wr(s,x.tobytes())
 uc.call(f,0,state,*b)
 r.append([uc.rf(i) for i in b[2:]])
r=np.array(r,dtype='f4')
t=(inp.astype('f8')*np.array([float.fromhex('0x1.6a09e5e333598p-1'),float.fromhex('0x1.6a09e6ecb41fep-1')])).astype('f4')
assert np.array_equal(t.view('u4'),r.view('u4')) and not uc.unknown
print(json.dumps(dict(original=e['pin'](e['BIN']),actual_current_symbol=hex(f), signed_float32_words_bit_exact=r.size, digest=hashlib.sha256(r.tobytes()).hexdigest(),scope='settled mode0+position0 pan primitive; connected original pan/branch ordering scoped to source graph and authored control admission'),indent=2))
