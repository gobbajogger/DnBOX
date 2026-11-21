#!/usr/bin/env python3
"""Source-matched extras/edit visible text with exact caption contracts/mutants."""
import argparse,gzip,hashlib,json,os,re,shutil,subprocess
from pathlib import Path
from run import prepare,once,flags

def parse(log):
    frames=[];labels={}
    for line in log.read_text().splitlines():
        if line.startswith('VISIBLE_FRAME '):frames.append(json.loads(line[14:]))
        if line.startswith('VISIBLE_TEXT '):
            row=json.loads(line[13:]);row['text']=bytes.fromhex(row.pop('textHex')).decode('utf-8');labels.setdefault(row['id'],[]).append(row)
    if len({r['id'] for r in frames})!=len(frames):raise ValueError('duplicate visible-text case')
    return frames,labels

def array_strings(text,name):
    found=re.search(r'\b'+name+r'\[[^]]+\]\s*=\s*\{([^}]+)\}',text)
    if not found:raise ValueError('missing source caption array '+name)
    return [json.loads('"'+s+'"') for s in re.findall(r'"((?:\\.|[^"\\])*)"',found[1])]

def contracts(expected,source):
    panel=(source/'firmware/src/app/panel.c').read_text();buttons=array_strings(panel,'B_NAME');encoders=array_strings(panel,'E_NAME')
    rules={}
    for case in expected:
        required=[];forbidden=[]
        if case=='smc-explicit-peek-pickup':required=['PICKUP: UP'];forbidden=['PICKUP: MOVE UP TO TARGET','PICKUP: MOVE DOWN TO TARGET']
        if case in ('boot-safe-mode','boot-safe-crash-core-0','boot-safe-crash-core-1'):
            required=['DnBOX SAFE MODE','STARTUP CRASHES: 2','AUDIO: OFF    USB: ON','WEB INSTALLER: REINSTALL','PLAY: RETRY']
            forbidden=['IT CRASHED TWICE','WHILE STARTING.','NO SOUND. USB IS ON:','REINSTALL FROM THE','WEB INSTALLER.','PLAY: TRY AGAIN']
            if case.endswith(('core-0','core-1')):
                core=int(case[-1])+1;required += [{'text':'PC 12345678','y':200},{'text':f'{core} V01 D00001000','y':220}]
        if case=='boot-safe-retry':required=[{'text':'STARTING DnBOX...','y':172}];forbidden=['PLAY: TRY AGAIN','PLAY: RETRY']
        if case.startswith('boot-post-crash-core-'):
            core=int(case[-1])+1;required=[{'text':f'CRASH: CORE {core}','y':176},{'text':'PC 12345678','y':196}];forbidden=['RESTARTED AFTER A CRASH','THE SECOND CORE CRASHED']
        if case=='boot-crash-diagnostics':required=[{'text':'DnBOX CRASH','y':8},{'text':'00000001','y':40},{'text':'12345678','y':60},{'text':'00000000','y':84},{'text':'00001000','y':102},{'text':'00000000','y':120}]
        if case.startswith('hardware-calibration-button-'):required=['HARDWARE CALIBRATION','PRESS',buttons[int(case.rsplit('-',1)[1])]];forbidden=['TEACH EACH BUTTON AND KNOB']
        if case.startswith('hardware-calibration-encoder-'):required=['HARDWARE CALIBRATION','TURN RIGHT',encoders[int(case.rsplit('-',1)[1])]];forbidden=['TEACH EACH BUTTON AND KNOB']
        if case=='padcal-orientation':required=['K2: TOP-LEFT PAD','PLAY: CONFIRM'];forbidden=['pad 1 top-left?']
        if case.startswith('edited-part'):
            kind=expected[case][0]['state']['lane'];required=[('VELOCITY','CHANCE','PITCH','PAN')[kind]]
        rules[case]={'required':required,'forbidden':forbidden,'fullyVisibleRequired':True,'safetyDisplayOnly':case.startswith('boot-'),'evidenceKind':expected[case][0]['evidenceKind']}
    needed={'smc-explicit-peek-pickup','boot-safe-retry','boot-post-crash-core-0','boot-post-crash-core-1','boot-crash-diagnostics'}
    if not needed<=rules.keys():raise ValueError('newly fixed native display cases absent from expected source inventory')
    return rules

