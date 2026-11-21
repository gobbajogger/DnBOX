#!/usr/bin/env python3
"""Summarize exact replacement ELF sizes, helper closure and fixed stack."""
from pathlib import Path
import ast
import csv
import hashlib
import json
import re
import sys

HERE=Path(__file__).resolve().parent
compact='--compact' in sys.argv[1:]
OUT=HERE/('compact-variant-native' if compact else 'replacement-native')
native=json.loads((OUT/'RESULT.json').read_text())
tree=ast.parse((HERE/'analyze_native.py').read_text())
ns={'re':re}
definitions=ast.Module(body=[n for n in tree.body if isinstance(n,ast.FunctionDef) and n.name in ['functions','register_count','frame_info']],type_ignores=[])
exec(compile(definitions,'reviewed-stack-helpers','exec'),ns)
functions,frame_info=ns['functions'],ns['frame_info']
source=((OUT if compact else HERE)/'src/dnbx_core.c').read_text()
core_names=set(re.findall(r'^(?:static\s+)?(?:uint32_t|size_t|void|int|float|unsigned)\s+(\w+)\s*\(',source,re.M))
roots=['juno6_render_stereo','juno6_set','juno6_note_on','juno6_chorus_process','juno6_read_envelope']
rows=[]
symbols=[]
for link in native['links']:
    label=link['model']+'-'+link['optimization']+'-'+('counters'+str(link['diagnostic_counters']) if compact else link['policy'])
    funcs=functions((OUT/(label+'-disassembly.log')).read_text())
    addresses={s['address']:s['name'] for s in link['symbols'] if s['section']=='.text' and s['bytes']}
    info={}
    missing=set()
    def collect(name):
        if name in info:return
        if name not in funcs:missing.add(name);return
        frame=frame_info(funcs[name],addresses)
        info[name]=frame
        if not frame['recognized']:return
        edges=set(frame['callees'])
        edges.update(n for n in frame['potential_tail_callees'] if n in funcs and n!=name)
        frame['edges']=sorted(edges)
        for child in frame['edges']:collect(child)
    for root in roots:collect(root)
    memo={}
    def depth(name,parents=()):
        if name in memo:return memo[name]
        if name in parents:raise RuntimeError('recursion '+str(parents))
        if name not in info or not info[name]['recognized']:return {'complete':False,'path':[name]}
        children=[depth(n,(*parents,name)) for n in info[name]['edges']]
        if any(not c['complete'] for c in children):return {'complete':False,'path':[name]}
        child=max(children,key=lambda c:c['bytes'],default={'bytes':0,'path':[]})
        row={'complete':True,'bytes':info[name]['fixed_frame_bytes']+child['bytes'],'path':[name,*child['path']]}
        memo[name]=row;return row
    paths={root:depth(root) for root in roots}
    row={k:v for k,v in link.items() if k not in ['symbols','sections']}
    row['incremental_image_bytes']=link['load_image_bytes']-(551492 if link['model']=='elite' else 551484)
    row['gate_shortfall_bytes']=1024-link['free_bytes']
    row['stack']={'frames':info,'root_paths':paths,'missing':sorted(missing),
                  'scope':'Fixed frames/direct calls within adapter and core only; caller/interrupt stacks and hardware high water excluded'}
    rows.append(row)
    for symbol in link['symbols']:
        name=symbol['name']
        if symbol['section']=='.text' and symbol['bytes'] and (name.split('.')[0] in core_names or name.startswith(('juno6_','fm_'))):
            symbols.append({'build':label,'symbol':name,'address':hex(symbol['address']),'bytes':symbol['bytes'],
                            'fixed_frame_bytes':info.get(name,{}).get('fixed_frame_bytes','')})
    print(label,'image',link['load_image_bytes'],'gate_shortfall',row['gate_shortfall_bytes'],'stack',paths)
(OUT/'ANALYSIS.json').write_text(json.dumps({'native_layout':native['native_layout'],'builds':rows},indent=2))
with (OUT/'SYMBOL-SIZES.csv').open('w',newline='') as file:
    writer=csv.DictWriter(file,fieldnames=['build','symbol','address','bytes','fixed_frame_bytes']);writer.writeheader();writer.writerows(symbols)

if compact:
    raise SystemExit(0)

# Verify the isolated source changes exactly; unrelated baseline files and
# target configuration are preserved, not reclaimed to create apparent fit.
base_manifest=json.loads((HERE.parent/'native-baseline-source-manifest.json').read_text())
entries=base_manifest['files'] if isinstance(base_manifest,dict) and 'files' in base_manifest else base_manifest
if isinstance(entries,dict):entries=[{'path':p,**v} for p,v in entries.items()]
changes=[];unchanged=0
for entry in entries:
    path=entry.get('path',entry.get('relative_path',''))
    if Path(path).is_absolute():path=str(Path(path).relative_to(HERE.parent/'native-baseline-76040'))
    if not path:continue
    base=HERE.parent/'native-baseline-76040'/path
    target=HERE/'replacement-tree'/path
    if not base.is_file() or not target.is_file():continue
    old=hashlib.sha256(base.read_bytes()).hexdigest();new=hashlib.sha256(target.read_bytes()).hexdigest()
    if old==new:unchanged+=1
    else:changes.append({'path':path,'baseline_sha256':old,'fixture_sha256':new})
(OUT/'SOURCE-DIFF-AUDIT.json').write_text(json.dumps({'baseline_unchanged_files':unchanged,'changed_files':changes,
    'added_files':['firmware/src/dsp/dnbx_core.c','firmware/src/dsp/dnbx_core.h'],
    'note':'Build outputs excluded; exact source pins are in replacement-PROVENANCE.json'},indent=2))
print('unchanged baseline source files',unchanged,'changed',len(changes))
