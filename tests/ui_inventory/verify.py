#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""Read-only supplemental validation of a saved real renderer attempt."""
import argparse,hashlib,importlib.util,json,sys
from pathlib import Path
from PIL import Image

def verify(evidence,source,output):
    evidence=evidence.resolve();source=source.resolve()
    report=json.loads((evidence/'report.json').read_text());closure=json.loads((evidence/'source-closure.json').read_text())
    for name,digest in closure.items():
        if hashlib.sha256((source/name).read_bytes()).hexdigest()!=digest:raise ValueError('changed frozen source '+name)
    if hashlib.sha256(json.dumps(closure,sort_keys=True).encode()).hexdigest()!=report['sourceClosure']:raise ValueError('source closure mismatch')
    sys.path.insert(0,str(evidence/'runner-inputs'))
    from run import verify_tree
    tree=verify_tree(source)
    if tree!=report['productionTree']:raise ValueError('production tree receipt mismatch')
    sys.path.insert(0,str(evidence/'source/tests/ui_defects/first_slice'))
    from screen_walk import validate
    from full_geometry import validate_audits
    log=(evidence/'render.log').read_text()
    legacy=validate(log,evidence/'host',report['rendererExit'])
    audits=validate_audits(log)
    states=set();files=set();captures=set();count=0
    for name,frames in report['frames'].items():
        if not frames:raise ValueError('empty stable state')
        states.add(name)
        for frame in frames:
            path=evidence/frame['path']
            if path.is_symlink() or path in files:raise ValueError('linked or duplicate invocation PNG')
            files.add(path)
            if hashlib.sha256(path.read_bytes()).hexdigest()!=frame['sha256']:raise ValueError('changed PNG '+frame['path'])
            with Image.open(path) as im:
                im.load()
                if im.format!='PNG' or im.size!=(240,240):raise ValueError('invalid PNG')
                if hashlib.sha256(im.convert('RGB').tobytes()).hexdigest()!=frame['pixelSha256']:raise ValueError('changed decoded RGB')
            if frame['capture'] in captures or not frame['state']:raise ValueError('duplicate capture or missing actual state')
            captures.add(frame['capture']);count+=1
    if captures!=set(range(count)):raise ValueError('truncated invocation receipts')
    if len(states)!=report['uniqueFrames']:raise ValueError('state count mismatch')
    lanes={}
    for part in range(5):
        rows=[frame for frames in report['frames'].values() for frame in frames if frame['id'].startswith('live-part'+str(part)+'-lane')]
        kinds={frame['state']['lane'] for frame in rows}
        if kinds!=set(range(len(report['stepLaneLabels']))):raise ValueError('missing physical lane family state')
        lanes[str(part)]=[report['stepLaneLabels'][k] for k in sorted(kinds)]
    result=dict(status='PASS',scope='Saved-source/tree, complete legacy walk/audits, PNG/RGB/actual-state integrity and physical lane family coverage only; F1 acceptance remains FAIL and extended final-text geometry requires separate producer.',
                reportSHA256=hashlib.sha256((evidence/'report.json').read_bytes()).hexdigest(),verifierSHA256=hashlib.sha256(Path(__file__).read_bytes()).hexdigest(),
                productionTree=tree,legacyWalkerFrames=legacy['screens'],legacyGeometry=audits,
                actualSnapshots=count,stableStateIDs=len(states),physicalLaneFamilies=lanes,F1=report['F1'],visualApproval=False)
    with output.open('x') as f:json.dump(result,f,indent=2);f.write('\n')
    print(json.dumps(result))

if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('--evidence',type=Path,required=True);p.add_argument('--source-freeze',type=Path,required=True);p.add_argument('--output',type=Path,required=True);a=p.parse_args()
    try:verify(a.evidence,a.source_freeze,a.output)
    except (OSError,ValueError) as e:print(json.dumps({'status':'FAIL','reason':str(e)}));raise SystemExit(1)
