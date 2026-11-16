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
