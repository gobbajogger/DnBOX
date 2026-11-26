import ctypes as C,sys,json,math,struct
from pathlib import Path
from emu import IMP_BASE
p=Path('sj6-modern-owner/probe.py')
ns={};exec(compile(p.read_text().split('def trial(')[0],str(p),'exec'),ns)
e=ns['Emu'](str(ns['BIN'])); assert [e.import_names[(e.r64(i)-IMP_BASE)//4] for i in (0x100755af8,0x1007559f8,0x100755ce8)]==['_log2f','_exp2f','_tanf']
libs=[C.CDLL(a) for a in sys.argv[1:]]
for l in libs:l.main_settle_g.argtypes=[C.c_float,C.c_float]; l.main_settle_g.restype=C.c_float
f=lambda x:struct.unpack('<f',struct.pack('<f',x))[0]
bits=lambda x:struct.unpack('<I',struct.pack('<f',x))[0]
def loglaw(hz,fs):
 cap=min(30000.,f(fs*f(.9)))
 v=max(3.,min(f(hz),cap))
 v=f(2**f(math.log2(v)))
 return f(math.tan(f(f(f(math.pi)*v)/f(fs+fs))))
cases=[]; aggregate=[]
for fs in (32000,44100,48000,96000):
 for hz in (0,3,15,20,100,500,1000,3500,8510.4130859375,11000,14400,16000,18000,22000,23000,24000,26000,28000,29000,30000.0078125,35000):
  p=e.calloc(0xa0);e.wf(p+0x40,fs)
  for r in (0.,.001,.1,.25,.5,.65,.75,1.):
   e.call(0x1003b5d38,p,16,1,doubles=(hz,r,struct.unpack('<d',struct.pack('<II',bits(1.),0))[0]))
   g,fb,mix,trim=[e.rf(p+i) for i in (0x68,0x70,0x78,0x80)] if len(cases)%8 else [e.rf(p+i) for i in (0x64,0x70,0x78,0x80)]
   if r==0:
    assert fb==0 and mix==1 and trim==1
    exact=loglaw(hz,fs)
    assert g==exact,(fs,hz,g,exact)
    cand=[l.main_settle_g(hz,fs) for l in libs]
    aggregate.append([abs(bits(z)-bits(g)) for z in cand])
   cases.append([fs,hz,r,g,fb,mix,trim])
assert not e.unknown,e.unknown
print(json.dumps(dict(expected_sha=ns['pin'](ns['BIN']),control_source='reached setup 0x1003b5d38, q input is effective receiver argument not a UI value',new_state='fresh filter and rate, then repeated q-only writes; g unchanged with no new target',columns=['rate','cutoff_hz','physical_feedback_input','g','feedback_used','filter_mix_nominal','gain_trim'],cases=cases,zero_settled_exact_control_reconstruction=len(aggregate),max_ulp_stock_fast=[max(x) for x in zip(*aggregate)]),indent=1))
