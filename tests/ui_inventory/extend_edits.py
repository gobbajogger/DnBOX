#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""Capture the unchanged physical edit/persistence fixture on prepared source.

Preserves the primary attempt and emits a separate invocation plus a combined
mapping. It does not repeat the ordinary walk or claim native device writes.
"""
import argparse,hashlib,json,os,re,shutil,subprocess
from pathlib import Path
from PIL import Image

def extend(base):
    base=base.resolve();report=json.loads((base/'report.json').read_text());manifest=json.loads((base/'source-inventory.json').read_text())
    out=base/'extensions/edited-lanes'
    out.mkdir(parents=True,exist_ok=False);work=out/'source'
    shutil.copytree(base/'source',work,symlinks=True,ignore=shutil.ignore_patterns('build','__pycache__','.git'))
    source=work/'tests/host/ui_step_lane_edit_test.c'
    inputs={}
    for folder in ('firmware','host','tests','tools','experiments'):
        for p in sorted((work/folder).rglob('*')):
            if p.is_file() and '__pycache__' not in p.parts:inputs[p.relative_to(work).as_posix()]=hashlib.sha256(p.read_bytes()).hexdigest()
    (out/'instrumented-inputs.json').write_text(json.dumps(inputs,indent=2)+'\n')
    env={k:v for k,v in os.environ.items() if 'MUTATE' not in k}
    env.update(SMC_UI_OUT=str(out/'host'),SMC_UI_TEST_FILE='tests/host/ui_step_lane_edit_test.c')
    with (out/'render.log').open('x') as log:
        proc=subprocess.run(['sh','tests/host/smc_ui_test.sh'],cwd=work,env=env,stdout=log,stderr=subprocess.STDOUT,timeout=900)
    rows=[];states={};observed=set();(out/'renders').mkdir()
    for line in (out/'render.log').read_text().splitlines():
        if line.startswith('F1_STATE '):
            event=json.loads(line[9:]);capture=event.pop('capture')
            if capture in states:raise ValueError('duplicate edit state')
            states[capture]=event
        if line.startswith('F1_FRAME '):rows.append(json.loads(line[9:]))
    engine=(work/'firmware/src/app/engine.h').read_text()
    match=re.search(r'enum\s*\{([^}]*PART_909[^}]*NPARTS[^}]*)\}',engine)
    parts=[s.strip() for s in match[1].split(',') if s.strip()!='NPARTS']
    expected={f'edited-part{part}-kind{kind}' for part in range(len(parts)) for kind in range(len(manifest['stepLaneLabels']))}
    if proc.returncode or len(rows)!=len(expected) or {r['id'] for r in rows}!=expected:raise ValueError('edit renderer failed or incomplete coverage')
    if {r['capture'] for r in rows}!=set(range(len(rows))) or set(states)!=set(range(len(rows))):raise ValueError('truncated or duplicate edit invocation state')
    frames={}
    for row in rows:
        name=row['id'];capture=row['capture'];path=out/'host'/('f1-%04d-%s.png'%(capture,name));saved=out/'renders'/path.name
        with Image.open(path) as im:
            im.load()
            if im.size!=(240,240) or im.format!='PNG':raise ValueError('invalid edit PNG')
            pixels=im.convert('RGB').tobytes();im.save(saved,optimize=True)
        with Image.open(saved) as im:
            if im.convert('RGB').tobytes()!=pixels:raise ValueError('edit PNG changed pixels')
        if capture not in states:raise ValueError('missing actual edit state')
        indices=row['renderers']
        if any(type(i)!=int or i<0 or i>=len(manifest['renderers']) for i in indices):raise ValueError('invalid edit renderer identity')
        refs=[manifest['renderers'][i]['id'] for i in indices];observed.update(refs)
        frames[name]=[dict(id=name,capture=capture,invocation='edited-lanes',state=states[capture],
                           evidenceKind='COMPLETE_UI_FRAME_WITH_PHYSICAL_EDIT_AND_HOST_PROJECT_PERSISTENCE_FIXTURE',
                           path=saved.relative_to(base).as_posix(),sha256=hashlib.sha256(saved.read_bytes()).hexdigest(),
                           nativePNGsha256=hashlib.sha256(path.read_bytes()).hexdigest(),pixelSha256=hashlib.sha256(pixels).hexdigest(),renderers=refs,branches=[])]
    result=dict(status='PASS',scope='Exact existing physical lane-edit fixture, including host project writer/reader persistence; no device write or F1/R22 acceptance.',
                productionTree=report['productionTree'],rendererExit=proc.returncode,snapshots=len(rows),
                fixtureSHA256=hashlib.sha256(source.read_bytes()).hexdigest(),extensionRunnerSHA256=hashlib.sha256(Path(__file__).read_bytes()).hexdigest(),
                primaryReportSHA256=hashlib.sha256((base/'report.json').read_bytes()).hexdigest(),frames=frames,
                instrumentedInputClosure=hashlib.sha256(json.dumps(inputs,sort_keys=True).encode()).hexdigest())
    (out/'report.json').write_text(json.dumps(result,indent=2)+'\n');shutil.copyfile(__file__,out/'extension-runner.py')
    combined=dict(report);combined['frames']={name:[dict(frame,invocation='main') for frame in frameset] for name,frameset in report['frames'].items()}
    if combined['frames'].keys()&frames.keys():raise ValueError('extension state collision')
    combined['frames'].update(frames);combined['uniqueFrames']=len(combined['frames'])
    all_observed={ref for frameset in combined['frames'].values() for frame in frameset for ref in frame['renderers']}
    combined['observedRenderers']=len(all_observed);combined['uncovered']=[r['id'] for r in manifest['renderers'] if r['id'] not in all_observed]
    combined['uncoveredDisposition']={r['id']:r['sourceDisposition'] for r in manifest['renderers'] if r['id'] in combined['uncovered']}
    combined['sourceCoverage']={'status':'PARTIAL' if combined['uncovered'] else 'COMPLETE','observedFunctions':len(all_observed),'inventoriedFunctions':len(manifest['renderers']),'unobservedFunctions':len(combined['uncovered'])}
    combined['rendererInvocations']=[{'name':'main','report':'report.json','exit':report['rendererExit']},{'name':'edited-lanes','report':'extensions/edited-lanes/report.json','exit':proc.returncode}]
    combined['measurementScope']='Primary full walker plus independently retained exact physical edit/persistence invocation on the same instrumented production tree; external lane/R22 acceptance not evaluated.'
    lines=['stable_id\tsource_sha256\tregistered\tsource_disposition\tobserved_frames\tstatus\tsample_state']
    for row in manifest['renderers']:
        reached=sorted(name for name,frameset in combined['frames'].items() if any(row['id'] in frame['renderers'] for frame in frameset))
        lines.append('\t'.join([row['id'],manifest['sources'][row['path']],str(row['registeredInTranslationUnit']),row['sourceDisposition'],str(len(reached)),'OBSERVED' if reached else 'NOT_RUN',reached[0] if reached else '']))
    (base/'combined-stable-renderer-ids.tsv').write_text('\n'.join(lines)+'\n')
    (base/'combined-report.json').write_text(json.dumps(combined,indent=2)+'\n')
    print(json.dumps({k:v for k,v in result.items() if k!='frames'}))

if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('--evidence',type=Path,required=True);a=p.parse_args()
    try:extend(a.evidence)
    except (OSError,ValueError,subprocess.SubprocessError) as e:print(json.dumps({'status':'FAIL','reason':str(e)}));raise SystemExit(1)
