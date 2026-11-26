from pathlib import Path
import random, json, struct, sys, ctypes as C
base=Path(sys.argv[1]); rng=random.Random(651); f=lambda a:struct.pack('<f',a)
F=C.POINTER(C.c_float)
co=(C.c_float*5)(.22,.52,.31,.88,.04)
# Each row one physical base frame; Params change before both internal steps.
# Simulate rate paths without claiming these synthetic params are exact rate laws.
d={}
for kind,size in [('original',84),('packed',64),('separate',52)]:
 if not (base/(kind+'-dyn.so')).exists():continue
 a=C.CDLL(str(base/(kind+'-dyn.so'))); b=C.CDLL(str(base/(kind+'-new-dyn.so')))
 for q in (a,b):
  q.one.argtypes=[C.c_void_p,C.c_float,F,F];q.one.restype=C.c_float
 st=[(C.c_float*(size//4))() for _ in (0,1)]
 for i in range(8192):
  p=(C.c_float*4)(.005+3*(i/8192),.00037,.5 if i%2 else 1.,1.)
  x=rng.uniform(-12,12)
  assert f(a.one(st[0],x,p,co))==f(b.one(st[1],x,p,co))
  assert bytes(st[0])==bytes(st[1])
 d[kind]=8192
assert len(d)==3
print(json.dumps(d,indent=2))
