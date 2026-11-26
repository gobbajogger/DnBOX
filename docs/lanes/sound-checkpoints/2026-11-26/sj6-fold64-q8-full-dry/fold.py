"""Offline source-coefficient transform, never reads holdouts.
Frozen A4 independent projections only; no reference table bytes."""
import cmath, math, argparse, json
from generate import transfer,halfband
p=argparse.ArgumentParser()
p.add_argument('anchor');p.add_argument('size',type=int);p.add_argument('width',type=int)
p.add_argument('--flat',action='store_true')
a=p.parse_args()
a4=json.load(open(a.anchor));N=a.size;Q=a.width;R=(1<<(Q-1))-1
out=[];indices=[];gains=[]
for shape in ('sine','tri'):
 rowmap=[]
 q=a4[shape]; f=math.tau*440/44100;c=q['normal_df1']
 h0=transfer(f,c,a4.get('native_suffix',False))
 coeff=[1+0j]
 for n,(db,ang) in enumerate(q['harmonics_h2_to_h18'],2):
    coeff.append(10**(db/20)*cmath.exp(1j*math.radians(ang))*abs(h0)*cmath.exp(1j*n*cmath.phase(h0))/transfer(n*f,c,a4.get('native_suffix',False)))
 caps=(14,14,14,8,4,2,1) if shape=='sine' else (46,32,16,8,4,2,1)
 rows=[]
 for band,limit in enumerate(caps):
    w=math.tau*(.35*(2**band)/64.) if band else math.tau*440/44100
    kern=lambda w:halfband(w)*halfband(w*.5)
    ang=cmath.phase(kern(w))
    hs=[h*kern(w*n)*cmath.exp(-1j*n*ang) if n>1 and not a.flat else h for n,h in enumerate(coeff[:min(limit,N//2-1)],1)]
    rows.append([sum((h*cmath.exp(1j*math.tau*j*n/N)).real for n,h in enumerate(hs,1)) for j in range(N)])
 scale=max(abs(v) for r in rows for v in r)/R
 gains.append(scale)
 for row in rows:
    r=tuple(round(v/scale) for v in row)
    if r not in out:out.append(r)
    rowmap.append(out.index(r))
 indices.append(rowmap)
print('#include <stdint.h>')
print(f'enum {{ TBL_N={N} }};')
print(f'static const int{Q}_t TBL[{len(out)}][TBL_N]={{')
for r in out:print('{'+','.join(map(str,r))+'},')
print('}; static const uint8_t TBL_MAP[2][7]={'+','.join('{'+','.join(map(str,r))+'}' for r in indices)+'};')
print('static const float TBL_GAIN[2]={'+','.join(float(g).hex()+'f' for g in gains)+'};')
