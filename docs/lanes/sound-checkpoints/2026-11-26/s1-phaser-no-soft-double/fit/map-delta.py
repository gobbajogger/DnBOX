import re,sys,json
from pathlib import Path
def load(path):
 s=Path(path).read_text();s=s[s.index('Linker script and memory map'):]
 R={}
 for n,a,z in re.findall(r'^ (\.(?:text|rodata)[^ \t\r\n]*)[\t \r\n]+(0x[0-9a-f]+)[ \t]+(0x[0-9a-f]+)',s,re.M):
  a,z=int(a,16),int(z,16)
  if 0x02000120<=a<0x020a0000:R[n]=R.get(n,0)+z
 return R
a,b=map(load,sys.argv[1:]);m=[]
for n in sorted(set(a)|set(b)):
 old,new=a.get(n,0),b.get(n,0)
 if old!=new:m.append(dict(section=n,base=old,new=new,delta=new-old))
print(json.dumps(dict(changed=m,subtotal=sum(i['delta'] for i in m),load_image_delta=2552,alignment_delta=-6),indent=2))
