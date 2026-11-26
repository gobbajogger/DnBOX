"""Emit exact three-bit ULP corrections from executed source oracle."""
import json
from pathlib import Path
p=Path(__file__).resolve().parent
grid=json.loads((p/'oracle.json').read_text())['grid']
base=[list(map(int,q.split())) for q in (p/'base-grid.txt').read_text().splitlines()]
assert len(grid)==len(base)==128
print('static const uint8_t sj6_fx_fix[2][48]={')
for j,shift in enumerate((2,1)):
 v=[x[j]-y[j]+shift for x,y in zip(grid,base)]
 assert min(v)>=0 and max(v)<8
 packed=sum(x<<(3*i) for i,x in enumerate(v)).to_bytes(48,'little')
 print('{'+','.join('0x%02x'%b for b in packed)+'},')
print('};')
