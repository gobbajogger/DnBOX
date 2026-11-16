#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""Run exact frozen renderer copies once; retain source closure, hooks and exits.
No hardware, network, source edits in inputs, generated workflows, or gate retries.
"""
import argparse, hashlib, json, os, shutil, subprocess, sys, time
from pathlib import Path
from collections import Counter
from instrument import instrument, HERE
from detectors import inspect_screen


def digest(path):return hashlib.sha256(path.read_bytes()).hexdigest()
def write(path,obj):path.write_text(json.dumps(obj,indent=2,sort_keys=True)+'\n')


def closure(root):
    rows=[]
    for p in sorted(root.rglob('*')):
        if any(x in {'.git','build','__pycache__'} for x in p.relative_to(root).parts):continue
        if p.is_symlink():
            link=os.readlink(p).encode();rows.append({'path':p.relative_to(root).as_posix(),'sha256':hashlib.sha256(link).hexdigest(),'bytes':len(link),'mode':'120000','symlink':os.readlink(p)})
        elif p.is_file():
            rows.append({'path':p.relative_to(root).as_posix(),'sha256':digest(p),'bytes':p.stat().st_size,'mode':oct(p.stat().st_mode & 0o777)})
    data=json.dumps(rows,sort_keys=True,separators=(',',':')).encode()
    return {'sha256':hashlib.sha256(data).hexdigest(),'files':rows}


def validate_screens(screens, complete):
    expected=json.loads((HERE/'expected-screens.json').read_text())['screens']
    wanted={s['id']:s for s in expected}
    if not complete or complete.get('rendererExit')!=0 or complete.get('screens')!=len(wanted):
        raise ValueError('incomplete renderer: expected explicit45-screen inventory')
    ids=[s.get('id') for s in screens]
    if len(ids)!=len(wanted) or len(set(ids))!=len(ids) or set(ids)!=set(wanted):
        raise ValueError('missing, extra or duplicate constructed screen')
    for screen in screens:
        if any(screen.get(k)!=v for k,v in wanted[screen['id']].items()):raise ValueError('screen identity fields mismatch')
        if screen.get('width')!=240 or screen.get('height')!=240:raise ValueError('wrong screen dimensions')
        mask=screen.get('mask',[])
        if len(mask)!=240 or any(len(row)!=240 or set(row)-{'0','1'} for row in mask):raise ValueError('missing or malformed actual raster mask')
        events=screen.get('events',[])
        if not screen.get('panels') or not any(e.get('kind')=='glyph' and e.get('pixels',0)>0 for e in events):raise ValueError('missing required panel/glyph structure')
        if not any(e.get('kind')=='finaltext' and e.get('expectedOpaquePixels',0)>0 for e in events):raise ValueError('missing final-frame native font audit')
        if any(not e.get('ref') for e in events if e.get('kind')=='glyph'):raise ValueError('glyph without typed reference')
    return True


def parse_trace(log):
    screens=[];current=None;complete=None
    for line in log.splitlines():
        if not line.startswith('FIRST:'):continue
        event=json.loads(line[6:]);kind=event['kind']
        if kind=='screen':
            current={**event,'events':[],'panels':[],'width':240,'height':240};screens.append(current)
        elif kind=='complete':complete=event
        elif current is None:raise ValueError('event without screen')
        elif kind=='mask':
            bits=event['bits'];assert len(bits)==240*240
            current['mask']=[bits[i:i+240] for i in range(0,len(bits),240)]
        elif kind=='panel':
            if event not in current['panels']:current['panels'].append(event)
        else:current['events'].append(event)
    validate_screens(screens,complete)
    return screens,complete


def validate_trace_against_log(log, raw):
    screens,complete=parse_trace(log)
    if raw!={'screens':screens,'complete':complete}:
        raise ValueError('saved trace differs from authenticated actual renderer log')
    return True


def validate_render_receipts(root):
    source=json.loads((root/'source-closure.json').read_text())
    inst=json.loads((root/'instrumented-closure.json').read_text())
    hooks=json.loads((root/'instrumentation.json').read_text())
    run=json.loads((root/'run.json').read_text())
    report=json.loads((root/'report.json').read_text())
    logpath=root/'renderer-first-attempt.log'
    if digest(logpath)!=run.get('logSha256'):raise ValueError('renderer log hash differs from actual run receipt')
    for data in (source,inst):
        actual=hashlib.sha256(json.dumps(data['files'],sort_keys=True,separators=(',',':')).encode()).hexdigest()
        if actual!=data.get('sha256'):raise ValueError('source manifest canonical hash mismatch')
    if not (source['sha256']==run.get('sourceClosureSha256')==report.get('sourceClosureSha256')==hooks.get('before')):
        raise ValueError('source closure/run/report/hook identity mismatch')
    if not (inst['sha256']==run.get('instrumentedClosureSha256')==report.get('instrumentedClosureSha256')==hooks.get('after')):
        raise ValueError('instrumented closure/run/report/hook identity mismatch')
    if report.get('run')!=run or run.get('exit')!=0:raise ValueError('report lacks successful actual renderer run receipt')
    srcrows={r['path']:r for r in source['files']};instrows={r['path']:r for r in inst['files']}
    if srcrows.get('SOURCE-PROFILE.json',{}).get('sha256')!=report.get('profileSha256'):
        raise ValueError('source profile differs from source manifest')
    changes=[p for p in sorted(set(srcrows)|set(instrows)) if srcrows.get(p)!=instrows.get(p)]
    allowed=['firmware/src/app/ui_drum_voice.inc','firmware/src/gfx.c','tests/host/first_slice_capture.c']
    if changes!=allowed or hooks.get('changedPaths')!=allowed:raise ValueError('unapproved instrumented source paths')
    raw=json.loads((root/'traces.json').read_text())
    validate_trace_against_log(logpath.read_text(),raw)
    return report,raw


def run_one(label, source, output):
    resultdir=output/label;resultdir.mkdir()
    before=closure(source);write(resultdir/'source-closure.json',before)
    work=resultdir/'instrumented-source'
    shutil.copytree(source,work,symlinks=True,ignore=shutil.ignore_patterns('.git','build','__pycache__'))
    copied=closure(work)
    if copied!=before:raise ValueError('copied source closure differs before hooks')
    instrument(work)
    after=closure(work);write(resultdir/'instrumented-closure.json',after)
    originals={r['path']:r for r in before['files']}; mutated={r['path']:r for r in after['files']}
    changes=[p for p in sorted(mutated) if mutated[p]!=originals.get(p)]
    allowed=['firmware/src/app/ui_drum_voice.inc','firmware/src/gfx.c','tests/host/first_slice_capture.c']
    if changes!=allowed:raise ValueError(f'unexpected instrumentation changes: {changes}')
    write(resultdir/'instrumentation.json',{'changedPaths':changes,'before':before['sha256'],'after':after['sha256'],
        'hooks':{p.name:digest(p) for p in HERE.iterdir() if p.is_file()}})
    env=os.environ.copy();env.update({'SMC_UI_OUT':str(resultdir/'host'),'SMC_UI_TEST_FILE':'tests/host/first_slice_capture.c'})
    command=['sh','tests/host/smc_ui_test.sh'];start=time.time()
    with (resultdir/'renderer-first-attempt.log').open('w') as log:
        proc=subprocess.run(command,cwd=work,env=env,stdout=log,stderr=subprocess.STDOUT,timeout=600)
    receipt={'command':command,'exit':proc.returncode,'firstAttempt':True,'durationSeconds':time.time()-start,
             'sourceClosureSha256':before['sha256'],'instrumentedClosureSha256':after['sha256'],
             'logSha256':digest(resultdir/'renderer-first-attempt.log')}
    write(resultdir/'run.json',receipt)
    if proc.returncode:return {'sourceClosureSha256':before['sha256'],'run':receipt,'status':'NOT_RUN','reason':'host renderer compile/run failed'}
    screens,complete=parse_trace((resultdir/'renderer-first-attempt.log').read_text())
    write(resultdir/'traces.json',{'screens':screens,'complete':complete})
    inspected=[inspect_screen(s) for s in screens]
    checks={}
    for i in range(1,25):
        if i>4:checks[str(i)]={'status':'NOT_RUN','evidence':'Outside first-slice implementation. No acceptance implied.'};continue
        failures=[{'screen':s['id'],**f} for s in inspected for f in s['checks'][str(i)]['failures']]
        status='FAIL' if failures else ('NOT_RUN' if any(s['checks'][str(i)]['status']!='PASS' for s in inspected) else 'PASS')
        checks[str(i)]={'status':status,'failures':failures,'evidence':f'{len(screens)} actual drum voice/page host renders; traces.json'}
    overlaps=[{'screen':s['id'],**f} for s in inspected for f in s['overlaps']]
    report={'schema':'dnbox-firmware-ui-defects/first-slice-v1','sourceClosureSha256':before['sha256'],
            'profileSha256':digest(source/'SOURCE-PROFILE.json'),'instrumentedClosureSha256':after['sha256'],
            'run':receipt,'checks':checks,'screens':inspected,'coverageComplete':False,
            'coverage':{'drumPartScreens':len(screens),'otherScreens':'NOT_RUN','hardware':'NOT_RUN','humanScoring':'NOT_RUN'},
            'overlaps':overlaps,'detectorExit':1 if any(c['status']=='FAIL' for c in checks.values()) or overlaps else 0,
            'limitations':['Test-only actual host rendering; known hardware/clip substitutes inherited.',
                'Only checks1–4 and supplemental actual glyph/value bounds collisions. No all24 or UI acceptance.',
                'Panel ink excludes backgrounds/borders; glyph/text/body/step pixels are actual changed raster pixels.',
                'Panel coverage is bbox area, not filled-ink density. Empty area uses exact integer-pixel rectangle.',
                'No human visual score is generated or inferred.']}
    write(resultdir/'report.json',report)
    return report


def main():
    parser=argparse.ArgumentParser();parser.add_argument('--old',type=Path,required=True);parser.add_argument('--current',type=Path,required=True);parser.add_argument('--output',type=Path,required=True)
    args=parser.parse_args();out=args.output.resolve();out.mkdir(parents=True,exist_ok=False)
    summary={}
    for label,source in [('old',args.old.resolve()),('current',args.current.resolve())]:
        result=run_one(label,source,out);summary[label]={k:result.get(k) for k in ('sourceClosureSha256','profileSha256','run','detectorExit','coverageComplete')}
        if 'checks' in result:summary[label]['counts']={i:{'status':c['status'],'failures':len(c.get('failures',[]))} for i,c in result['checks'].items()};summary[label]['overlaps']=len(result['overlaps'])
        else:summary[label].update({'status':result['status'],'reason':result.get('reason')})
        write(out/'summary.json',summary)
        print(label,json.dumps(summary[label]),flush=True)
    # A complete qualification does not exist from this slice, even if its local
    # detectors were to pass. Exit reflects actual implemented detectors only.
    return max(x.get('detectorExit',2) for x in summary.values())

if __name__=='__main__':sys.exit(main())
