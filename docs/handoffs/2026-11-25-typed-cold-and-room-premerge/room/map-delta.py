import collections,re,sys
from pathlib import Path
PREFIXES=('.text','.rodata','.data','.ram_text','.c1_text','.bss','.pool','.lab_','.xdata')
TOP=re.compile(r'^([.][\w.]+)\s+0x[0-9a-f]+\s+(0x[0-9a-f]+)',re.M)
SUB=re.compile(r'^ ([.][\w.]+)\s+0x[0-9a-f]+\s+(0x[0-9a-f]+)\s+(?:\S+\.o|\S+\(\S+\.o\))',re.M)
def map_read(p):
 t=Path(p).read_text()
 y={n:int(s,16) for n,s in TOP.findall(t)}
 out=collections.Counter()
 t=t[t.index('\n.text '):t.index('\n.ram_sink ')]
 for k,v in SUB.findall(t):
  k=re.sub(r'\.\d+$','',k)
  out[k]+=int(v,16)
 return xip(y),out
def xip(y):return {n:v for n,v in y.items() if n.startswith(PREFIXES)}
a,aa=map_read(sys.argv[1]);b,bb=map_read(sys.argv[2])
print('SECTION')
for k in a.keys()|b.keys():
 print(k,a.get(k,0),b.get(k,0),b.get(k,0)-a.get(k,0))
print('HOT .text section child deltas (fold numeric LTO suffix)')
d={k:bb[k]-aa[k] for k in aa.keys()|bb.keys()}
focus=('rv8','s1c_render','IMA_','s1u_rvb','ima_decode')
for k,v in sorted(d.items(),key=lambda x:(-abs(x[1]),x[0])):
 if any(q in k for q in focus) and v:print(k,v)
print('Other .text section child deltas')
for k,v in sorted(d.items(),key=lambda x:(-abs(x[1]),x[0])):
 if abs(v)>=8 and not any(q in k for q in focus):print(k,v)
assert sum(d.values())==sum(bb.values())-sum(aa.values())
print('Total child delta',sum(d.values()),'focus delta',sum(v for n,v in d.items() if any(q in n for q in focus)))