def check(frames,labels,rules):
    failures=[]
    if {r['id'] for r in frames}!=set(rules):failures.append({'case':'*','reason':'frame identity coverage mismatch'})
    for case,rule in rules.items():
        fully=[r for r in labels.get(case,[]) if r['fullyVisible']]
        if not fully:failures.append({'case':case,'reason':'no surviving opaque text, including retained regions'})
        for req in rule['required']:
            req={'text':req} if isinstance(req,str) else req
            if not any(r['text']==req['text'] and ('y' not in req or r['origin'][1]==req['y']) for r in fully):failures.append({'case':case,'reason':'required exact visible caption missing','required':req})
        for word in rule['forbidden']:
            if any(r['text']==word for r in fully):failures.append({'case':case,'reason':'stale caption visibly painted','forbidden':word})
    return failures

def execute(work,out,label,test,mode=None):
    env={k:v for k,v in os.environ.items() if 'MUTATE' not in k};env.update(F2_EXTRAS_ONLY='1',SMC_UI_TEST_FILE=test,SMC_UI_OUT=str(out/label))
    logpath=out/(label+'.log')
    if mode:
        env['VISIBLE_MUTATE_'+mode.upper()]='1';env['SMC_UI_SHOTS']=str(out/(label+'-shots'));Path(env['SMC_UI_SHOTS']).mkdir()
        cmd=[str(out/label.split('-mutant-')[0]/'test')]
    else:cmd=['sh','tests/host/smc_ui_test.sh']
    with logpath.open('x') as log:p=subprocess.run(cmd,cwd=work,env=env,stdout=log,stderr=subprocess.STDOUT,timeout=900)
    (out/(label+'.exit')).write_text(str(p.returncode)+'\n')
    if p.returncode:raise ValueError('renderer failure, cannot credit text oracle: '+label)
    return logpath

