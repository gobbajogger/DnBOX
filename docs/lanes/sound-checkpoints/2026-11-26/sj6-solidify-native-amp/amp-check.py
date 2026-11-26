from pathlib import Path
import sys,json,subprocess,struct,math
t=Path(sys.argv[1]);d=json.loads((t/(sys.argv[2] if len(sys.argv)>2 else 'native.json')).read_text())
r={}
for m in d['cases']:
 start=' '.join(map(str,m['raw']))+'\n'
 ev={16384:1,38434:2,49459:3 if m['kind']=='0' else 1,82534:2}
 pp=m['prep']; inp=start+''.join(f'{f} {n} {ev.get(f,0)}\n' for f,n,*_ in pp)
 s=subprocess.check_output([t/'amp-oracle'],input=inp.encode()).decode()
 j=[[int(a),int(b),int(c),float.fromhex(z)] for a,b,c,z in (x.split() for x in s.splitlines())]
 assert len(j)==len(pp)
 m={'rows':len(j),'stage_equal':0,'sample_bit_equal':0,'sample_maxabs':0,'first_different':None}
 for (f,n,st,x),(fg,ng,stage,y) in zip(pp,j):
  assert f==fg and n==ng
  m['stage_equal']+=st==stage
  m['sample_bit_equal']+=x==y
  m['sample_maxabs']=max(m['sample_maxabs'],abs(x-y))
  if x!=y and not m['first_different']:m['first_different']=[f,x,y,st,stage]
 r[str(ev[49459])+'@'+start.split()[0]]=m
print(json.dumps(r,indent=2))
