#!/usr/bin/env python3
"""Synthetic qualification-contract tests; never actual visual acceptance."""
import copy,importlib.util,pathlib
p=pathlib.Path(__file__).resolve().parents[1]/'tools/dnbox-ui-gate.py';spec=importlib.util.spec_from_file_location('uigate',p);m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m)
h='a'*64;base={'profileSha256':h,'run':{'exit':0},'detectorExit':0,'coverageComplete':True,'checks':{str(i):{'status':'PASS'} for i in range(1,25)}}
assert not m.unmet(base,h)
partial=copy.deepcopy(base);partial['coverageComplete']=False
for i in range(5,25):partial['checks'][str(i)]['status']='NOT_RUN'
assert len(m.unmet(partial,h))==21
for status in ['FAIL','NOT_RUN','SKIP',None]:
 r=copy.deepcopy(base);r['checks']['1']['status']=status;assert m.unmet(r,h)
for key,value in [('profileSha256','b'*64),('coverageComplete',False),('detectorExit',1)]:
 r=copy.deepcopy(base);r[key]=value;assert m.unmet(r,h)
r=copy.deepcopy(base);r['run']['exit']=2;assert m.unmet(r,h)
print('PASS 10 UI contract cases; partial detector0 never qualifies full UI')

# Actual supplied v2 reference identity; this does not render or accept a page.
import tempfile,shutil,json
root=p.parents[1]
with tempfile.TemporaryDirectory() as tmp:
 r=pathlib.Path(tmp)
 for rel in ['tests/ui_defects/canonical_spec.py','tests/ui_defects/canonical-spec.json','docs/design/UI-SOURCE-OF-TRUTH.md']:
  dest=r/rel;dest.parent.mkdir(parents=True,exist_ok=True);shutil.copy2(root/rel,dest)
 reference=m.authenticate_reference(r)
 assert reference['path']=='docs/design/UI-SOURCE-OF-TRUTH.md'
 assert reference['sha256']=='c9e69f4885a85e51862ef1092af168760f50075a1daf4222f0be2486e0c649ad'
 descriptor=r/'tests/ui_defects/canonical-spec.json'
 d=json.loads(descriptor.read_text());d['path']='docs/ui/UI-SOURCE-OF-TRUTH.md';descriptor.write_text(json.dumps(d))
 try:m.authenticate_reference(r)
 except ValueError:pass
 else:raise AssertionError('historical reference accepted')
print('PASS v2 wrapper reference authentication and old-path rejection; no page acceptance')
