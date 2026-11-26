"""Create independent table data. No original partials, waveform, or PCM input.
Only use the observed row-dependent limits as a guard against aliasing.
"""
import math,sys,cmath
from table_preflight import terms,synth
N,H,Q=map(int,sys.argv[1:])
assert H<N/2 and Q in (8,16) and N in (32,64,128)
# Seven source-backed anti-alias row supports; Fourier weights are ours.
caps=((14,14,14,8,4,2,1),(46,32,16,8,4,2,1))
C=[chr(48+q) for q in (0,1)]
banks=[];scale=[]
for shape in range(2):
    aligned=-cmath.phase(terms(shape,1)[1])/math.tau
    rows=[synth(shape,N,min(H,cap),aligned,True) for cap in caps[shape]]
    amplitude=max(abs(x) for r in rows for x in r)
    qmax=(1<<(Q-1))-1
    scale.append(amplitude/qmax)
    banks.append([tuple(round(x*qmax/amplitude) for x in r) for r in rows])
s='#include <stdint.h>\n'
s+=f'enum {{ TBL_N={N}, TBL_H={H}, TBL_Q={Q} }};\n'
s+=f'static const int{Q}_t TBL[2][7][{N}]={{\n'
for b in banks:
    s+=' {\n'
    for r in b:s+='  {'+','.join(map(str,r))+'},\n'
    s+=' },\n'
s+='};\n'
s+='static const float TBL_GAIN[2]={'+','.join(float(a).hex()+'f' for a in scale)+'};\n'
print(s)
