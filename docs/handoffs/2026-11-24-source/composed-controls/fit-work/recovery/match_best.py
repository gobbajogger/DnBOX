"""Find byte-verified files from exact historical unified diffs, never fuzz."""
from pathlib import Path
from collections import defaultdict, deque
import hashlib,re,sys,heapq,itertools

H=Path(sys.argv[1]).resolve()
SRC=Path(sys.argv[2]).resolve()
C=H/'composed-controls'
LIMIT=int(sys.argv[3]) if len(sys.argv)>3 else 450
wanted={p:h for h,p in (line.split(None,1) for line in (C/'changed-source.sha256').read_text().splitlines())}
pending={k:v for k,v in wanted.items() if not (SRC/k).is_file() or hashlib.sha256((SRC/k).read_bytes()).hexdigest()!=v}
if len(sys.argv)>4: pending={k:v for k,v in pending.items() if k.endswith(sys.argv[4])}
def ident(a):
    return hashlib.sha256(a).hexdigest()
def fname(x):
    for k in pending:
        if x.endswith('/'+k) or x==k:return k
def read_patch(p):
    ff=None; res=defaultdict(list); deleted=False; oldnull=False
    for l in p.read_bytes().splitlines(keepends=True):
        if l.startswith(b'--- '):
            ff=None; oldnull=l.startswith(b'--- /dev/null')
        elif l.startswith(b'+++ '):
            ff=fname(l[4:].decode(errors='replace').split(None,1)[0])
        elif ff and (m:=re.match(rb'^@@ -(\d+)(?:,\d+)? \+(\d+)',l)):
            res[ff].append([int(m[1]),[],[],oldnull])
        elif ff and res[ff]:
            if l[:1] in (b' ',b'-'):res[ff][-1][1].append(l[1:])
            if l[:1] in (b' ',b'+'):res[ff][-1][2].append(l[1:])
    return {k:[(n,b''.join(o),b''.join(t),empty)for n,o,t,empty in v] for k,v in res.items()}
def diff_apply(s,hunks):
    out=[];pos=0
    for _,pre,post,empty in hunks:
        if not pre:
            if not empty or s:return None
            off=0
        else:
            off=s.find(pre,pos)
            if off<0:return None
            if off and s[off-1]!=10:return None
        out.extend((s[pos:off],post));pos=off+len(pre)
    return b''.join(out)+s[pos:]
final={}
patches=defaultdict(list)
for p in sorted(H.rglob('*.patch')):
    for k,r in read_patch(p).items():
        if p==C/'patches'/(k+'.patch'):final[k]=r
        patches[k].append((str(p.relative_to(H)),r))
for file,goal in pending.items():
    port=SRC/file
    candidates=[(port.read_bytes() if port.is_file() else b'',[])]
    b=Path('fw')/file
    if b.is_file():candidates.append((b.read_bytes(),['published source']))
    for p in H.rglob(Path(file).name):
        if p.is_file() and str(p).endswith('/'+file):candidates.append((p.read_bytes(),[str(p.relative_to(H))]))
    seen=set()
    frontier=[];count=itertools.count()
    fin=final.get(file)
    def score(s):
        return sum(len(pre) for _,pre,post,empty in fin or [] if pre and pre in s)
    for v, q in candidates: heapq.heappush(frontier,(-score(v),next(count),v,q))
    found=False
    while frontier and len(seen)<LIMIT:
        _,_,s,r=heapq.heappop(frontier)
        h=ident(s)
        if h in seen:continue
        seen.add(h)
        if h==goal:
            port.parent.mkdir(parents=True,exist_ok=True);port.write_bytes(s);print("EXACT",file,r,flush=True);found=True;break
        insert=[]
        for label,changes in patches[file]:
            q=diff_apply(s,changes)
            if q is None or q==s:continue
            v=ident(q)
            if v==goal:
                port.parent.mkdir(parents=True,exist_ok=True);port.write_bytes(q);print("EXACT",file,r+[label],flush=True);found=True;break
            if v not in seen:insert.append((score(q),q,r+[label]))
        if found:break
        for k,q,b in insert:heapq.heappush(frontier,(-k,next(count),q,b))
    if not found:print("NEEDS PREIMAGE",file,len(seen),flush=True)
