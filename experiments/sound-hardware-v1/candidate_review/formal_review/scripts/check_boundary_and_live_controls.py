#!/usr/bin/env python3
"""Targeted recheck of final indexed runners and the short live-control policy test."""
import argparse,hashlib,json,pathlib,re,subprocess,sys
HERE=pathlib.Path(__file__).resolve().parent
parser=argparse.ArgumentParser(description=__doc__)
parser.add_argument('--package',type=pathlib.Path,required=True)
parser.add_argument('--output',type=pathlib.Path,required=True)
parser.add_argument('--review-dir',type=pathlib.Path,required=True)
args=parser.parse_args()
PACKAGE=args.package.expanduser().resolve()
OUT=args.output.expanduser().resolve()
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def indexed(rel):return (PACKAGE/rel).read_bytes()
def main():
    OUT.mkdir();inputs={};commands=[]
    def copy(rel,dst):
        data=indexed(rel);dst.parent.mkdir(parents=True,exist_ok=True);dst.write_bytes(data)
        inputs[rel]=hashlib.sha256(data).hexdigest()
    def run(argv,name,expected=0):
        cp=subprocess.run([str(x) for x in argv],capture_output=True,text=True)
        (OUT/(name+'.stdout.log')).write_text(cp.stdout)
        (OUT/(name+'.stderr.log')).write_text(cp.stderr)
        commands.append({'argv':[str(x) for x in argv],'exit_code':cp.returncode,'expected_exit_code':expected})
        if cp.returncode!=expected:raise RuntimeError(name+' unexpected process result')
        return cp
    runners=OUT/'runners';runners.mkdir()
    for n in ('verify_candidate.py','verify_compact_profile.py','check_compact_public_boundaries.py','run_callback_audit.py'):
        copy('candidate_review/'+n,runners/n)
    # These libraries were built from accepted/compact hash-pinned inputs by the
    # earlier private fresh-label relocation check in this formal review.
    libraries=args.review_dir.expanduser().resolve()
    before={p.name:sha(p) for p in libraries.iterdir() if p.is_file()}
    checker=runners/'check_compact_public_boundaries.py'
    run([sys.executable,checker,'--review-dir',libraries],'boundaries_fresh')
    boundary=json.loads((libraries/'PUBLIC-BOUNDARIES.json').read_text())
    assert boundary['case_count']==130 and boundary['all_pass']
    after={p.name:sha(p) for p in libraries.iterdir() if p.is_file()}
    assert all(after[k]==v for k,v in before.items())
    run([sys.executable,checker,'--review-dir',libraries],'boundaries_repeat_rejected',2)
    assert after=={p.name:sha(p) for p in libraries.iterdir() if p.is_file()}
    missing=OUT/'missing_libraries';missing.mkdir()
    run([sys.executable,checker,'--review-dir',missing],'boundaries_missing_rejected',2)
    assert list(missing.iterdir())==[]
    # Exact current no-overwrite guard prevents any compiler/timing execution.
    retained=runners/'retained';retained.mkdir();(retained/'marker').write_text('preserve\n')
    for name in ('verify_compact_profile.py','run_callback_audit.py'):
        run([sys.executable,runners/name,'--label','retained'],name+'.existing_rejected',2)
        assert list(retained.iterdir())==[retained/'marker'] and (retained/'marker').read_text()=='preserve\n'
    src=OUT/'accepted_src'
    for name in ('dnbx_core.c','dnbx_core.h','fastmath.h'):copy('hardware_candidate/src/'+name,src/name)
    test=OUT/'test_live_controls.c';copy('hardware_candidate/tests/test_live_controls.c',test)
    # Apply the same public behavioral timeline to the independently owned filter
    # envelope, replacing only public amp/filter field selectors in the test.
    mapping={'amp':'filter','filter':'amp','amp_stage':'filter_stage','filter_stage':'amp_stage'}
    swapped=OUT/'test_live_filter_controls.c'
    swapped.write_text(re.sub(r'\b(?:amp|filter|amp_stage|filter_stage)\b',lambda m:mapping[m.group()],test.read_text()))
    tests=[]
    for name,path in [('amp',test),('filter',swapped)]:
        exe=OUT/('live_'+name)
        cmd=['cc','-std=c11','-O2','-Wall','-Wextra','-Werror','-ffp-contract=off','-fno-fast-math','-DX0X_HOST','-I'+str(src),str(path),str(src/'dnbx_core.c'),'-o',str(exe)]
        run(cmd,'live_'+name+'_compile');cp=run([exe],'live_'+name+'_run')
        tests.append({'envelope':name,'test_sha256':sha(path),'pass':True,'stdout':cp.stdout.strip()})
    result={'source':'Exact current indexed blobs copied into reviewer-owned storage',
        'inputs_sha256':inputs,'commands':commands,'boundary_cases':130,
        'fresh_boundary_pass':True,'prior_receipts_unchanged':True,
        'repeat_boundary_rejected_without_mutation':True,'missing_libraries_rejected_without_mutation':True,
        'compact_and_callback_existing_outputs_rejected_without_execution':True,
        'live_control_policy_tests':tests,'native_or_long_timing_repeated':False,'all_pass':True}
    (OUT/'RESULT.json').write_text(json.dumps(result,indent=2)+'\n')
    print(json.dumps({k:v for k,v in result.items() if k not in ('commands','inputs_sha256')},indent=2))
if __name__=='__main__':main()
