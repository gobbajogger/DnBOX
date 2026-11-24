"""Compare top actual text symbol costs from two normal generated maps."""
import collections,re,sys
from pathlib import Path
def sizes(file):
    t=Path(file).read_text()
    t=t[t.index('\n.text '):].split('\n.ram_text ',1)[0]
    s=collections.Counter()
    for name,addr,n in re.findall(r'^ \.(text\.[^\s]+)\s+(0x[0-9a-f]+)\s+(0x[0-9a-f]+)',t,re.M):
        if int(addr,16)>=0x2000120:s[re.sub(r"\.\d+$","",name[5:])]+=int(n,16)
    return s
a=sizes(sys.argv[1]);b=sizes(sys.argv[2])
# engine_render appears out of line in S1-enabled LTO output; attribute its former ISR body jointly.
for x in (a,b):
    x["engine_render + fm1_alnk0_irq"]=x.pop("engine_render",0)+x.pop("fm1_alnk0_irq",0)
for s in sorted(a.keys()|b.keys(),key=lambda n:-(b[n]-a[n]))[:19]:
    print(f'{s:43s} {a[s]:5d} -> {b[s]:5d} delta {b[s]-a[s]:+5d}')
