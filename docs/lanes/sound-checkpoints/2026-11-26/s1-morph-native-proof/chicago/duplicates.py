from pathlib import Path
import collections,hashlib,re,subprocess,sys
f=sys.argv[1];b=Path(f).read_bytes();spans={}
for l in subprocess.check_output(['readelf','-SW',f],text=True).splitlines():
 m=re.match(r'\s*\[\s*(\d+)\]\s+\S+\s+\S+\s+([a-f0-9]+)\s+([a-f0-9]+)\s+([a-f0-9]+)',l)
 if m:
  i,v,o,n=m.groups();spans[i]=tuple(int(x,16) for x in (v,o,n))
seen=set(); groups=collections.defaultdict(list)
for l in subprocess.check_output(['readelf','-sW',f],text=True).splitlines():
 a=l.split()
 if len(a)<8 or a[3]!='FUNC' or a[6] not in spans:continue
 v,n=int(a[1],16),int(a[2]);sv,o,sn=spans[a[6]]
 if n<24 or not sv<=v<=sv+sn-n or v in seen:continue
 seen.add(v)
 groups[b[o+v-sv:o+v-sv+n]].append((a[7],v,n))
dupes=sorted((v for v in groups.values() if len(v)>1),key=lambda a:-(len(a)-1)*a[0][2])
print(f'equal executable byte strings (not proved relocation-equivalent); functions scanned {len(seen)}; conservative upper bound {sum((len(x)-1)*x[0][2] for x in dupes)} B')
for g in dupes[:20]:print(g)
