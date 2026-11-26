from pathlib import Path
from collections import Counter
import json,re,sys
t=Path(sys.argv[1]).read_text().split('#else')[0]
def vals(s):
 a=re.search(r'\b'+s+r'\[\d+\]\s*=\s*\{([^}]+)\}',t)
 return bytes(int(v.strip(),0) for v in a.group(1).split(',') if v.strip())
lut=vals('x0x_smp_huff_lut')
out={}
for name,n in [('hh',20250),('crash',43300),('ride',46999)]:
 blob=vals('x0x_smp_'+name)
 bit=ix=0; rows=[]
 for i in range(n):
  j=bit>>3
  e=lut[((blob[j]|(blob[j+1]<<8))>>(bit&7))&63]
  bit+=e>>4; mag=e&7
  rows.append((ix,mag))
  ix=max(0,min(88,ix+(-1,-1,-1,-1,2,4,6,8)[mag]))
 out[name]={}
 for label,a in [('first_4096',rows[:4096]),('all',rows)]:
  h=Counter(k for k,m in a)
  m=Counter(m for k,m in a)
  width=48
  off=max(range(90-width),key=lambda i:sum(h[k] for k in range(i,i+width)))
  out[name][label]={'samples':len(a),'mag0_1_count':m[0]+m[1],
                    'best_48_idxrange':(off,off+width-1),
                    'best_48_cover':sum(h[k] for k in range(off,off+width)),
                    'fixed_39_86_cover':sum(h[k] for k in range(39,87)),
                    'best_48_table_bytes_excludes_decoder':width*8*2}
print(json.dumps(out,indent=2))
