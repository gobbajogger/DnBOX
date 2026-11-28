#!/usr/bin/env python3
import bisect, collections, re, struct, subprocess, sys
from pathlib import Path
elf,obj,trace=map(Path,sys.argv[1:])
N,BASE,LO,HI=6340,0x205fda2,0x20608a0,0x2060c20
def read(p,*args):
    return subprocess.check_output(['readelf',*args,p],text=True,stderr=subprocess.DEVNULL)
def sect(p,n):
    l=next(l for l in read(p,'-WS').splitlines() if re.search(r'\]\s+'+re.escape(n)+r'\s',l))
    _,_,address,offset,size=l.split(']')[1].split()[:5]
    i,j=int(offset,16),int(size,16)
    return int(address,16),p.read_bytes()[i:i+j]
address,linked=sect(elf,'.text')
_,current=sect(obj,'.text.d9_render')
_,rr=sect(obj,'.rela.text.d9_render')
rel={q+j for q,_,_ in struct.iter_unpack('<III',rr) for j in range(6)}
old=linked[BASE-address:BASE-address+N]
different={i for i,(a,b) in enumerate(zip(old,current)) if a!=b}
assert len(old)==len(current)==N and different<=rel
print(f'{N} bytes; {len(different)} linked differences all inside relocation spans')
g=next(g for g in read(obj,'--debug-dump=decodedline').split('\n\n\n') if re.search(r'^profile-base.c\s+-\s+0x18c4$',g,re.M))
lines=[(int(c,16),a,int(b)) for a,b,c in re.findall(r'^(\S+\.\w+)\s+(\d+)\s+(0x\w+|0)\s',g,re.M)]
offs=[s[0] for s in lines]
assert len(offs)==1165 and offs==sorted(offs)
stats,counts=collections.Counter(),collections.Counter()
for row in trace.read_text().splitlines():
    core,addr,num=row.split()
    addr,num=int(addr,16),int(num)
    if core=='m1':
        counts['C1 misses']+=num
        if (LO&~63) <= addr < ((HI+63)&~63): counts['tom aligned misses']+=num
    if core!='1':continue
    counts['C1 cycles']+=num
    if BASE<=addr<BASE+N:counts['all d9_render cycles']+=num
    if not LO<=addr<HI:continue
    counts['tom hot strip cycles']+=num
    _,f,s=lines[bisect.bisect_right(offs,addr-BASE)-1]
    if f=='drum909_dsp.h':
        tag='biquad' if 185<=s<=190 else 'envelope' if 126<=s<=141 else 'phase' if s==196 else 'tanh' if 238<=s<=252 else None
    elif f=='profile-base.c':
        tag='phase' if 37<=s<=56 or 874<=s<=884 else 'shape' if 59<=s<=64 or s==889 else 'loop' if s==872 else 'noise/DC/other' if 885<=s<=893 else None
    else:tag=None
    stats[tag or 'unattributed/inlined']+=num
for name,n in counts.items(): print(name,n)
for k,n in stats.most_common(): print(k,n,f'{100*n/sum(stats.values()):.2f}%')
