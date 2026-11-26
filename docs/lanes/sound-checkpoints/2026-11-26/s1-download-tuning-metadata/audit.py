import collections, hashlib, json, math, pathlib, sys, zipfile
from install_bank import parse_s1
from lite_bank import encode_lite
p=pathlib.Path(sys.argv[1])
assert hashlib.sha256(p.read_bytes()).hexdigest()=='68ad8b5fa0adca8b5da2b03f9303e9321c6a20907f5945e3fe879bc6494009a4'
counts=collections.Counter(); rows=[]; banks=0; brokes=0
with zipfile.ZipFile(p) as z:
 for n in sorted(z.namelist()):
  if not n.startswith('Synth one/') or not n.endswith('.json'):continue
  raw=z.read(n); src=json.loads(raw); no=[]
  for r in src:
   case=dict(r)
   ratios=case.pop('tuningMasterSet',None); name=case.pop('tuningName',None)
   no.append(case)
   if ratios is None:
    assert name is None; counts['absent']+=1; continue
   assert isinstance(name,str) and ratios and all(math.isfinite(w) and w!=0 for w in ratios)
   canonical=[]
   for w in ratios:
    v=abs(w)
    while v<1:v*=2
    while v>=2:v*=.5
    canonical.append(v)
   canonical.sort()
   if len(canonical)==12 and all(abs(w-2**(j/12))<=1e-6 for j,w in enumerate(canonical)):
    counts['explicit_12et']+=1;continue
   counts['custom']+=1;rows.append(len(canonical))
   if r['name']=='WSW - Keys - Broke Stringthing':
    brokes+=1
    assert canonical==[1.125,1.25,1.5,1.75]
    root=261.6255653006
    print('Broke Stringthing requested MIDI60–64 Hz:',*[round(root*x,6) for x in canonical+[2*canonical[0]]])
  a=encode_lite(parse_s1(raw,pathlib.Path(n).stem))
  b=encode_lite(parse_s1(json.dumps(no).encode(),pathlib.Path(n).stem))
  assert a==b
  banks+=1
assert (banks,counts['custom'],counts['explicit_12et'],counts['absent'],brokes)==(13,16,672,218,1)
print(dict(counts),'bank blobs exact with tuning metadata removed',banks,'custom ratios total',sum(rows),'degrees',min(rows),max(rows))
