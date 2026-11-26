'''Lossless removal of identical independently authored rows, for pricing only.'''
from pathlib import Path
import sys,re
s=Path(sys.argv[1]).read_text()
rows=[tuple(map(int,q.split(','))) for q in re.findall(r'^  \{([\d,\-]+)\},$',s,flags=re.M)]
assert len(rows)==14 and len(set(map(len,rows)))==1
N=len(rows[0]);idx={};mapping=[]
for t in rows:
    if t not in idx:idx[t]=len(idx)
    mapping.append(idx[t])
start=s.index('static const int')
end=s.index('static const float TBL_GAIN')
width=re.search(r'const int(8|16)_t',s).group(1)
print(s[:start],end='')
print(f'#define TBL_DEDUP 1\nstatic const int{width}_t TBL[{len(idx)}][{N}]={{')
for t in idx:print(' {'+','.join(map(str,t))+'},')
print('};')
print('static const uint8_t TBL_IDX[2][7]={{'+','.join(map(str,mapping[:7]))+'},{'+','.join(map(str,mapping[7:]))+'}};')
print(s[end:],end='')
