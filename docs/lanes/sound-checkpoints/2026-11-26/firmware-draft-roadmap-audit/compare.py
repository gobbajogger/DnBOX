import os,subprocess,json
from collections import Counter
E=dict(os.environ,GIT_NO_LAZY_FETCH='1')
g=lambda *a: subprocess.check_output(['git','-C','fw',*a],env=E)
tip='73749f55d17a0643f1ddd072114172d29eecc2ae'; base='8f97fe7d2bbf4f878cd0061c657c13c6f4322ea6'
heads={1010:'e15d665426fbe853036cef43c3cf5f18b20ecbec',1012:'14c6df7b585d4ce73817ac8411977cc577b35486',1013:'fbaac6e4a7034ef604d6efa44f41a1077b056aad',1014:'d01f0e79a197abb2b10829834b1566b908cb04f4',1016:'2861005eebbb2d684d5cfded9eba34e19514892c'}
def tree(ref):
 q={}
 for item in g('ls-tree','-r','-z',ref).split(b'\0'):
  if not item: continue
  tok,path=item.split(b'\t',1); q[path.decode()]=tok.decode()
 return q
a,b=tree(base),tree(tip)
result={}
for n,c in heads.items():
 h=tree(c); out={}; by={}
 for p in set(a)|set(h):
  x,y,z=a.get(p),h.get(p),b.get(p)
  if x==y:continue
  w='exact_tip' if y==z else 'unchanged_at_tip' if x==z else 'different_at_tip'
  out[w]=out.get(w,0)+1
  if p.startswith(('firmware/src/','tools/','scripts/','tests/host/','firmware/hal/','docs/lanes/')) or n!=1010:
   by[p]=w
 lines=len(g('rev-list',base+'..'+c).splitlines())
 result[n]={'head':c,'counts':out,'commits':lines,'paths':dict(sorted(by.items()))}
print(json.dumps(dict(tip=tip,base=base,prs=result),indent=2))
