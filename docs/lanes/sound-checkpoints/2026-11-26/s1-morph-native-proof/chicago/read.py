import json,struct,subprocess,sys
from pathlib import Path
f,r=sys.argv[1:]
s={}
for l in subprocess.check_output(['readelf','-sW',f],text=True).splitlines():
 a=l.split()
 if len(a)>=8 and a[2].isdigit() and a[6]!='UND':s[a[7]]=(int(a[1],16),int(a[2]))
b=Path(r).read_bytes()
def read(k,n):
 a,z=s[k];assert z==n*4
 return struct.unpack_from(f'<{n}I',b,a-0x01c00000)
half,us,ms,*us24=read('lab_chicago_worst',7)
assert us==read('x0x_dbg',13)[2] and us>11000 and 0<ms<=8000
print(json.dumps(dict(half=half,worst_us=us,latched_ms=ms,components_us=dict(zip(('block admission','dry voice/clock plus inline control ticks','wet FX/drive/master','stereo result copies'),(x/24 for x in us24))),sum_scoped_us=sum(us24)/24),indent=2))
