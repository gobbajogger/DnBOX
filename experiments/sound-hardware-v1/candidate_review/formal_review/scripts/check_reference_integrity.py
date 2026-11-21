#!/usr/bin/env python3
"""Short reference-preflight controls, using retained curves instead of rerendering DSP.

Runs the real verifier main with compiler/nonreference test leaves stubbed. The
reference metric uses the unchanged frozen candidate trace and selected original
CSV rows. This probes the runner's evidence/exit logic, not full audio acceptance.
"""
import argparse,csv,hashlib,io,json,pathlib,subprocess,sys
HERE=pathlib.Path(__file__).resolve().parent
parser=argparse.ArgumentParser(description=__doc__)
parser.add_argument('--package',type=pathlib.Path,required=True)
parser.add_argument('--output',type=pathlib.Path,required=True)
args=parser.parse_args()
PACKAGE=args.package.expanduser().resolve()
OUT=args.output.expanduser().resolve()
def indexed(rel):return (PACKAGE/rel).read_bytes()
def digest(data):return hashlib.sha256(data).hexdigest()
DRIVER=r'''
import csv,importlib.util,json,pathlib,subprocess,sys
script,root,curves=map(pathlib.Path,sys.argv[1:4])
spec=importlib.util.spec_from_file_location('guarded_verifier',script);m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m)
m.HERE=root/'candidate_review';m.BASE=root
def compiler(args,**kw):
    (root/'compiler_leaf_reached').write_text('stubbed; no compiler execution\n')
    return subprocess.CompletedProcess(args,0,'stubbed compiler\n','')
m.subprocess.run=compiler;m.subprocess.check_output=lambda *a,**kw:'stubbed compiler (guard test only)\n'
m.bind=lambda p:None;m.Api=lambda d:None
def metrics(api,out):
    fixture=json.loads((root/'mapping/results/implementer_brief/recorded_envelope_behavior.json').read_text())
    rows=list(csv.DictReader((root/'mapping/results/implementer_brief/amp_observed_levels.csv').open()))
    result=[]
    for case in (1,2):
        cfg=next(x['configuration'] for x in fixture['cases'] if x['case_index']==case)
        levels=[float(x['candidate_amp_snapshot']) for x in csv.DictReader((curves/f'envelope_case{case}.csv').open())]
        selected=[x for x in rows if int(x['case_index'])==case];positions=[];expected=[]
        for row in selected:
            for endpoint in ('first','last'):
                positions.append(int(row[endpoint+'_observed_frame']));expected.append(float(row[endpoint+'_observed_level']))
        stat=m.error_stats(levels,expected,positions)
        stat.update(case_index=case,ratio_crosscheck_pass=True,ratio_crosscheck_source='unchanged frozen_03 receipt; no new DSP rendering')
        stat['negative_wrong_time_grid']=m.error_stats(levels[16:]+[levels[-1]]*16,expected,positions)
        stat['negative_linear_envelope']=m.error_stats(m.linear_contour(cfg,48000),expected,positions)
        result.append(stat)
    return result
m.envelope_tests=metrics
m.velocity_tests=lambda api:{'linear_pass':True,'negative_squared_rejected':True}
m.lifecycle_tests=lambda api:{'synthetic':{'pass':True}}
m.error_tests=lambda api:{'all_pass':True};m.finite_tests=lambda api:{'all_pass':True};m.long_duration_tests=lambda api:{'pass':True}
sys.argv=[str(script),'--label','observed','--source',str(root/'source')]
m.main()
'''
def main():
    OUT.mkdir();driver=OUT/'guard_driver.py';driver.write_text(DRIVER)
    current=indexed('candidate_review/verify_candidate.py')
    old=(HERE.parent/'historical/verify_candidate_pre_reference_guard.py').read_bytes()
    assert digest(old)=='6935f9507cdd98b0d012d42b58bf886004f3e15871e62db09fffeb3a081706f0'
    declaration=indexed('candidate_review/ACCEPTANCE-DECLARATION.json')
    expected=json.loads(declaration)['reference_hashes']
    refs={rel:indexed(rel) for rel in expected}
    curves=PACKAGE/'candidate_review/frozen_03'
    cases=[]
    specs=[('old_truncated',old,'truncate',True),('fixed_original',current,'none',True),
        ('fixed_truncated',current,'truncate',False),('fixed_changed_json',current,'json',False),
        ('fixed_changed_brief',current,'brief',False),('fixed_missing_csv',current,'missing',False),
        ('fixed_incomplete_declaration',current,'declaration',False)]
    for label,source,mutation,should_pass in specs:
        root=OUT/label;review=root/'candidate_review';review.mkdir(parents=True)
        script=review/'verify_candidate.py';script.write_bytes(source)
        (review/'ACCEPTANCE-DECLARATION.json').write_bytes(declaration)
        for rel,data in refs.items():p=root/rel;p.parent.mkdir(parents=True,exist_ok=True);p.write_bytes(data)
        observed=root/'mapping/results/implementer_brief/amp_observed_levels.csv'
        if mutation=='truncate':
            reader=csv.DictReader(io.StringIO(observed.read_text()));rows=list(reader);names=reader.fieldnames
            with observed.open('w',newline='') as f:
                w=csv.DictWriter(f,fieldnames=names);w.writeheader()
                for case in (1,2):w.writerow(next(x for x in rows if int(x['case_index'])==case))
        elif mutation in ('json','brief'):
            name='recorded_envelope_behavior.json' if mutation=='json' else 'BEHAVIOR-BRIEF.md'
            p=root/'mapping/results/implementer_brief'/name;p.write_bytes(p.read_bytes()+b' ')
        elif mutation=='missing':observed.unlink()
        elif mutation=='declaration':
            x=json.loads(declaration);x['reference_hashes'].pop('mapping/results/implementer_brief/BEHAVIOR-BRIEF.md')
            (review/'ACCEPTANCE-DECLARATION.json').write_text(json.dumps(x))
        src=root/'source';src.mkdir()
        for name in ('dnbx_core.c','dnbx_core.h','fastmath.h'):(src/name).write_bytes(indexed('hardware_candidate/src/'+name))
        cmd=[sys.executable,str(driver),str(script),str(root),str(curves)]
        cp=subprocess.run(cmd,capture_output=True,text=True)
        (root/'stdout.log').write_text(cp.stdout);(root/'stderr.log').write_text(cp.stderr)
        receipt_path=review/'observed/RESULT.json';receipt=json.loads(receipt_path.read_text()) if receipt_path.exists() else None
        success=cp.returncode==0 and receipt is not None and receipt['scoped_acceptance_pass']
        no_work=not (review/'observed').exists() and not (root/'compiler_leaf_reached').exists()
        recorded=receipt.get('verified_reference_sha256') if receipt else None
        case={'case':label,'source_sha256':digest(source),'mutation':mutation,'command':cmd,'exit_code':cp.returncode,
            'synthetic_nonreference_leaves':True,'full_audio_acceptance_claim':False,
            'scoped_acceptance_reported':receipt['scoped_acceptance_pass'] if receipt else None,
            'envelope_comparisons':sum(x['comparisons'] for x in receipt['envelope']) if receipt else None,
            'original_declaration_sha256':digest(declaration),
            'output_not_reserved_and_compiler_not_reached':no_work,
            'verified_reference_sha256':recorded,
            'pass':success if should_pass else cp.returncode!=0 and no_work}
        if label=='fixed_original':case['pass'] &= recorded==expected
        if label=='old_truncated':case['pass'] &= case['envelope_comparisons']==4
        cases.append(case)
    result={'purpose':'Reference provenance guard/control-flow verification; no compiler, DSP, original engine, native timing or long test execution.',
        'current_source_sha256':digest(current),'old_source_sha256':digest(old),'cases':cases,
        'old_negative_control_detected':cases[0]['pass'],'all_pass':all(x['pass'] for x in cases)}
    (OUT/'RESULT.json').write_text(json.dumps(result,indent=2)+'\n')
    print(json.dumps({'current_source_sha256':digest(current),'cases':[{k:v for k,v in x.items() if k in ('case','exit_code','envelope_comparisons','output_not_reserved_and_compiler_not_reached','pass')} for x in cases],'all_pass':result['all_pass']},indent=2))
    if not result['all_pass']:raise SystemExit(1)
if __name__=='__main__':main()
