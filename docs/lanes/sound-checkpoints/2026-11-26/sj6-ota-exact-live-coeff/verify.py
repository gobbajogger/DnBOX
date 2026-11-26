import ctypes as C,json,sys,struct
from pathlib import Path
p=Path('sj6-modern-owner/probe.py');ns={};exec(compile(p.read_text().split('def trial(')[0],str(p),'exec'),ns)
e=ns['Emu'](str(ns['BIN']))
t=C.CDLL(sys.argv[1]);buf=C.c_ubyte*24
t.main_request.argtypes=[C.POINTER(C.c_ubyte),C.c_float,C.c_float,C.c_int32]
t.main_frame.argtypes=[C.POINTER(C.c_ubyte)];t.main_frame.restype=C.c_float
fp=lambda v:struct.pack('<f',v)
flt_bits=lambda v:struct.unpack('<d',fp(v)+b'\0'*4)[0]
rows=words=0
for fs in (32000,44100,48000,96000):
 p=e.calloc(0xa0);e.wf(p+0x40,fs)
 s=buf()
 for hz,duration,count in [(20,16,0),(100,16,4),(100,31,8),(30000.0078125,16,17),(35000,5,8),(8500,1,1),(8510.4130859375,0,2),(1337,-3,3),(30,113,48),(14400,23,25),(0,11,12),(3,99,2)]:
  e.call(0x1003b5d38,p,duration&0xffffffff,1,doubles=(hz,0.,flt_bits(1.)))
  t.main_request(s,hz,fs,duration)
  assert fp(e.rf(p+0x60))==bytes(s[:4]),('host log2f vs receiver not exact here',fs,hz)
  assert fp(e.rf(p+0x64))==bytes(s[4:8]),(fs,hz)
  assert e.r32(p+0x88)==struct.unpack('<I',bytes(s[16:20]))[0]
  if e.r32(p+0x88):
   assert e.rd(p+0x68,8)==bytes(s[8:16]),(fs,hz,'unrounded target/step')
  for i in range(count):
   e.call(0x1003b83c4,p,floats=(0.,))
   r=t.main_frame(s)
   assert fp(e.rf(p+0x64))==fp(r),('frame mismatch',fs,hz,i)
   assert e.r32(p+0x88)==struct.unpack('<I',bytes(s[16:20]))[0]
   words+=1
  rows+=1
assert not e.unknown,e.unknown
print(json.dumps(dict(source=ns['pin'](ns['BIN']),requests_exact=rows,frames_exact=words,old_g_at_request='unchanged when live; first path immediate',scope='original attached sampler receiver cold, warm, retained mid-ramp targets, equivalent clamp repeats, interrupted bursts, final snap; no wider mode or knob qualification'),indent=2))