def run(prepared,out,expected_report,parse_only=False):
    if not parse_only:
        out.mkdir(parents=True,exist_ok=False);work=prepare(prepared,out)
        p=work/'firmware/src/gfx.c';s=p.read_text();s=once(s,'    const char *geo_text=s;','    s=visible_stale(s);\n    const char *geo_text=s;');p.write_text(s)
        reference=json.loads((work/'production-tree.json').read_text())
        receipt={'productionTree':reference['tree'],'preparedSource':str(prepared),'expectedReport':str(expected_report),'runnerSHA256':hashlib.sha256(Path(__file__).read_bytes()).hexdigest(),'traceSHA256':hashlib.sha256(Path(__file__).with_name('visible-trace.inc').read_bytes()).hexdigest()}
        (out/'source-receipt.json').write_text(json.dumps(receipt,indent=2)+'\n')
        for label,test in [('extended','tests/ui_geometry/capture.c'),('edited','tests/ui_geometry/capture-visible-edited.c')]:
            execute(work,out,label,test)
            for mode in ('missing','stale'):execute(work,out,label+'-mutant-'+mode,test,mode)
    work=out/'source';receipt=json.loads((out/'source-receipt.json').read_text());base=json.loads(expected_report.read_text())
    if base['productionTree']['tree']!=receipt['productionTree']:raise ValueError('expected report/source tree mismatch')
    ordinary=json.loads((work/'tests/ui_defects/first_slice/screen_inventory.json').read_text())
    expected={k:v for k,v in base['frames'].items() if k not in {r['id'] for r in ordinary['screens']}}
    rules=contracts(expected,work);all_frames=[];all_labels={};receipts=[];mutants=[]
    for label in ('extended','edited'):
        log=out/(label+'.log');frames,labels=parse(log);all_frames.extend(frames);all_labels.update(labels)
        subset={k:v for k,v in rules.items() if k.startswith('edited-part')==(label=='edited')}
        fail=check(frames,labels,subset)
        if fail:
            (out/'positive-failures.json').write_text(json.dumps(fail,indent=2)+'\n');raise ValueError('positive exact-caption checks failed; see positive-failures.json')
        receipts.append({'name':label,'frames':len(frames),'exit':int((out/(label+'.exit')).read_text()),'logSHA256':hashlib.sha256(log.read_bytes()).hexdigest()})
        for mode in ('missing','stale'):
            mlog=out/(label+'-mutant-'+mode+'.log');mf,ml=parse(mlog);failure=check(mf,ml,subset)
            if not failure:raise ValueError('runtime '+mode+' mutant accepted: '+label)
            if mode=='missing' and 'VISIBLE_MISSING_MUTANT' not in mlog.read_text():raise ValueError('missing-pixel mutant lacks intended marker')
            targeted={case for case,rule in subset.items() if rule['required'] and case!='padcal-orientation'}
            failed_cases={row['case'] for row in failure}
            if not targeted<=failed_cases:raise ValueError('mutant did not reject every targeted case: '+str(sorted(targeted-failed_cases)))
            mutants.append({'invocation':label,'mode':mode,'rendererExit':int((out/(label+'-mutant-'+mode+'.exit')).read_text()),'rejected':True,'targetedCases':sorted(targeted),'rejectedTargetCases':sorted(targeted&failed_cases),'positiveOnlyCases':['padcal-orientation'] if 'padcal-orientation' in subset else [],'failures':failure,'logSHA256':hashlib.sha256(mlog.read_bytes()).hexdigest()})
    if {r['id'] for r in all_frames}!=set(expected):raise ValueError('complete extended/edit text inventory mismatch')
    frame_rows=[];tsv=['case\tvisibility\tx\ty\texpected_opaque\tmatching_opaque\ttext'];candidate=[];vocabulary={}
    for f in all_frames:
        invocation='edited' if f['id'].startswith('edited-part') else 'extended'
        image=out/invocation/(f['id']+'.png');image_sha=hashlib.sha256(image.read_bytes()).hexdigest()
        if image_sha!=expected[f['id']][0]['nativePNGsha256']:raise ValueError('text observer changed primary native PNG bytes: '+f['id'])
        rows=sorted(all_labels.get(f['id'],[]),key=lambda r:(r['origin'][1],r['origin'][0],r['text']));unique=[];seen=set()
        for r in rows:
            key=(r['text'],tuple(r['origin']),r['fullyVisible'])
            if key in seen:continue
            seen.add(key);unique.append(r);tsv.append('\t'.join([f['id'],'FULL' if r['fullyVisible'] else 'PARTIAL',*map(str,r['origin']),str(r['expectedOpaque']),str(r['matchedOpaque']),r['text'].replace('\t',' ').replace('\n',' ')]))
            if r['fullyVisible']:
                vocabulary.setdefault(r['text'],set()).add(f['id']);reasons=flags(r['text'],f['id'])
                if reasons:candidate.append({'case':f['id'],'text':r['text'],'reasons':reasons,'action':'REVIEW_ONLY; no text modified by this observer'})
        frame_rows.append(dict(f,evidenceKind=expected[f['id']][0]['evidenceKind'],nativePNGsha256=image_sha,byteIdenticalToPrimary=True,labels=unique))
    report={'status':'PASS','productionTree':receipt['productionTree'],'frames':len(frame_rows),'invocations':receipts,'exactCaptionCases':sum(bool(v['required'] or v['forbidden']) for v in rules.values()),'allCasesWithVisibleText':True,'frameInventory':frame_rows,'visibleVocabulary':{k:sorted(v) for k,v in sorted(vocabulary.items())},'instructionCandidates':candidate,'mutants':mutants,'sourceReceipt':receipt,'validatorSHA256':hashlib.sha256(Path(__file__).read_bytes()).hexdigest(),'expectedReportSHA256':hashlib.sha256(expected_report.read_bytes()).hexdigest(),'scope':'Actual committed text provenance + native opaque-mask survival on all source-declared extended and persisted-edit cases. Partial source strings retained separately; no ordinary runtime, no repack, no R22 acceptance. Pad-calibration orientation is positive-only exact-caption coverage, not a claimed runtime mutant.'}
    for name,value in [('visible-text-inventory.json',report),('case-assertions.json',rules),('instruction-candidates.json',candidate),('mutant-receipts.json',mutants)]:
        data=(json.dumps(value,indent=2)+'\n').encode();(out/name).write_bytes(data);(out/(name+'.gz')).write_bytes(gzip.compress(data,mtime=0))
    data=('\n'.join(tsv)+'\n').encode();(out/'visible-text-inventory.tsv').write_bytes(data);(out/'visible-text-inventory.tsv.gz').write_bytes(gzip.compress(data,mtime=0))
    print(json.dumps({k:v for k,v in report.items() if k not in ('frameInventory','visibleVocabulary','instructionCandidates','mutants')}))

if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('--prepared-source',type=Path,required=True);p.add_argument('--expected-report',type=Path,required=True);p.add_argument('--output',type=Path,required=True);p.add_argument('--parse-only',action='store_true');a=p.parse_args();run(a.prepared_source.resolve(),a.output.resolve(),a.expected_report.resolve(),a.parse_only)
