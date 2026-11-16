import sys,ctypes,struct
from conductor import *
from conductor import _default
def fb(x):return struct.pack('>f',x).hex()
_libm=ctypes.CDLL('libm.so.6')
_powf=_libm.powf;_powf.argtypes=[ctypes.c_float,ctypes.c_float];_powf.restype=ctypes.c_float
_expf=_libm.expf;_expf.argtypes=[ctypes.c_float];_expf.restype=ctypes.c_float
def powf(a,b):return float(_powf(ctypes.c_float(a),ctypes.c_float(b)))
def expf(x):return float(_expf(ctypes.c_float(x)))
expected=[f'D {i} {fb(_default(i,powf))}'for i in range(218)]
for flag in (0,1):
 r=plan_main_pass((),b'',17,bool(flag),256,frozenset({'control_apply','delay_enable_prepare','deferred_volume'}),powf)
 assert r.status=='OK_MAIN_PASS_ONLY'
 for j,c in enumerate(r.commands):expected.append(f'P {flag} {j} {c.id} '+str({'apply_control':0,'delay_enable_prepare':1,'defer_volume':2}[c.op])+f' {fb(c.value)} {fb(c.delay_seconds)}')
for i in range(1000):
 r=cutoff_step(CutoffState(2,F((i%101)/100.),F(((i*13)%101)/100.),True),F(float(i%37)/48000.),256,frozenset({'cutoff_apply','global_dispatch'}),expf)
 assert r.status=='PENDING_COMMAND_ACK'
 expected.append(f'C {i} {fb(r.commands[0].value)} {fb(r.state.current)} {fb(r.state.target)} {r.commands[0].token}')
for i in range(1000):
 r=beat_catchup(BeatState(2,10.,float(i%12)/1000.),10.+float(i%103)/1000.,120.,True,256)
 assert r.status=='OK_CLOCK'
 expected.append(f'B {i} {len(r.commands)} {fb(r.state.accumulator)}')
actual=open(sys.argv[1]).read().splitlines()
assert len(actual)==len(expected),(len(actual),len(expected))
for i,(a,b)in enumerate(zip(actual,expected)):assert a==b,(i,a,b)
print(f'{len(expected)} cross-language exact bit/state/command records pass; host libm only, not original-platform math proof')
