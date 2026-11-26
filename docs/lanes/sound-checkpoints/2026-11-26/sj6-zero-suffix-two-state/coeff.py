import math,struct,sys
from pathlib import Path
a,b,c=map(float.fromhex,['0x1.ffa2ap-1','0x1.10427p-19','0x1.ff4564p-1'])
g=math.sqrt(b/(2+2*c-b));den=(2+2*c-b)/4
k=(1-c)/(2*g*den)
a1=1/(1+g*(g+k));a2=g*a1;a3=g*a2
binary32=lambda x:struct.unpack('<f',struct.pack('<f',x))[0]
p1=float.fromhex('0x1.fd70a4p-1');p2=binary32(.999)
v=dict(TPT_K=k,TPT_A1=a1,TPT_A2=a2,TPT_A3=a3,TPT_M=a1-1,DC1_DELTA=1-p1,DC2_DELTA=1-p2)
Path(sys.argv[1]).write_text(''.join(f'#define {n} {value.hex()}f\n' for n,value in v.items()))
